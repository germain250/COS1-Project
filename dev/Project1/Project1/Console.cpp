#include "Console.h"
#include "Helper.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
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
	std::string PromptText(const std::string& label, bool required, std::size_t minLen, std::size_t maxLen)
	{
		return std::string();
	}
}