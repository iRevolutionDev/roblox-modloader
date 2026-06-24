#pragma once
#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/roblox/instance.hpp"
#include "RobloxModLoader/roblox/reflection/property_descriptor.hpp"
#include "RobloxModLoader/roblox/reflection/type.hpp"
#include "RobloxModLoader/util/memory.hpp"
#include "interop_registry.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace rml::dotnet
{
	using InteropStringPool = std::vector<char*>;

	[[nodiscard]] inline InteropVariant null_value() noexcept
	{
		InteropVariant out{};
		out.tag = InteropValueTag::Null;
		out.as_uint64 = 0;
		return out;
	}

	[[nodiscard]] inline InteropVariant instance_value(const uintptr_t ptr) noexcept
	{
		if (!utils::memory::is_valid_pointer(ptr))
			return null_value();

		InteropVariant out{};
		out.tag = InteropValueTag::Instance;
		out.as_instance = ptr;
		return out;
	}

	[[nodiscard]] inline InteropVariant bool_value(const bool value) noexcept
	{
		InteropVariant out{};
		out.tag = InteropValueTag::Bool;
		out.as_bool = value;
		return out;
	}

	[[nodiscard]] inline InteropVariant int64_value(const int64_t value) noexcept
	{
		InteropVariant out{};
		out.tag = InteropValueTag::Int64;
		out.as_int64 = value;
		return out;
	}

	[[nodiscard]] inline InteropVariant float_value(const float value) noexcept
	{
		InteropVariant out{};
		out.tag = InteropValueTag::Float;
		out.as_float = value;
		return out;
	}

	[[nodiscard]] inline InteropVariant double_value(const double value) noexcept
	{
		InteropVariant out{};
		out.tag = InteropValueTag::Double;
		out.as_double = value;
		return out;
	}

	[[nodiscard]] inline InteropVariant string_value(const char* text)
	{
		InteropVariant out{};
		out.tag = InteropValueTag::String;
		out.as_string = strdup(text ? text : "");
		return out;
	}

	[[nodiscard]] inline InteropVariant string_value(const char* text, InteropStringPool& pool)
	{
		const auto out = string_value(text);
		if (out.as_string)
			pool.push_back(out.as_string);
		return out;
	}

	inline InteropVariant engine_variant_to_interop(const RBX::Reflection::Variant& variant, InteropStringPool& strings)
	{
		if (variant.is_void())
			return null_value();

		const auto& type = variant.type();

		if (RBX::Reflection::RefPropertyDescriptor::is_ref_property_descriptor(type))
		{
			const auto* shared = variant.try_cast<std::shared_ptr<RBX::Instance>>();
			const auto instance = shared ? reinterpret_cast<uintptr_t>(shared->get()) : 0;
			if (!utils::memory::is_valid_pointer(instance))
			{
				RML_WARN_AT("Interop", "Dropping implausible instance handle {:#x} for type '{}'",
				    instance,
				    type.name.c_str());
				return null_value();
			}
			return instance_value(instance);
		}

		if (type.name == "Instance")
		{
			const auto* instance = variant.try_cast<RBX::Instance*>();
			return instance ? instance_value(reinterpret_cast<uintptr_t>(*instance)) : null_value();
		}

		if (type.name == "string")
			return string_value(variant.try_cast<std::string>()->c_str(), strings);

		if (type.name == "bool")
			return bool_value(*variant.try_cast<bool>());

		if (variant.is_enum())
			return int64_value(*variant.try_cast<int>());

		if (variant.is_float())
			return type.name == "float" ? float_value(*variant.try_cast<float>()) : double_value(*variant.try_cast<double>());

		if (variant.is_number())
			return int64_value(type.name == "int" ? *variant.try_cast<int>() : *variant.try_cast<int64_t>());

		RML_WARN_AT("Interop", "Unsupported event argument type '{}'", type.name.c_str());
		return null_value();
	}

	inline void write_return_value(const RBX::Reflection::Type* type, const uint64_t ret, [[maybe_unused]] const uint64_t ret_slot, const uintptr_t ret_slot_addr, InteropVariant& out) noexcept
	{
		if (!type)
		{
			out = null_value();
			return;
		}

		if (type->name == "Instances")
		{
			out.tag = InteropValueTag::InstanceArray;
			out.as_instance = 0;

			auto* slot = reinterpret_cast<std::shared_ptr<RBX::Instances>*>(ret_slot_addr);
			if (const auto& instances_ptr = *slot; instances_ptr && !instances_ptr->empty())
			{
				const auto& instances = *instances_ptr;
				const auto count = static_cast<uint32_t>(instances.size());
				const auto buf_size = sizeof(uint64_t) + static_cast<size_t>(count) * sizeof(uintptr_t);

				if (auto* buf = static_cast<uint8_t*>(std::malloc(buf_size)))
				{
					auto* count_field = reinterpret_cast<uint32_t*>(buf);
					auto* handles = reinterpret_cast<uintptr_t*>(buf + sizeof(uint64_t));
					uint32_t written = 0;

					for (const auto& element : instances)
					{
						if (const auto handle = reinterpret_cast<uintptr_t>(element.get()))
							handles[written++] = handle;
					}
					*count_field = written;
					out.as_instance = reinterpret_cast<uintptr_t>(buf);
				}
			}

			std::destroy_at(slot);
			return;
		}

		if (type->name == "Instance" || RBX::Reflection::RefPropertyDescriptor::is_ref_property_descriptor(*type))
		{
			auto* slot = reinterpret_cast<std::shared_ptr<RBX::Instance>*>(ret_slot_addr);
			out = instance_value(reinterpret_cast<uintptr_t>(slot->get()));
			std::destroy_at(slot);
			return;
		}

		if (type->name == "string")
		{
			auto* slot = reinterpret_cast<std::string*>(ret_slot_addr);
			out = string_value(slot->c_str());
			std::destroy_at(slot);
			return;
		}

		if (!ret)
		{
			out = null_value();
			return;
		}

		out = int64_value(static_cast<int64_t>(ret));
	}

	[[nodiscard]] inline bool read_bool(const InteropVariant& v, bool& out) noexcept
	{
		switch (v.tag)
		{
		case InteropValueTag::Bool: out = v.as_bool; return true;
		case InteropValueTag::Int64: out = v.as_int64 != 0; return true;
		default: return false;
		}
	}

	[[nodiscard]] inline bool read_int64(const InteropVariant& v, int64_t& out) noexcept
	{
		switch (v.tag)
		{
		case InteropValueTag::Int64: out = v.as_int64; return true;
		case InteropValueTag::Bool: out = v.as_bool ? 1 : 0; return true;
		default: return false;
		}
	}

	[[nodiscard]] inline bool read_double(const InteropVariant& v, double& out) noexcept
	{
		switch (v.tag)
		{
		case InteropValueTag::Double: out = v.as_double; return true;
		case InteropValueTag::Float: out = v.as_float; return true;
		case InteropValueTag::Int64: out = static_cast<double>(v.as_int64); return true;
		default: return false;
		}
	}

	[[nodiscard]] inline const char* read_string(const InteropVariant& v) noexcept
	{
		return v.tag == InteropValueTag::String ? v.as_string : nullptr;
	}

	template<typename T>
	[[nodiscard]] T* read_instance(const InteropVariant& v) noexcept
	{
		return v.tag == InteropValueTag::Instance ? reinterpret_cast<T*>(v.as_instance) : nullptr;
	}
} // namespace rml::dotnet
