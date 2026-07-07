#include "RobloxModLoader/qt/qslider.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"
#include "qt_connect.hpp"

#include <utility>

namespace rml::qt
{
	QSlider* QSlider::create(const Orientation orientation, QWidget* parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, int, void*)>("??0QSlider@@QEAA@W4Orientation@Qt@@PEAVQWidget@@@Z");
		return detail::heap_construct<QSlider>(detail::WIDGET_INSTANCE_SIZE, construct, static_cast<int>(orientation), parent);
	}

	QtOwned<QSlider> QSlider::create_owned(const Orientation orientation)
	{
		return QtOwned<QSlider>(create(orientation, nullptr));
	}

	void QSlider::destroy(QSlider* slider)
	{
		static const auto dtor = detail::widgets<void (*)(void*)>("??1QSlider@@UEAA@XZ");
		detail::heap_destroy(dtor, slider);
	}

	void QSlider::setRange(const int minimum, const int maximum)
	{
		static const auto fn = detail::widgets<void (*)(void*, int, int)>("?setRange@QAbstractSlider@@QEAAXHH@Z");
		if (fn)
			fn(this, minimum, maximum);
	}

	void QSlider::setValue(const int value)
	{
		static const auto fn = detail::widgets<void (*)(void*, int)>("?setValue@QAbstractSlider@@QEAAXH@Z");
		if (fn)
			fn(this, value);
	}

	int QSlider::value() const
	{
		static const auto fn = detail::widgets<int (*)(const void*)>("?value@QAbstractSlider@@QEBAHXZ");
		return fn ? fn(this) : 0;
	}

	void QSlider::setOrientation(const Orientation orientation)
	{
		static const auto fn = detail::widgets<void (*)(void*, int)>("?setOrientation@QAbstractSlider@@QEAAXW4Orientation@Qt@@@Z");
		if (fn)
			fn(this, orientation);
	}

	void QSlider::setSingleStep(const int step)
	{
		static const auto fn = detail::widgets<void (*)(void*, int)>("?setSingleStep@QAbstractSlider@@QEAAXH@Z");
		if (fn)
			fn(this, step);
	}

	void QSlider::setPageStep(const int step)
	{
		static const auto fn = detail::widgets<void (*)(void*, int)>("?setPageStep@QAbstractSlider@@QEAAXH@Z");
		if (fn)
			fn(this, step);
	}

	void QSlider::setTracking(const bool enabled)
	{
		static const auto fn = detail::widgets<void (*)(void*, bool)>("?setTracking@QAbstractSlider@@QEAAX_N@Z");
		if (fn)
			fn(this, enabled);
	}

	void QSlider::on_value_changed(std::function<void(int)> handler) const
	{
		static void* const signal = detail::widgets_export("?valueChanged@QAbstractSlider@@QEAAXH@Z");
		static const void* const meta = detail::widgets_export("?staticMetaObject@QAbstractSlider@@2UQMetaObject@@B");
		detail::connect_function(this, signal, meta, [handler = std::move(handler)](void** args) {
			if (handler && args && args[1])
				handler(*static_cast<int*>(args[1]));
		});
	}
}
