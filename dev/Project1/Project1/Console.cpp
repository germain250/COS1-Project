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
			return std::string();
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
}