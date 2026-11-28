#include "Helper.h"
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>

namespace Helper {

	// Convert an integer monetary amount expressed in cents into a human-readable currency string.
	std::string FormatCurrency(long long cents)
	{
		bool negative = cents < 0;

		// calculate the absolute magnitude of cents
		unsigned long long absCents = negative ? 
			static_cast<unsigned long long>(-(cents + 1)) + 1ULL : 
			static_cast<unsigned long long>(cents);

		unsigned long long dollars = absCents / 100ULL;
		unsigned long long remCents = absCents % 100ULL;

		std::ostringstream oss;
		if (negative) oss << '-';
		oss << '$' << dollars << '.' << std::setw(2) << std::setfill('0') << remCents;
		return oss.str();
	}

	// Divide a monetary amount among participants without losing or creating money

	std::vector<long long> SplitCentsEqually(long long totalCents, std::size_t count)
	{
		if (count == 0) return {};

		const long long divisor = static_cast<long long>(count);

		long long baseShare = totalCents / divisor;
		long long remainder = totalCents % divisor;

		std::vector<long long> shares(count, baseShare);
		long long step = (remainder >= 0) ? 1 : -1;

		// std::llabs() returns the absolute value of a long long
		// how many get the extra cent in the order of the shares vector
		size_t extraCount = std::llabs(remainder);

		for (size_t i = 0; i < extraCount; i++) {
			shares[i] += step;
		}

		return shares;
	}
	std::string Trim(const std::string& str)
	{
		const auto first = str.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) return "";
		const auto last = str.find_last_not_of(" \t\r\n");

		return str.substr(first, last - first +1);
	}

	std::string GenerateID(const std::string& prefix) {
		auto generateSeed = []() {
			std::random_device rd;
			return rd();
			};

		std::mt19937_64 rng(generateSeed());

		std::uniform_int_distribution<std::uint64_t> dist;
		std::ostringstream oss;
		oss << prefix << '_' << std::hex << std::setw(12) << std::setfill('0')
			<< (dist(rng) & 0xFFFFFFFFFFFFULL);
		return oss.str();
	}

	namespace {
		std::uint64_t fnv1a64(const std::string& input) {
			std::uint64_t hash = 14695981039346656037ULL;
			for (unsigned char c : input) {
				hash ^= static_cast<std::uint64_t>(c);
				hash *= 1099511628211ULL;
			}
			return hash;
		}
	} // namespace

	std::string HashPassword(const std::string& plainPassword) {
		const std::string salt = GenerateID("salt").substr(5, 8);
		std::uint64_t acc = fnv1a64(salt + ":" + plainPassword);
		for (int i = 0; i < 10000; ++i) {
			acc = fnv1a64(std::to_string(acc) + salt + plainPassword);
		}

		std::ostringstream oss;
		oss << salt << '$' << std::hex << std::setw(16) << std::setfill('0') << acc;
		return oss.str();
	}

	bool IsValidEmail(const std::string& email)
	{
		if (email.empty() || email.size() > 255) {
			return false;
		}

		const size_t atPos = email.find('@');
		if (atPos == std::string::npos || atPos == 0 || atPos + 1 >= email.size()) {
			return false;
		}

		// Reject multiple '@' characters
		if (email.find('@', atPos + 1) != std::string::npos) {
			return false;
		}

		const std::string local = email.substr(0, atPos);
		const std::string domain = email.substr(atPos + 1);

		// Reject leading, trailing or consective dotsin local and domain
		if (local.front() == '.' || local.back() == '.' || 
			local.find("..") != std::string::npos) {
			return false;
		}

		if (domain.front() == '.' || domain.back() == '.' ||
			domain.find("..") != std::string::npos) {
			return false;
		}

		// Domain must contain at least one dot to separate name and TLD

		const size_t lastDot = domain.rfind('.');
		if (lastDot == std::string::npos || domain.size() - lastDot - 1 < 2) {
			return false;
		}

		// validate allowed character set for email

		for (unsigned char ch : local) {
			// the current character in iteration is a valid alphanumric or one of the allowed
			const bool ok = std::isalnum(ch) || ch == '.' || ch == '_' || ch == '-' ||
				ch == '+' || ch == '%';
			if (!ok) {
				return false;
			}
		}

		return true;
	}

	bool VerifyPassword(const std::string& plainPassword, const std::string& storedHash) {
		const std::size_t sep = storedHash.find('$');
		if (sep == std::string::npos) return false;

		const std::string salt = storedHash.substr(0, sep);
		std::uint64_t acc = fnv1a64(salt + ":" + plainPassword);
		for (int i = 0; i < 10000; ++i) {
			acc = fnv1a64(std::to_string(acc) + salt + plainPassword);
		}

		std::ostringstream oss;
		oss << salt << '$' << std::hex << std::setw(16) << std::setfill('0') << acc;
		return oss.str() == storedHash;
	}

	std::string NowIsoTimestamp()
	{
		const auto now = std::chrono::system_clock::now();

		const std::time_t t = std::chrono::system_clock::to_time_t(now);
		std::tm utcTime{};

#ifdef _WIN32
		gmtime_s(&utcTime, &t);
#else
		gmtime_r(&t, &utcTime)
#endif // _WIN32

		std::ostringstream oss;
		oss << std::put_time(&utcTime, "%Y-%m-%dT%H:%M:%SZ");
		return oss.str();

		return std::string();
	}


}