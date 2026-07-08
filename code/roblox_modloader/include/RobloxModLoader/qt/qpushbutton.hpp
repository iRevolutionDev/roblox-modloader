#pragma once

#include "RobloxModLoader/qt/qabstractbutton.hpp"
#include "RobloxModLoader/qt/qt_owned.hpp"

#include <string_view>

namespace rml::qt
{
	class RML_EXPORT QPushButton : public QAbstractButton
	{
	public:
		[[nodiscard]] static QPushButton* create(std::string_view text, QWidget* parent);
		[[nodiscard]] static QtOwned<QPushButton> create_owned(std::string_view text);
		static void destroy(QPushButton* button);
	};
}
