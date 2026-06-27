#pragma once

#include "RobloxModLoader/qt/qwidget.hpp"

#include <functional>
#include <string_view>

namespace rml::qt
{
	class RML_EXPORT QAbstractButton : public QWidget
	{
	public:
		void setText(std::string_view text);

		void setChecked(bool checked);
		[[nodiscard]] bool isChecked() const;

		void on_clicked(std::function<void()> handler) const;
		void on_toggled(std::function<void(bool)> handler) const;
	};
}
