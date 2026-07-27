#include "RobloxModLoader/qt/qpixmap.hpp"

#include "RobloxModLoader/memory/foreign_call.hpp"
#include "RobloxModLoader/qt/qcolor.hpp"
#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QPixmap::QPixmap(const std::string_view file_path)
	{
		static const auto ctor = detail::gui<void (*)(void*, const void*, const char*, int)>("QPixmap::QPixmap(QString const&, char const*, QFlags<Qt::ImageConversionFlag>)");
		if (!ctor)
			return;

		const QString path(file_path);
		ctor(m_storage, path.data(), nullptr, 0);
		set_owned(true);
	}

	QPixmap::QPixmap(const int width, const int height)
	{
		static const auto ctor = detail::gui<void (*)(void*, int, int)>("QPixmap::QPixmap(int, int)");
		if (!ctor)
			return;
		ctor(m_storage, width, height);
		set_owned(true);
	}

	QPixmap::QPixmap(const QPixmap& other)
	{
		if (!other.owned())
			return;

		static const auto copy_ctor = detail::gui<void (*)(void*, const void*)>("QPixmap::QPixmap(QPixmap const&)");
		if (!copy_ctor)
			return;

		copy_ctor(m_storage, other.m_storage);
		set_owned(true);
	}

	QPixmap::QPixmap(QPixmap&& other) noexcept
	{
		if (!other.owned())
			return;

		adopt(other);
	}

	QPixmap& QPixmap::operator=(const QPixmap& other)
	{
		if (this == &other)
			return *this;

		destroy();

		if (other.owned())
		{
			static const auto copy_ctor = detail::gui<void (*)(void*, const void*)>("QPixmap::QPixmap(QPixmap const&)");
			if (copy_ctor)
			{
				copy_ctor(m_storage, other.m_storage);
				set_owned(true);
			}
		}

		return *this;
	}

	QPixmap& QPixmap::operator=(QPixmap&& other) noexcept
	{
		if (this == &other)
			return *this;

		destroy();

		if (other.owned())
			adopt(other);

		return *this;
	}

	QPixmap::~QPixmap()
	{
		destroy();
	}

	void QPixmap::destroy()
	{
		if (!owned())
			return;

		static const auto dtor = detail::gui<void (*)(void*)>("QPixmap::~QPixmap()");
		if (dtor)
			dtor(m_storage);
		set_owned(false);
	}

	int QPixmap::width() const
	{
		static const auto fn = detail::gui<int (*)(const void*)>("QPixmap::width() const");
		return fn ? fn(m_storage) : 0;
	}

	int QPixmap::height() const
	{
		static const auto fn = detail::gui<int (*)(const void*)>("QPixmap::height() const");
		return fn ? fn(m_storage) : 0;
	}

	void QPixmap::fill(const QColor& color) const
	{
		static const auto fn = detail::gui<void (*)(const void*, const void*)>("QPixmap::fill(QColor const&)");
		if (fn)
			fn(m_storage, color.data());
	}

	bool QPixmap::save(const std::string_view file_path) const
	{
		static const auto fn = detail::gui<bool (*)(const void*, const void*, const char*, int)>("QPixmap::save(QString const&, char const*, int) const");
		if (!fn)
			return false;
		const QString path(file_path);
		return fn(m_storage, path.data(), nullptr, -1);
	}

	QPixmap QPixmap::scaled(const int width, const int height, const AspectMode aspect, const TransformMode transform) const
	{
		static void* const fn = detail::gui_export("QPixmap::scaled(QSize const&, Qt::AspectRatioMode, Qt::TransformationMode) const");

		QPixmap result;
		if (!fn || !owned() || width <= 0 || height <= 0)
			return result;

		const int size[2]{width, height};
		memory::call_returning_member(fn, result.m_storage, static_cast<const void*>(m_storage),
		    static_cast<const void*>(size), static_cast<int>(aspect), static_cast<int>(transform));
		result.set_owned(true);
		return result;
	}

	QPixmap QPixmap::blurred(const double radius) const
	{
		static void* const to_image = detail::gui_export("QPixmap::toImage() const");
		static const auto blur = detail::widgets<void (*)(void* image, double radius, bool quality, int transposed)>("qt_blurImage(QImage&, double, bool, int)");
		static void* const from_image = detail::gui_export("QPixmap::fromImage(QImage const&, QFlags<Qt::ImageConversionFlag>)");
		static const auto image_dtor = detail::gui<void (*)(void*)>("QImage::~QImage()");

		QPixmap result;
		if (!to_image || !blur || !from_image || !owned() || radius <= 0.0)
			return result;

		alignas(void*) unsigned char image_storage[32]{};
		memory::call_returning_member(to_image, image_storage, static_cast<const void*>(m_storage));
		blur(image_storage, radius, true, 0);
		memory::call_returning(from_image, result.m_storage, static_cast<const void*>(image_storage), 0);
		result.set_owned(true);

		if (image_dtor)
			image_dtor(image_storage);

		return result;
	}
}
