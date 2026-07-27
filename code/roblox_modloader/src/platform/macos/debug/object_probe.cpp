#include "object_probe.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/memory/module.hpp"
#include "RobloxModLoader/platform/memory/host_image.hpp"
#include "pointers.hpp"

#include <array>
#include <bit>
#include <cstdint>
#include <mach/mach.h>

RML_LOG_SCOPE("ObjectProbe");

namespace rml::debug
{
	bool safe_read(const void* address, void* out, const std::size_t size)
	{
		if (!address)
			return false;

		vm_size_t read = 0;
		const kern_return_t status = vm_read_overwrite(mach_task_self(),
		    reinterpret_cast<vm_address_t>(address), size, reinterpret_cast<vm_address_t>(out), &read);

		return status == KERN_SUCCESS && read == size;
	}

	static std::string demangle_nested(const char* name)
	{
		if (!name || name[0] != 'N')
			return name ? name : "";

		std::string result;
		for (std::size_t i = 1; name[i] && name[i] != 'E';)
		{
			std::size_t length = 0;
			while (name[i] >= '0' && name[i] <= '9')
				length = length * 10 + static_cast<std::size_t>(name[i++] - '0');

			if (length == 0)
				break;

			if (!result.empty())
				result += "::";
			result.append(name + i, length);
			i += length;
		}
		return result;
	}

	std::string rtti_name(const void* object)
	{
		static const memory::module image{platform::studio_image_name()};
		if (!image.loaded())
			return {};

		const auto base = image.begin().as<std::uintptr_t>();
		const auto end = base + image.size();
		const auto in_image = [&](std::uintptr_t p) { return p >= base && p < end; };

		std::uintptr_t vtable = 0;
		if (!safe_read(object, &vtable, sizeof(vtable)) || !in_image(vtable))
			return {};

		std::uintptr_t type_info = 0;
		if (!safe_read(reinterpret_cast<const void*>(vtable - sizeof(void*)), &type_info, sizeof(type_info)) || !in_image(type_info))
			return {};

		std::uintptr_t name_ptr = 0;
		if (!safe_read(reinterpret_cast<const void*>(type_info + sizeof(void*)), &name_ptr, sizeof(name_ptr)) || !in_image(name_ptr))
			return {};

		std::array<char, 256> name{};
		if (!safe_read(reinterpret_cast<const void*>(name_ptr), name.data(), name.size() - 1))
			return {};

		name.back() = '\0';
		if (name[0] != 'N' && !(name[0] >= '0' && name[0] <= '9'))
			return {};

		return demangle_nested(name.data());
	}

	void dump_object(const char* label, const void* object, const std::size_t span)
	{
		RML_INFO("=== {} object {} ===", label, object);

		for (std::size_t off = 0; off < span; off += sizeof(void*))
		{
			std::uintptr_t value = 0;
			const auto* field = static_cast<const std::uint8_t*>(object) + off;

			if (!safe_read(field, &value, sizeof(value)))
			{
				RML_INFO("  +0x{:02x}  <unreadable>", off);
				continue;
			}

			const std::string cls = rtti_name(reinterpret_cast<const void*>(value));
			if (!cls.empty())
				RML_INFO("  +0x{:02x}  0x{:016x}  -> object <{}>", off, value, cls);
			else
				RML_INFO("  +0x{:02x}  0x{:016x}", off, value);
		}
	}
}

namespace rml::debug
{

}
