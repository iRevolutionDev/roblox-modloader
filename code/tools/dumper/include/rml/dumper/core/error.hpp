#pragma once

#include <format>
#include <string>
#include <utility>

namespace rml::dumper
{
	enum class ErrorCode : std::uint8_t
	{
		usage = 1,
		fetch = 2,
		invalid_image = 3,
		anchor = 4,
		recovery = 5,
		validation = 6,
	};

	class Error
	{
	public:
		Error(const ErrorCode code, std::string message) :
		    m_code(code),
		    m_message(std::move(message))
		{
		}

		template<typename... Args>
		[[nodiscard]] static Error make(const ErrorCode code, const std::format_string<Args...> fmt, Args&&... args)
		{
			return Error(code, std::format(fmt, std::forward<Args>(args)...));
		}

		[[nodiscard]] ErrorCode code() const { return m_code; }
		[[nodiscard]] const std::string& message() const { return m_message; }

	private:
		ErrorCode m_code;
		std::string m_message;
	};
}
