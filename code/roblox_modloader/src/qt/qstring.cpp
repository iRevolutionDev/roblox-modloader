#include "RobloxModLoader/qt/qstring.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QString::QString(const std::string_view utf8)
	{
		static const auto from_utf8 =
		    detail::core<void* (*)(void*, const char*, int)>("?fromUtf8@QString@@SA?AV1@PEBDH@Z");
		if (from_utf8)
			from_utf8(&m_storage, utf8.data(), static_cast<int>(utf8.size()));
	}

	QString::QString(const char* utf8) : QString(std::string_view(utf8 ? utf8 : "")) {}

	QString::~QString()
	{
		if (!m_storage)
			return;

		static const auto destroy = detail::core<void (*)(void*)>("??1QString@@QEAA@XZ");
		if (destroy)
			destroy(&m_storage);
	}
}
