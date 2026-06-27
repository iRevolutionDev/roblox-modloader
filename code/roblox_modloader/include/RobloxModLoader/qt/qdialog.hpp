#pragma once

#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	class RML_EXPORT QDialog : public QWidget
	{
	public:
		[[nodiscard]] static QDialog* create(QWidget* parent = nullptr);
		static void destroy(QDialog* dialog);

		void setModal(bool modal);
		
		int exec();
	};
}
