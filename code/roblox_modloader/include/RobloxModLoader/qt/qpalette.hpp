#pragma once

#include "RobloxModLoader/qt/qt_value_type.hpp"
#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class QBrush;

	class RML_EXPORT QPalette : public detail::QtValueType<32>
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
			return owned();
		}

	private:
		void destroy();
	};
}
