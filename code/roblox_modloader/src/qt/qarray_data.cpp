#include "RobloxModLoader/qt/qarray_data.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

#include <atomic>

namespace rml::qt::detail
{
	using RefCount = std::atomic<int>;

	static constexpr std::size_t BLOCK_ALIGNMENT = alignof(void*);
	static constexpr int PERSISTENT = -1;

	void release_array_data(void*& d, const std::size_t element_size)
	{
		if (!d)
			return;

		static const auto deallocate = core<void (*)(void*, std::size_t, std::size_t)>(
		    "QArrayData::deallocate(QArrayData*, unsigned long, unsigned long)");

		if (!deallocate)
			return;

		auto* const ref = static_cast<RefCount*>(d);
		const int count = ref->load(std::memory_order_relaxed);

		if (count == PERSISTENT)
		{
			d = nullptr;
			return;
		}

		if (count != 0 && ref->fetch_sub(1, std::memory_order_acq_rel) != 1)
		{
			d = nullptr;
			return;
		}

		deallocate(d, element_size, BLOCK_ALIGNMENT);
		d = nullptr;
	}

	static void destroy_container(void*& d, void* const destructor, const std::size_t element_size)
	{
		if (!d)
			return;

		if (destructor)
		{
			reinterpret_cast<void (*)(void*)>(destructor)(&d);
			d = nullptr;
			return;
		}

		release_array_data(d, element_size);
	}

	void destroy_qstring(void*& d)
	{
		static void* const destructor = core_export_optional("QString::~QString()");
		destroy_container(d, destructor, sizeof(char16_t));
	}

	void destroy_qbytearray(void*& d)
	{
		static void* const destructor = core_export_optional("QByteArray::~QByteArray()");
		destroy_container(d, destructor, sizeof(char));
	}
	
	static constexpr std::size_t OFFSET_FIELD = 16;

	const char* array_data_begin(const void* d)
	{
		if (!d)
			return nullptr;

		const auto* const block = static_cast<const char*>(d);

		std::ptrdiff_t offset = 0;
		std::memcpy(&offset, block + OFFSET_FIELD, sizeof(offset));

		return block + offset;
	}
}
