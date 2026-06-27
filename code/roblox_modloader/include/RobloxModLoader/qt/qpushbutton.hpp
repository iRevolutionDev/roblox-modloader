#pragma once

#include "RobloxModLoader/qt/qabstractbutton.hpp"

#include <string_view>

namespace rml::qt
{
	class RML_EXPORT QPushButton : public QAbstractButton
	{
	public:
		[[nodiscard]] static QPushButton* create(std::string_view text, QWidget* parent = nullptr);
		static void destroy(QPushButton* button);
	};
}
