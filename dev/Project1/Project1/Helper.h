#pragma once
#include <string>
#include <vector>

// Color Roles

enum class ColorRole {
	Brand,
	Success,
	Warning,
	Error,
	Info,
	Muted
};

enum class SoundCue {
	None,
	Success,
	Warning,
	Error,
	Prompt
};

namespace Helper {
	// Console UI & Formatting
	void ClearScreen();
	void Pause(const std::string& message = "Press Enter to continue...");
	void PrintDivider(char ch = '-', int width = 56);
	void PrintHeader(const std::string& title, int width = 56);

	void PrintSuccess(const std::string& message);
	void PrintError(const std::string& message);
	void PrintWarning(const std::string& message);
	void PrintInfo(const std::string& message);

	void PrintTable(
		const std::vector<std::string>& headers, 
		const std::vector<std::vector<std::string>>& rows
	);

	// Safe Input and Menu Loops
	std::string PromptText(
		const std::string& label, 
		bool required = true, 
		std::size_t minLen = 1, 
		std::size_t maxLen = 128
	);

	std::string PromptPassword(
		const std::string& label,
		std::size_t minLen = 6,
		std::size_t maxLen = 24
	);

	int PromptInt(const std::string& label, int minVal, int maxVal);

	long long PromptMoneyCents(
		const std::string& label, 
		long long minCents = 1, 
		long long maxCents = 100000000 // up to $1,000,000
	);

	int PromptChoice(const std::string& title, const std::vector<std::string>& options, bool includeBackOption = true);

	bool PromptConfirm(const std::string& question);

	// Money, Validation, Security and Identity

	std::string FormatCurrency(long long cents);
	std::vector<long long> SplitCentsEqually(long long totalCents, std::size_t count);

	std::string trim(std::string& str);
	std::string NormalizeUsername(const std::string& username);

	std::string GenerateID(const std::string& prefix);
	std::string HashPassword(const std::string& plainPassword);
	bool IsValidEmail(const std::string& email);
	bool VerifyPassword(const std::string& plainPassword, const std::string& storedHash);

	std::string NowIsoTimestamp();

	// Console extras

	void InitConsoleCapabilities();
	void SetColorEnabled(bool enabled);
	void SetSoundEnabled(bool enabled);
	std::string Colorized(const std::string& text, ColorRole role);
	void PlaySound(SoundCue cue);
}