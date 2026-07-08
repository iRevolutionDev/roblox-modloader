#pragma once

#include "RobloxModLoader/qt/qdialog.hpp"
#include "RobloxModLoader/qt/qpushbutton.hpp"
#include "RobloxModLoader/qt/qt_owned.hpp"

namespace rml::qt
{
	class QString;

	enum class TextFormat : int
	{
		PlainText = 0,
		RichText = 1,
		AutoText = 2,
		MarkdownText = 3,
	};

	class RML_EXPORT QMessageBox : public QDialog
	{
	public:
		enum Icon
		{
			NoIcon = 0,
			Information = 1,
			Warning = 2,
			Critical = 3,
			Question = 4,
		};

		enum StandardButton
		{
			Ok = 0x00000400,
		};

		[[nodiscard]] static QMessageBox* create(QWidget* parent);
		[[nodiscard]] static QtOwned<QMessageBox> create_owned();
		static void destroy(QMessageBox* box);

		void setText(const QString& text);
		void setIcon(Icon icon);
		void setTextFormat(TextFormat format);
		QPushButton* addButton(StandardButton button);
	};
}
