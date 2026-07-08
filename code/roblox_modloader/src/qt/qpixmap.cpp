#include "RobloxModLoader/qt/qpixmap.hpp"

#include "RobloxModLoader/qt/qcolor.hpp"
#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QPixmap::QPixmap(const std::string_view file_path)
	{
		static const auto ctor = detail::gui<void (*)(void*, const void*, const char*, int)>("??0QPixmap@@QEAA@AEBVQString@@PEBDV?$QFlags@W4ImageConversionFlag@Qt@@@@@Z");
		if (!ctor)
			return;

		const QString path(file_path);
		ctor(m_storage, path.data(), nullptr, 0);
		set_owned(true);
	}

	QPixmap::QPixmap(const int width, const int height)
	{
		static const auto ctor = detail::gui<void (*)(void*, int, int)>("??0QPixmap@@QEAA@HH@Z");
		if (!ctor)
			return;
		ctor(m_storage, width, height);
		set_owned(true);
	}

	QPixmap::QPixmap(const QPixmap& other)
	{
		if (!other.owned())
			return;

		static const auto copy_ctor = detail::gui<void (*)(void*, const void*)>("??0QPixmap@@QEAA@AEBV0@@Z");
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
			static const auto copy_ctor = detail::gui<void (*)(void*, const void*)>("??0QPixmap@@QEAA@AEBV0@@Z");
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

		static const auto dtor = detail::gui<void (*)(void*)>("??1QPixmap@@UEAA@XZ");
		if (dtor)
			dtor(m_storage);
		set_owned(false);
	}

	int QPixmap::width() const
	{
		static const auto fn = detail::gui<int (*)(const void*)>("?width@QPixmap@@QEBAHXZ");
		return fn ? fn(m_storage) : 0;
	}

	int QPixmap::height() const
	{
		static const auto fn = detail::gui<int (*)(const void*)>("?height@QPixmap@@QEBAHXZ");
		return fn ? fn(m_storage) : 0;
	}

	void QPixmap::fill(const QColor& color) const
	{
		static const auto fn = detail::gui<void (*)(const void*, const void*)>("?fill@QPixmap@@QEAAXAEBVQColor@@@Z");
		if (fn)
			fn(m_storage, color.data());
	}

	bool QPixmap::save(const std::string_view file_path) const
	{
		static const auto fn = detail::gui<bool (*)(const void*, const void*, const char*, int)>("?save@QPixmap@@QEBA_NAEBVQString@@PEBDH@Z");
		if (!fn)
			return false;
		const QString path(file_path);
		return fn(m_storage, path.data(), nullptr, -1);
	}

	QPixmap QPixmap::scaled(const int width, const int height, const AspectMode aspect, const TransformMode transform) const
	{
		static const auto fn = detail::gui<void (*)(const void* self, void* sret, int, int, int, int)>("?scaled@QPixmap@@QEBA?AV1@HHW4AspectRatioMode@Qt@@W4TransformationMode@3@@Z");

		QPixmap result;
		if (!fn || !owned() || width <= 0 || height <= 0)
			return result;

		fn(m_storage, result.m_storage, width, height, static_cast<int>(aspect), static_cast<int>(transform));
		result.set_owned(true);
		return result;
	}

	QPixmap QPixmap::blurred(const double radius) const
	{
		static const auto to_image = detail::gui<void (*)(const void* self, void* sret)>("?toImage@QPixmap@@QEBA?AVQImage@@XZ");
		static const auto blur = detail::widgets<void (*)(void* image, double radius, bool quality, int transposed)>("?qt_blurImage@@YAXAEAVQImage@@N_NH@Z");
		static const auto from_image = detail::gui<void (*)(void* sret, const void* image, int flags)>("?fromImage@QPixmap@@SA?AV1@AEBVQImage@@V?$QFlags@W4ImageConversionFlag@Qt@@@@@Z");
		static const auto image_dtor = detail::gui<void (*)(void*)>("??1QImage@@UEAA@XZ");

		QPixmap result;
		if (!to_image || !blur || !from_image || !owned() || radius <= 0.0)
			return result;

		alignas(void*) unsigned char image_storage[32]{};
		to_image(m_storage, image_storage);
		blur(image_storage, radius, true, 0);
		from_image(result.m_storage, image_storage, 0);
		result.set_owned(true);

		if (image_dtor)
			image_dtor(image_storage);

		return result;
	}
}
