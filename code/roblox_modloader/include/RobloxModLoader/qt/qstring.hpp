#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <string>
#include <string_view>

namespace rml::qt
{
	class RML_EXPORT QString
	{
	public:
		QString() = default;

		QString(std::string_view utf8);

		QString(const char* utf8);

		~QString();

		QString(const QString&) = delete;

		QString& operator=(const QString&) = delete;

		[[nodiscard]] const void* data() const
		{
			return &m_storage;
		}

		[[nodiscard]] void* storage()
		{
			return &m_storage;
		}

		[[nodiscard]] std::string to_utf8() const;

	private:
		void* m_storage = nullptr;
	};
}
