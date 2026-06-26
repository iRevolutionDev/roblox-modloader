#pragma once

#include "RobloxModLoader/qt/qdialog.hpp"
#include "RobloxModLoader/qt/qpushbutton.hpp"

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

	class QMessageBox : public QDialog
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

		explicit QMessageBox(const QWidget& parent = QWidget{});

		~QMessageBox();

		QMessageBox(const QMessageBox&) = delete;

		QMessageBox& operator=(const QMessageBox&) = delete;

		void setText(const QString& text) const;

		void setIcon(Icon icon) const;

		void setTextFormat(TextFormat format) const;

		QPushButton addButton(StandardButton button) const;

	private:
		bool m_owned = false;
	};
}
