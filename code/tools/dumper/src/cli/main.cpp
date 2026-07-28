#include "cli/runner.hpp"

#include <cstdio>
#include <exception>

int main()
{
	try
	{
		if (const auto result = rml::dumper::cli::Runner().run(); !result)
		{
			std::fputs(result.error().message().c_str(), stderr);
			std::fputc('\n', stderr);
			return static_cast<int>(result.error().code());
		}
		return 0;
	}
	catch (const std::exception& exception)
	{
		std::fputs(exception.what(), stderr);
		std::fputc('\n', stderr);
		return 1;
	}
}
