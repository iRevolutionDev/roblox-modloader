#pragma once

#include "RobloxModLoader/qt/qt_owned.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	class RML_EXPORT QDialog : public QWidget
	{
	public:
		[[nodiscard]] static QDialog* create(QWidget* parent);
		[[nodiscard]] static QtOwned<QDialog> create_owned();
		static void destroy(QDialog* dialog);

		void setModal(bool modal);
		
		int exec();
	};
}
