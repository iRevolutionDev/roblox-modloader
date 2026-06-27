#include "RobloxModLoader/qt/qstring.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

#include <windows.h>

namespace rml::qt
{
	QString::QString(const std::string_view utf8)
	{
		static const auto from_utf8 = detail::core<void* (*)(void*, const char*, int)>("?fromUtf8@QString@@SA?AV1@PEBDH@Z");
		if (from_utf8)
			from_utf8(&m_storage, utf8.data(), static_cast<int>(utf8.size()));
	}

	QString::QString(const char* utf8) :
	    QString(std::string_view(utf8 ? utf8 : ""))
	{
	}

	QString::~QString()
	{
		if (!m_storage)
			return;

		static const auto destroy = detail::core<void (*)(void*)>("??1QString@@QEAA@XZ");
		if (destroy)
			destroy(&m_storage);
	}

	std::string QString::to_utf8() const
	{
		if (!m_storage)
			return {};
		
		static const auto utf16 = detail::core<const unsigned short* (*)(const void*)>("?utf16@QString@@QEBAPEBGXZ");
		if (!utf16)
			return {};

		const auto* const wide = reinterpret_cast<const wchar_t*>(utf16(&m_storage));
		if (!wide || !wide[0])
			return {};

		const int bytes = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
		if (bytes <= 1)
			return {};

		std::string out(static_cast<std::size_t>(bytes - 1), '\0');
		WideCharToMultiByte(CP_UTF8, 0, wide, -1, out.data(), bytes, nullptr, nullptr);
		return out;
	}
}
