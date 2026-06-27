#pragma once

#include "RobloxModLoader/qt/qobject.hpp"
#include "RobloxModLoader/qt/qpalette.hpp"

namespace rml::qt
{
	class QString;
	
	class RML_EXPORT QWidget : public QObject
	{
	public:
		virtual void setVisible(bool visible) = 0;
		virtual void* sizeHint() const = 0;
		virtual void* minimumSizeHint() const = 0;
		virtual int heightForWidth(int width) const = 0;
		virtual bool hasHeightForWidth() const = 0;
		virtual void mousePressEvent(void* e) = 0;
		virtual void mouseReleaseEvent(void* e) = 0;
		virtual void mouseDoubleClickEvent(void* e) = 0;
		virtual void mouseMoveEvent(void* e) = 0;
		virtual void wheelEvent(void* e) = 0;
		virtual void keyPressEvent(void* e) = 0;
		virtual void keyReleaseEvent(void* e) = 0;
		virtual void focusInEvent(void* e) = 0;
		virtual void focusOutEvent(void* e) = 0;
		virtual void enterEvent(void* e) = 0;
		virtual void leaveEvent(void* e) = 0;
		virtual void paintEvent(void* e) = 0;

		void setWindowTitle(const QString& title);
		void setStyleSheet(const QString& style);
		void show();

		void resize(int width, int height);
		void setFixedSize(int width, int height);
		void setGeometry(int x, int y, int width, int height);
		void move(int x, int y);

		void setEnabled(bool enabled);
		void setToolTip(const QString& text);
		bool close();

		[[nodiscard]] QWidget* viewport() const;

		[[nodiscard]] QPalette palette() const;
		void set_palette(const QPalette& palette);

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		void set_auto_fill_background(bool enabled);

		void update();
	};
}
