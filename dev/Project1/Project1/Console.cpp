#include "Console.h"
#include "Helper.h"

#include <algorithm>
//#include <cctype>
#include <cmath>
//#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <memory>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

namespace Console {
	namespace {
		bool colorEnabled = true;
		bool soundEnabled = true;
		bool consoleInitialized = false;

		bool IsStdoutInteractive() {
		#ifdef _WIN32
			return _isatty(_fileno(stdout)) != 0;
		#else
			return isatty(fileno(stdout)) != 0;

		#endif

		}

		bool HasNoColorEnv() {
#ifdef _WIN32
			char* rawVal = nullptr;
			std::size_t len = 0;
			if (_dupenv_s(&rawVal, &len, "NO_COLOR") != 0 || rawVal == nullptr) {
				return false;
			}
			std::unique_ptr<char, decltype(&std::free)> guard(rawVal, &std::free);
			return guard.get()[0] != '\0';
#else
			const char* envVal = std::getenv("NO_COLOR");
			return envVal != nullptr && envVal[0] != '\0';
#endif
		}

		const char* AnsiCodeFor(ColorRole role) {
			switch (role) {
			case ColorRole::Brand:   return "\x1b[38;2;249;115;22m";
			case ColorRole::Success: return "\x1b[32m";
			case ColorRole::Warning: return "\x1b[33m";
			case ColorRole::Error:   return "\x1b[1;31m";
			case ColorRole::Info:    return "\x1b[36m";
			case ColorRole::Muted:   return "\x1b[90m";
			}
			return "";
		}

		void EnsureInitialized() {
			if (!consoleInitialized) {
				InitCapabilities();
			}
		}

		void EmitBell() {
			std::cout << '\a' << std::flush;
		}
	}

	// Capabilities, Color & Sound

	void InitCapabilities()
	{
		consoleInitialized = true;

		const bool interactive = IsStdoutInteractive();
		const bool noColor = HasNoColorEnv();

#ifdef _WIN32
		if (interactive) {
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (hOut != INVALID_HANDLE_VALUE) {
				DWORD dwMode = 0;
				if (GetConsoleMode(hOut, &dwMode)) {
					dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
					SetConsoleMode(hOut, dwMode);
				}
			}
		}
#endif // _WIN32

		colorEnabled = interactive && !noColor;
		soundEnabled = interactive;
	}
	void SetColorEnabled(bool enabled)
	{
		consoleInitialized = true;
		colorEnabled = enabled && !HasNoColorEnv();
	}
	bool IsColorEnabled()
	{
		EnsureInitialized();
		return colorEnabled;
	}
	void SetSoundEnabled(bool enabled)
	{
		consoleInitialized = true;
		soundEnabled = enabled;
	}
	bool IsSoundEnabled()
	{
		EnsureInitialized();
		return soundEnabled;
	}
	std::string Colorize(const std::string& text, ColorRole role)
	{
		EnsureInitialized();

		if (!colorEnabled || text.empty()) {
			return text;
		}

		return std::string(AnsiCodeFor(role)) + text + "\x1b[0m";
	}
	void PlaySoundCue(SoundCue cue)
	{
		EnsureInitialized();

		if (!soundEnabled || cue == SoundCue::None) return;

#ifdef _WIN32
		switch (cue)
		{
		case Console::SoundCue::Success:
			Beep(880, 90);
			Beep(1174, 110);
			break;
		case Console::SoundCue::Warning:
			Beep(587, 150);
			break;
		case Console::SoundCue::Error:
			Beep(262, 180);
			Beep(196, 220);
			break;
		case Console::SoundCue::Prompt:
			Beep(660, 70);
			break;
		case Console::SoundCue::None:
			break;
		}
#endif // _WIN32


	}


	// Console UI & Formatting

	void ClearScreen()
	{
		EnsureInitialized();

#ifdef _WIN32
		std::system("cls");
#else
		std::system("clear");
#endif // _WIN32

	}
	void Pause(const std::string& message)
	{
		EnsureInitialized();

		std::cout << "\n" << Colorize(message, ColorRole::Muted);
		std::string any;
		std::getline(std::cin, any);
	}
	void PrintDivider(char ch, int width)
	{
		EnsureInitialized();

		std::string line(std::max(1,width), ch);

		std::cout << Colorize(line, ColorRole::Muted);
	}
	void PrintHeader(const std::string& title, int width)
	{
		EnsureInitialized();

		const int safeWidth = std::max(1, width);
		const std::string border(safeWidth, '=');
		std::cout << border << '\n';
		std::cout << Colorize("  " + title, ColorRole::Brand) << '\n';
		std::cout << border << '\n';
	}
	void PrintSuccess(const std::string& message)
	{
		EnsureInitialized();

		std::cout << Colorize("[OK]", ColorRole::Success) << ' ' << message << '\n';

		PlaySoundCue(SoundCue::Success);
	}
	void PrintError(const std::string& message)
	{
		EnsureInitialized();

		std::cout << Colorize("[ERROR]", ColorRole::Error) << ' ' << message << '\n';

		PlaySoundCue(SoundCue::Error);
	}
	void PrintWarning(const std::string& message)
	{
		EnsureInitialized();

		std::cout << Colorize("[WARN]", ColorRole::Warning) << ' ' << message << '\n';

		PlaySoundCue(SoundCue::Warning);
	}
	void PrintInfo(const std::string& message)
	{
		EnsureInitialized();

		std::cout << Colorize("[INFO]", ColorRole::Info) << ' ' << message << '\n';
	}
	void PrintTable(const std::vector<std::string>& headers, const std::vector<std::vector<std::string>>& rows)
	{
		if (headers.empty()) return;

		const size_t col = headers.size();
		std::vector<size_t> widths(col, 0);

		// store the length of each word in the headers vector as the width in widths
		for (size_t c = 0; c < col; c++) {
			widths[c] = headers[c].size();
		}

		for (const auto& row : rows) {
			for (size_t c = 0; c < std::max(col, row.size()); c++) {
				// have the previously saved widths from the size of column names overwritten by 
				// whatever is longer between themselves or the length of the current cell in iteration
				widths[c] = std::max(widths[c], row[c].size());
			}
		}
		// A lambda that dynamically print the separater based on the size of the current the table
		auto PrintSeparator = [&]() {
			std::ostringstream sep;
			sep << '+';
			for (size_t w : widths) {
				sep << std::string(w + 2, '-') << '+';
			}
			std::cout << Colorize(sep.str(), ColorRole::Muted) << '\n';
		};

		PrintSeparator();

		std::cout << Colorize("|", ColorRole::Muted);

		// Print the table headers
		for (size_t c = 0; c < col; c++) {
			std::ostringstream cellStream;

			cellStream << ' ' << std::left << std::setw(widths[c]) << headers[c] << ' ';

			std::cout << Colorize(cellStream.str(), ColorRole::Brand) << Colorize("|", ColorRole::Muted);

		}

		std::cout << '\n';
		PrintSeparator();

		// Handle empty state (When the table contains no data)
		if (rows.empty()) {
			size_t totalWidth = 0;
			for (size_t width : widths) {
				totalWidth += width + 3;
			}

			if (totalWidth > 1) totalWidth -= 1;

			std::ostringstream emptyRow;

			emptyRow << "|" << std::left << std::setw(totalWidth) << " (No records found)" << '|';

			std::cout << Colorize(emptyRow.str(), ColorRole::Muted) << '\n';

			PrintSeparator();
			return;
		}

		// Print the table body
		for (const auto& row : rows) {
			std::cout << Colorize("|", ColorRole::Muted);
			for (size_t c = 0; c < col;c++) {
				const std::string cell = (c < row.size()) ? row[c] : "";
				std::cout << ' ' << std::left << std::setw(widths[c]) << cell << ' ' << Colorize("|", ColorRole::Muted);
			}

			std::cout << '\n';
		}
		PrintSeparator();
	}

	// prompt the user a text of optionally ranging size

	std::string PromptText(const std::string& label, bool required, std::size_t minLen, std::size_t maxLen)
	{
		while (true) {
			std::cout << Colorize(label, ColorRole::Brand) << ": ";
			std::string line;

			if (!std::getline(std::cin, line)) {
				std::cin.clear();
				continue;
			}

			std::string cleaned = Helper::Trim(line);

			if (!required && cleaned.empty()) {
				return "";
			}

			if (cleaned.size() < minLen) {
				PrintError("Input must be atleast " + std::to_string(minLen) + " character(s).");
				continue;
			}

			if (cleaned.size() > maxLen) {
				PrintError("Input cannot exceed " + std::to_string(maxLen) + " charactors.");
				continue;
			}

			return cleaned;
		}
	}

	// Prompt the user an optionally validated password
	std::string PromptPassword(
		const std::string& label,
		std::size_t minLen,
		std::size_t maxLen,
		bool validate)
	{
		while (true) {
			std::cout << Colorize(label, ColorRole::Brand) << ": ";
			std::string line;

			if (!std::getline(std::cin, line)) {
				std::cin.clear();
				continue;
			}

			if (line.size() < minLen && validate) {
				PrintError("Password must be at least " + std::to_string(minLen) + " characters.");
				continue;
			}

			if (line.size() > maxLen && validate) {
				PrintError("Choose a password between " + std::to_string(minLen) + " and " + std::to_string(maxLen) + " characters.");
				continue;
			}
			
			if (line.find_first_of("\r\n\t") != std::string::npos) {
				PrintError("Password cannot contain tabs or newline charactors.");
				continue;
			}

			return line;
		}
	}

	// Prompt an int in optional range and optionally validate it
	int PromptInt(const std::string& label, int minVal, int maxVal, bool validate)
	{
		while (true) {
			std::cout << Colorize(label, ColorRole::Brand);
			validate ? 
				std::cout << Colorize(" (" + std::to_string(minVal) + "-" + std::to_string(maxVal) + ")", ColorRole::Muted) << ": "
				: std::cout << ": ";

			std::string line;

			if (!std::getline(std::cin, line)) {
				std::cin.clear();
				continue;
			}

			std::string cleaned = Helper::Trim(line);

			if (cleaned.empty()) {
				PrintError("Please enter a valid whole number.");
				continue;
			}

			try
			{
				// convert the cleaned input to integer and ensure validity
				std::size_t idx = 0;
				int value = std::stoi(cleaned, &idx);
				if (idx != cleaned.size()) {
					PrintError("Invalid characters in number.");
					continue;
				}
				// validate ranges if validate is true
				if ((value < minVal || value > maxVal) && validate) {
					PrintError("Value must be between " + std::to_string(minVal) + " and " + std::to_string(maxVal) + ".");
					continue;
				}

				return value;
			}
			catch (...)
			{
				PrintError("Please enter a valid whole number.");
			}
		}
		return 0;
	}
	// Prompt the user amount in dollars and have it validated and
	// converted to cents and returned as long long
	long long PromptMoneyCents(const std::string& label,
		long long minCents,
		long long maxCents) {

		while (true) {
			// Prompt the user for input of money in dollars
			std::cout << Colorize(label, ColorRole::Brand) << " ($): ";
			std::string line;
			if (!std::getline(std::cin, line)) {
				std::cin.clear();
				continue;
			}

			// trim and clean the input
			std::string cleaned = Helper::Trim(line);

			if (!cleaned.empty() && cleaned[0] == '$') {
				cleaned.erase(0, 1);
				cleaned = Helper::Trim(cleaned);
			}

			if (cleaned.empty()) {
				PrintError("Please enter a valid dollar amount (e.g., 25.50).");
				continue;
			}

			try {
				// converting input to double and validate the monetary format validity
				std::size_t idx = 0;
				double dollars = std::stod(cleaned, &idx);
				if (idx != cleaned.size() || !std::isfinite(dollars)) {
					PrintError("Invalid monetary format.");
					continue;
				}

				std::size_t dotPos = cleaned.find('.');
				if (dotPos != std::string::npos && cleaned.size() - dotPos - 1 > 2) {
					PrintError("Please enter at most 2 decimal places.");
					continue;
				}

				// Convert the validated input to cents and validate the desire range

				long long cents = static_cast<long long>(std::llround(dollars * 100.0));
				if (cents < minCents || cents > maxCents) {
					PrintError("Amount must be between " + Helper::FormatCurrency(minCents) +
						" and " + Helper::FormatCurrency(maxCents) + ".");
					continue;
				}
				return cents;
			}
			catch (...) {
				PrintError("Please enter a valid dollar amount (e.g., 25.50).");
			}
		}
	}

	// Console 'PromptChoice' to quicly print the Menu to the user with optional Back option
	// and return their choice 

	int PromptChoice(const std::string& title, const std::vector<std::string>& options, bool includeBackOption)
	{
		// Print the menu title
		PrintHeader(title);

		// iterate through the options on the meny and display them
		for (int i = 0; i < options.size(); i++) {
			std::cout << " " << Colorize("[" + std::to_string(i + 1) + "]", ColorRole::Brand) << ' ' << options[i] << '\n';
		}
		if (includeBackOption) {
			std::cout << Colorize("[0]", ColorRole::Muted) << ' ' << Colorize("Back", ColorRole::Muted) << '\n';
		}
		PrintDivider('-');

		// bounds of the menu choice
		int minOption = includeBackOption ? 0 : 1;
		int maxOption = options.size();

		return PromptInt("Select an Option", minOption, maxOption, true);
	}

	// A console 'PromptConfirm' for critical actions in the system
	bool PromptConfirm(const std::string& question)
	{
		PlaySoundCue(SoundCue::Prompt);

		while (true) {
			std::cout << Colorize(question, ColorRole::Brand) << Colorize(" [y,n]: ", ColorRole::Muted);

			std::string line;
			if (std::getline(std::cin, line)) {
				std::cin.clear();
				continue;
			}
			//trim the input using Trim Helper
			std::string cleaned = Helper::Trim(line);

			// convert the response to lowercase to be less rigid
			std::transform(cleaned.begin(), cleaned.end(), cleaned.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

			if (cleaned == "y" || cleaned == "yes") return true;
			if (cleaned == "n" || cleaned == "no") return false;

			PrintError("Please answer 'y' for Yes or 'n' for No");
		}
		return false;
	}
}