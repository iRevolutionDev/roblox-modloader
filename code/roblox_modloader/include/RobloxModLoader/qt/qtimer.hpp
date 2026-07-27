#pragma once

#include "RobloxModLoader/qt/qobject.hpp"
#include "RobloxModLoader/qt/qt_owned.hpp"

#include <functional>

namespace rml::qt
{
	class RML_EXPORT QTimer : public QObject
	{
	public:
		[[nodiscard]] static QTimer* create(QObject* parent);
		[[nodiscard]] static QtOwned<QTimer> create_owned();

		static void destroy(QTimer* timer);

		void setInterval(int milliseconds);
		void setSingleShot(bool single_shot);

		void start();
		void stop();

		void on_timeout(std::function<void()> handler) const;
	};
}
