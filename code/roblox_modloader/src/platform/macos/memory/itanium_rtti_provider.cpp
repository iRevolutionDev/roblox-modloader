#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/memory/i_rtti_provider.hpp"
#include "RobloxModLoader/memory/module.hpp"
#include "RobloxModLoader/platform/memory/host_image.hpp"

#include <string>
#include <vector>

RML_LOG_SCOPE("ItaniumRtti");

namespace rml::memory
{
	static std::string itanium_type_name(const std::string_view class_name)
	{
		std::vector<std::string_view> components;

		for (std::size_t start = 0;;)
		{
			const auto separator = class_name.find("::", start);
			const auto end = separator == std::string_view::npos ? class_name.size() : separator;

			components.push_back(class_name.substr(start, end - start));

			if (separator == std::string_view::npos)
				break;

			start = separator + 2;
		}

		std::string name;
		for (const auto component : components)
		{
			name += std::to_string(component.size());
			name += component;
		}

		return components.size() > 1 ? "N" + name + "E" : name;
	}

	static const char* find_type_name_string(const memory::module& image, const std::string_view type_name)
	{
		const std::string_view haystack{image.begin().as<const char*>(), image.size()};

		std::string needle{type_name};
		needle.push_back('\0');

		const auto position = haystack.find(needle);
		return position == std::string_view::npos ? nullptr : haystack.data() + position;
	}

	static std::vector<const std::uintptr_t*> find_references(const memory::module& image, const void* target)
	{
		const auto* const slots = image.begin().as<const std::uintptr_t*>();
		const std::size_t count = image.size() / sizeof(std::uintptr_t);
		const auto value = reinterpret_cast<std::uintptr_t>(target);

		std::vector<const std::uintptr_t*> references;
		for (std::size_t i = 0; i < count; ++i)
		{
			if (slots[i] == value)
				references.push_back(slots + i);
		}

		return references;
	}

	class ItaniumRttiProvider final : public IRttiProvider
	{
	public:
		std::optional<void**> find_class_vtable(const std::string_view class_name) override
		{
			const module image{platform::studio_image_name()};
			if (!image.loaded())
				return std::nullopt;

			const std::string type_name = itanium_type_name(class_name);

			const char* const name_string = find_type_name_string(image, type_name);
			if (!name_string)
			{
				RML_WARN("No RTTI name '{}' in the image for class '{}'", type_name, class_name);
				return std::nullopt;
			}

			for (const auto* const name_reference : find_references(image, name_string))
			{
				const auto* const type_info = name_reference - 1;

				for (const auto* const type_info_reference : find_references(image, type_info))
				{
					if (*(type_info_reference - 1) != 0)
						continue;

					auto** const vtable = reinterpret_cast<void**>(const_cast<std::uintptr_t*>(type_info_reference + 1));
					RML_DEBUG("RTTI '{}' -> vtable {}", class_name, static_cast<void*>(vtable));
					return vtable;
				}
			}

			RML_WARN("Found the RTTI name for '{}' but no primary vtable pointing at it", class_name);
			return std::nullopt;
		}
	};

	std::unique_ptr<IRttiProvider> create_rtti_provider()
	{
		return std::make_unique<ItaniumRttiProvider>();
	}
}
