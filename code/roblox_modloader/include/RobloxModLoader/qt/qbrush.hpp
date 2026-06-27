#pragma once

#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class QPixmap;

	class RML_EXPORT QBrush
	{
	public:
		explicit QBrush(const QPixmap& texture);
		~QBrush();

		QBrush(const QBrush&) = delete;
		QBrush& operator=(const QBrush&) = delete;

		[[nodiscard]] void* data() const
		{
			return const_cast<unsigned char*>(m_storage);
		}

	private:
		alignas(void*) unsigned char m_storage[16]{};
		bool m_constructed = false;
	};
}
