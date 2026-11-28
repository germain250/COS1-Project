#pragma once
#include <string>
#include <vector>

namespace Helper {

	// Money, Validation, Security and Identity

	std::string FormatCurrency(long long cents);
	std::vector<long long> SplitCentsEqually(long long totalCents, std::size_t count);

	std::string Trim(const std::string& str);

	std::string GenerateID(const std::string& prefix);
	std::string HashPassword(const std::string& plainPassword);
	bool IsValidEmail(const std::string& email);
	bool VerifyPassword(const std::string& plainPassword, const std::string& storedHash);

	std::string NowIsoTimestamp();
}