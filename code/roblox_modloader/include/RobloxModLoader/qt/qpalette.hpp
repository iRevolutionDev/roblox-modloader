#pragma once

#include "RobloxModLoader/rml_export.hpp"
#include "RobloxModLoader/util/layout_assert.hpp"

namespace rml::qt
{
	class QBrush;

	class RML_EXPORT QPalette
	{
	public:
		enum ColorRole
		{
			Base = 9,
		};

		QPalette() = default;

		explicit QPalette(const void* source);

		QPalette(QPalette&& other) noexcept;
		QPalette& operator=(QPalette&& other) noexcept;
		QPalette(const QPalette&) = delete;
		QPalette& operator=(const QPalette&) = delete;
		~QPalette();

		void set_brush(ColorRole role, const QBrush& brush);

		[[nodiscard]] bool valid() const
		{
			return m_owned;
		}

		[[nodiscard]] void* data() const
		{
			return const_cast<unsigned char*>(m_storage);
		}

	private:
		void destroy();

		static constexpr std::size_t STORAGE_SIZE = 32;

		alignas(void*) unsigned char m_storage[STORAGE_SIZE]{};
		bool m_owned = false;

		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(decltype(m_storage), STORAGE_SIZE);
		RML_LAYOUT_GUARD_END()
	};
}
