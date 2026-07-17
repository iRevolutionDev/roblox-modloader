#include "RobloxModLoader/qt/qstring.hpp"

#include "RobloxModLoader/memory/foreign_call.hpp"
#include "RobloxModLoader/qt/qarray_data.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"


namespace rml::qt
{
	QString::QString(const std::string_view utf8)
	{
		static void* const from_utf8 = detail::core_export({
		    "QString::fromUtf8(char const*, int)",
		    "QString::fromUtf8_helper(char const*, int)",
		});

		memory::call_returning(from_utf8, m_storage, utf8.data(), static_cast<int>(utf8.size()));
	}

	QString::QString(const char* utf8) :
	    QString(std::string_view(utf8 ? utf8 : ""))
	{
	}

	QString::~QString()
	{
		detail::destroy_qstring(m_storage);
	}

	std::string QString::to_utf8() const
	{
		if (!m_storage)
			return {};
		
		static const auto utf16 = detail::core<const unsigned short* (*)(const void*)>("QString::utf16() const");
		if (!utf16)
			return {};

		const auto* const utf16_data = reinterpret_cast<const char16_t*>(utf16(&m_storage));
		if (!utf16_data || !utf16_data[0])
			return {};

		std::string out;

		for (const char16_t* it = utf16_data; *it; ++it)
		{
			char32_t code_point = *it;

			if (code_point >= 0xD800 && code_point <= 0xDBFF && it[1] >= 0xDC00 && it[1] <= 0xDFFF)
			{
				code_point = 0x10000 + ((code_point - 0xD800) << 10) + (it[1] - 0xDC00);
				++it;
			}

			if (code_point < 0x80)
			{
				out.push_back(static_cast<char>(code_point));
			}
			else if (code_point < 0x800)
			{
				out.push_back(static_cast<char>(0xC0 | (code_point >> 6)));
				out.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
			}
			else if (code_point < 0x10000)
			{
				out.push_back(static_cast<char>(0xE0 | (code_point >> 12)));
				out.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
				out.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
			}
			else
			{
				out.push_back(static_cast<char>(0xF0 | (code_point >> 18)));
				out.push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
				out.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
				out.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
			}
		}

		return out;
	}
}
