#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <string_view>

namespace rml::qt
{
	class QColor;

	class RML_EXPORT QPixmap
	{
	public:
		enum class AspectMode
		{
			Ignore = 0,
			Keep = 1,
			KeepByExpanding = 2,
		};

		enum class TransformMode
		{
			Fast = 0,
			Smooth = 1,
		};

		QPixmap() = default;

		explicit QPixmap(std::string_view file_path);
		QPixmap(int width, int height);

		QPixmap(const QPixmap& other);
		QPixmap(QPixmap&& other) noexcept;
		QPixmap& operator=(const QPixmap& other);
		QPixmap& operator=(QPixmap&& other) noexcept;
		~QPixmap();

		[[nodiscard]] bool loaded() const
		{
			return m_loaded;
		}

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		void fill(const QColor& color) const;

		bool save(std::string_view file_path) const;

		[[nodiscard]] QPixmap scaled(int width, int height, AspectMode aspect = AspectMode::Ignore,
		                             TransformMode transform = TransformMode::Smooth) const;

		[[nodiscard]] QPixmap blurred(double radius) const;

		[[nodiscard]] void* data() const
		{
			return const_cast<unsigned char*>(m_storage);
		}

	private:
		friend class QMovie;

		void destroy();

		alignas(void*) unsigned char m_storage[32]{};
		bool m_loaded = false;
	};
}
