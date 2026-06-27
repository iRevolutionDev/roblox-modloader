#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <string_view>

namespace rml::qt
{
	class QColor;

	class RML_EXPORT QPixmap
	{
	public:
		QPixmap() = default;

		explicit QPixmap(std::string_view file_path);
		QPixmap(int width, int height);

		[[nodiscard]] bool loaded() const
		{
			return m_loaded;
		}

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		void fill(const QColor& color) const;

		bool save(std::string_view file_path) const;

		[[nodiscard]] void* data() const
		{
			return const_cast<unsigned char*>(m_storage);
		}

	private:
		alignas(void*) unsigned char m_storage[32]{};
		bool m_loaded = false;
	};
}
