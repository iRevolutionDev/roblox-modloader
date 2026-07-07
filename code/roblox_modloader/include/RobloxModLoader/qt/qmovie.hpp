#pragma once

#include "RobloxModLoader/qt/qobject.hpp"
#include "RobloxModLoader/qt/qpixmap.hpp"
#include "RobloxModLoader/qt/qt_owned.hpp"

#include <functional>

namespace rml::qt
{
	class QString;

	class RML_EXPORT QMovie : public QObject
	{
	public:
		enum class CacheMode
		{
			None = 0,
			All = 1,
		};

		[[nodiscard]] static QMovie* create(const QString& file_name, QObject* parent);
		[[nodiscard]] static QtOwned<QMovie> create_owned(const QString& file_name);

		static void destroy(QMovie* movie);

		[[nodiscard]] bool isValid() const;
		[[nodiscard]] int frameCount() const;
		[[nodiscard]] int currentFrameNumber() const;
		[[nodiscard]] int nextFrameDelay() const;

		[[nodiscard]] QPixmap currentPixmap() const;

		void setCacheMode(CacheMode mode);
		void setSpeed(int percent);

		void start();
		void stop();
		void setPaused(bool paused);
		bool jumpToNextFrame();

		void on_frame_changed(std::function<void()> handler) const;
	};
}
