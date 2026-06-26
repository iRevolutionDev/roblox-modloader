#pragma once

#include <string_view>

namespace rml::qt
{
	class QString
	{
	public:
		QString(std::string_view utf8);

		QString(const char* utf8);

		~QString();

		QString(const QString&) = delete;

		QString& operator=(const QString&) = delete;

		[[nodiscard]] const void* data() const
		{
			return &m_storage;
		}

	private:
		void* m_storage = nullptr;
	};
}
