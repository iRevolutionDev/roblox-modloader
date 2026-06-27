#pragma once

#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class QBrush;

	class RML_EXPORT QPalette
	{
	public:
		enum ColorRole
		{
			Base = 9,
		};

		QPalette() = default;

		explicit QPalette(const void* source);

		QPalette(QPalette&& other) noexcept;
		QPalette& operator=(QPalette&& other) noexcept;
		QPalette(const QPalette&) = delete;
		QPalette& operator=(const QPalette&) = delete;
		~QPalette();

		void set_brush(ColorRole role, const QBrush& brush);

		[[nodiscard]] bool valid() const
		{
			return m_owned;
		}

		[[nodiscard]] void* data() const
		{
			return const_cast<unsigned char*>(m_storage);
		}

	private:
		void destroy();

		alignas(void*) unsigned char m_storage[32]{};
		bool m_owned = false;
	};
}
