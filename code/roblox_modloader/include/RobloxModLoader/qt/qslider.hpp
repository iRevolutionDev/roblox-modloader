#pragma once

#include "RobloxModLoader/qt/qwidget.hpp"

#include <functional>

namespace rml::qt
{
	class RML_EXPORT QSlider : public QWidget
	{
	public:
		enum Orientation
		{
			Horizontal = 0x1,
			Vertical = 0x2,
		};

		[[nodiscard]] static QSlider* create(Orientation orientation, QWidget* parent = nullptr);

		static void destroy(QSlider* slider);

		void setRange(int minimum, int maximum);
		void setValue(int value);
		[[nodiscard]] int value() const;

		void setOrientation(Orientation orientation);
		void setSingleStep(int step);
		void setPageStep(int step);
		void setTracking(bool enabled);

		void on_value_changed(std::function<void(int)> handler) const;
	};
}
