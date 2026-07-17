#include "RobloxModLoader/qt/qcolor.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

#include <cstdint>
#include <cstring>

namespace rml::qt
{
	static constexpr std::int32_t SPEC_INVALID = 0;
	static constexpr std::int32_t SPEC_RGB = 1;
	static constexpr std::uint16_t CHANNEL_SCALE = 0x101;

	struct QColorRgbLayout
	{
		std::int32_t m_spec;
		std::uint16_t m_alpha;
		std::uint16_t m_red;
		std::uint16_t m_green;
		std::uint16_t m_blue;
		std::uint16_t m_pad;
	};

	static constexpr bool is_channel(const int value)
	{
		return value >= 0 && value <= 255;
	}

	QColor::QColor(const int red, const int green, const int blue, const int alpha)
	{
		static void* const ctor = detail::gui_export_optional("QColor::QColor(int, int, int, int)");
		if (ctor)
		{
			reinterpret_cast<void (*)(void*, int, int, int, int)>(ctor)(m_storage, red, green, blue, alpha);
			return;
		}
		
		QColorRgbLayout layout{};

		if (is_channel(red) && is_channel(green) && is_channel(blue) && is_channel(alpha))
		{
			layout.m_spec = SPEC_RGB;
			layout.m_alpha = static_cast<std::uint16_t>(alpha * CHANNEL_SCALE);
			layout.m_red = static_cast<std::uint16_t>(red * CHANNEL_SCALE);
			layout.m_green = static_cast<std::uint16_t>(green * CHANNEL_SCALE);
			layout.m_blue = static_cast<std::uint16_t>(blue * CHANNEL_SCALE);
		}
		else
		{
			layout.m_spec = SPEC_INVALID;
		}

		static_assert(sizeof(QColorRgbLayout) <= STORAGE_SIZE, "QColor storage cannot hold its layout");
		std::memcpy(m_storage, &layout, sizeof(layout));
	}
}
