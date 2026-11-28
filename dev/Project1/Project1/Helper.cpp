#include "Helper.h"
#include <sstream>
#include <iomanip>

namespace Helper {
	std::string FormatCurrency(long long cents)
	{
		bool negative = cents < 0;
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
	std::string Trim(std::string& str)
	{
		const auto first = str.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) return "";
		const auto last = str.find_last_not_of(" \t\r\n");

		return str.substr(first, last - first +1);
	}
}