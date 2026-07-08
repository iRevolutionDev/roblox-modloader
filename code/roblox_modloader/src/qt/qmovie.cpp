#include "RobloxModLoader/qt/qmovie.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"
#include "qt_connect.hpp"

#include <utility>

namespace rml::qt
{
	QMovie* QMovie::create(const QString& file_name, QObject* parent)
	{
		static const auto construct = detail::gui<void (*)(void*, void*)>("??0QMovie@@QEAA@PEAVQObject@@@Z");
		auto* const movie = detail::heap_construct<QMovie>(detail::WIDGET_INSTANCE_SIZE, construct, parent);
		if (!movie)
			return nullptr;

		static const auto set_file_name = detail::gui<void (*)(void*, const void*)>("?setFileName@QMovie@@QEAAXAEBVQString@@@Z");
		if (set_file_name)
			set_file_name(movie, file_name.data());

		return movie;
	}

	QtOwned<QMovie> QMovie::create_owned(const QString& file_name)
	{
		return QtOwned<QMovie>(create(file_name, nullptr));
	}

	void QMovie::destroy(QMovie* movie)
	{
		static const auto dtor = detail::gui<void (*)(void*)>("??1QMovie@@UEAA@XZ");
		detail::heap_destroy(dtor, movie);
	}

	bool QMovie::isValid() const
	{
		static const auto fn = detail::gui<bool (*)(const void*)>("?isValid@QMovie@@QEBA_NXZ");
		return fn && fn(this);
	}

	int QMovie::frameCount() const
	{
		static const auto fn = detail::gui<int (*)(const void*)>("?frameCount@QMovie@@QEBAHXZ");
		return fn ? fn(this) : 0;
	}

	int QMovie::currentFrameNumber() const
	{
		static const auto fn = detail::gui<int (*)(const void*)>("?currentFrameNumber@QMovie@@QEBAHXZ");
		return fn ? fn(this) : 0;
	}

	int QMovie::nextFrameDelay() const
	{
		static const auto fn = detail::gui<int (*)(const void*)>("?nextFrameDelay@QMovie@@QEBAHXZ");
		return fn ? fn(this) : 0;
	}

	QPixmap QMovie::currentPixmap() const
	{
		static const auto fn = detail::gui<void (*)(const void* self, void* sret)>("?currentPixmap@QMovie@@QEBA?AVQPixmap@@XZ");

		QPixmap result;
		if (!fn)
			return result;

		fn(this, result.m_storage);
		result.set_owned(true);
		return result;
	}

	void QMovie::setCacheMode(const CacheMode mode)
	{
		static const auto fn = detail::gui<void (*)(void*, int)>("?setCacheMode@QMovie@@QEAAXW4CacheMode@1@@Z");
		if (fn)
			fn(this, static_cast<int>(mode));
	}

	void QMovie::setSpeed(const int percent)
	{
		static const auto fn = detail::gui<void (*)(void*, int)>("?setSpeed@QMovie@@QEAAXH@Z");
		if (fn)
			fn(this, percent);
	}

	void QMovie::start()
	{
		static const auto fn = detail::gui<void (*)(void*)>("?start@QMovie@@QEAAXXZ");
		if (fn)
			fn(this);
	}

	void QMovie::stop()
	{
		static const auto fn = detail::gui<void (*)(void*)>("?stop@QMovie@@QEAAXXZ");
		if (fn)
			fn(this);
	}

	void QMovie::setPaused(const bool paused)
	{
		static const auto fn = detail::gui<void (*)(void*, bool)>("?setPaused@QMovie@@QEAAX_N@Z");
		if (fn)
			fn(this, paused);
	}

	bool QMovie::jumpToNextFrame()
	{
		static const auto fn = detail::gui<bool (*)(void*)>("?jumpToNextFrame@QMovie@@QEAA_NXZ");
		return fn && fn(this);
	}

	void QMovie::on_frame_changed(std::function<void()> handler) const
	{
		static void* const signal = detail::gui_export("?frameChanged@QMovie@@QEAAXH@Z");
		static const void* const meta = detail::gui_export("?staticMetaObject@QMovie@@2UQMetaObject@@B");
		detail::connect_function(this, signal, meta, [handler = std::move(handler)](void**) {
			if (handler)
				handler();
		});
	}
}
