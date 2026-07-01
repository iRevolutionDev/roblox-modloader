#pragma once
#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/roblox/instance.hpp"
#include "RobloxModLoader/roblox/reflection/property_descriptor.hpp"
#include "RobloxModLoader/roblox/reflection/type.hpp"
#include "RobloxModLoader/util/memory.hpp"
#include "interop_registry.hpp"

#include <cstdint>
#include <memory>
#include <optional>
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

	[[nodiscard]] inline InteropVariant tuple_value(const std::vector<InteropVariant>& values)
	{
		if (values.empty())
			return null_value();

		const size_t count = values.size();
		const size_t buf_size = sizeof(uint64_t) + count * sizeof(InteropVariant);

		auto* buf = static_cast<std::byte*>(std::malloc(buf_size));
		if (!buf)
			return null_value();

		*reinterpret_cast<uint64_t*>(buf) = count;
		std::memcpy(buf + sizeof(uint64_t), values.data(), count * sizeof(InteropVariant));

		InteropVariant out{};
		out.tag = InteropValueTag::Tuple;
		out.as_instance = reinterpret_cast<uintptr_t>(buf);
		return out;
	}

	template<size_t N>
	struct blittable_blob
	{
		std::byte data[N];
	};

	[[nodiscard]] inline size_t blittable_size(const RBX::Name& type_name) noexcept
	{
		static constexpr std::pair<const char*, size_t> table[] = {
		    {"Vector3", 12},
		    {"Vector2", 8},
		    {"Color3", 12},
		    {"CoordinateFrame", 48},
		    {"CFrame", 48},
		    {"UDim", 8},
		    {"UDim2", 16},
		    {"Ray", 24},
		    {"Rect", 16},
		    {"NumberRange", 8},
		    {"Region3", 60},
		    {"Faces", 4},
		    {"Axes", 4},
		    {"BrickColor", 4},
		};

		for (const auto& [name, sz] : table)
		{
			if (type_name == name)
				return sz;
		}
		return 0;
	}

	[[nodiscard]] inline InteropVariant blittable_value(const void* bytes, const size_t size)
	{
		InteropVariant out{};
		out.tag = InteropValueTag::Blittable;
		out.as_instance = 0;
		if (bytes && size)
		{
			if (auto* buf = std::malloc(size))
			{
				std::memcpy(buf, bytes, size);
				out.as_instance = reinterpret_cast<uintptr_t>(buf);
			}
		}
		return out;
	}

	[[nodiscard]] inline std::optional<InteropVariant> try_blittable_property(const RBX::Reflection::PropertyDescriptor* descriptor, const RBX::Reflection::DescribedBase* instance)
	{
		const auto size = blittable_size(descriptor->type.name);
		if (size == 0)
			return std::nullopt;

		RBX::Reflection::Variant variant;
		descriptor->get_variant(instance, variant);
		if (variant.is_void())
			return std::nullopt;

		return blittable_value(variant.try_cast<std::byte>(), size);
	}

	[[nodiscard]] inline bool try_set_blittable_property(const RBX::Reflection::PropertyDescriptor* descriptor, RBX::Reflection::DescribedBase* instance, const InteropVariant& value)
	{
		if (value.tag != InteropValueTag::Blittable || value.as_instance == 0)
			return false;

		const auto size = blittable_size(descriptor->type.name);
		if (size == 0)
			return false;

		const auto* bytes = reinterpret_cast<const void*>(value.as_instance);
		RBX::Property property(*descriptor, instance);

		switch (size)
		{
		case 4: property.set(*static_cast<const blittable_blob<4>*>(bytes)); return true;
		case 8: property.set(*static_cast<const blittable_blob<8>*>(bytes)); return true;
		case 12: property.set(*static_cast<const blittable_blob<12>*>(bytes)); return true;
		case 16: property.set(*static_cast<const blittable_blob<16>*>(bytes)); return true;
		case 24: property.set(*static_cast<const blittable_blob<24>*>(bytes)); return true;
		case 48: property.set(*static_cast<const blittable_blob<48>*>(bytes)); return true;
		case 60: property.set(*static_cast<const blittable_blob<60>*>(bytes)); return true;
		default: return false;
		}
	}

	struct engine_vector_header
	{
		const std::byte* begin;
		const std::byte* end;
		const std::byte* capacity;
	};

	[[nodiscard]] inline size_t sequence_stride(const RBX::Name& type_name) noexcept
	{
		if (type_name == "NumberSequence")
			return 12;
		if (type_name == "ColorSequence")
			return 20;
		return 0;
	}

	[[nodiscard]] inline InteropVariant pack_sequence(const void* vec_storage, const size_t stride)
	{
		InteropVariant out{};
		out.tag = InteropValueTag::Blittable;
		out.as_instance = 0;

		const auto* header = static_cast<const engine_vector_header*>(vec_storage);
		const size_t bytes = (header->end > header->begin) ? static_cast<size_t>(header->end - header->begin) : 0;
		const auto count = static_cast<int32_t>(bytes / stride);

		const size_t buf_size = sizeof(int32_t) + bytes;
		if (auto* buf = static_cast<std::byte*>(std::malloc(buf_size)))
		{
			*reinterpret_cast<int32_t*>(buf) = count;
			if (bytes)
				std::memcpy(buf + sizeof(int32_t), header->begin, bytes);
			out.as_instance = reinterpret_cast<uintptr_t>(buf);
		}
		return out;
	}

	[[nodiscard]] inline std::optional<InteropVariant> try_sequence_property(const RBX::Reflection::PropertyDescriptor* descriptor, const RBX::Reflection::DescribedBase* instance)
	{
		const auto stride = sequence_stride(descriptor->type.name);
		if (stride == 0)
			return std::nullopt;

		RBX::Reflection::Variant variant;
		descriptor->get_variant(instance, variant);
		if (variant.is_void())
			return std::nullopt;

		return pack_sequence(variant.try_cast<std::byte>(), stride);
	}

	[[nodiscard]] inline bool try_set_sequence_property(const RBX::Reflection::PropertyDescriptor* descriptor, RBX::Reflection::DescribedBase* instance, const InteropVariant& value)
	{
		if (value.tag != InteropValueTag::Blittable || value.as_instance == 0)
			return false;

		const auto stride = sequence_stride(descriptor->type.name);
		if (stride == 0)
			return false;

		const auto* buf = reinterpret_cast<const std::byte*>(value.as_instance);
		const auto count = *reinterpret_cast<const int32_t*>(buf);
		const auto* keys = buf + sizeof(int32_t);

		engine_vector_header header{};
		header.begin = keys;
		header.end = keys + static_cast<size_t>(count < 0 ? 0 : count) * stride;
		header.capacity = header.end;

		RBX::Property property(*descriptor, instance);
		property.set(*reinterpret_cast<const blittable_blob<sizeof(engine_vector_header)>*>(&header));
		return true;
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
				RML_WARN_AT("Interop", "Dropping implausible instance handle {:#x} for type '{}'", instance, type.name.c_str());
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

	[[nodiscard]] inline InteropVariant marshal_tuple(const RBX::Reflection::Tuple* tuple)
	{
		if (!tuple || !utils::memory::is_valid_pointer(reinterpret_cast<uintptr_t>(tuple)) || tuple->values.empty())
			return null_value();

		InteropStringPool strings;
		std::vector<InteropVariant> values;
		values.reserve(tuple->values.size());
		for (const auto& value : tuple->values)
			values.push_back(engine_variant_to_interop(value, strings));

		return tuple_value(values);
	}

	inline void write_return_value(const RBX::Reflection::Type* type, const uint64_t ret, [[maybe_unused]] const uint64_t ret_slot, const uintptr_t ret_slot_addr, InteropVariant& out) noexcept
	{
		if (!type)
		{
			out = null_value();
			return;
		}

		if (type->name == "Tuple")
		{
			auto* slot = reinterpret_cast<std::shared_ptr<const RBX::Reflection::Tuple>*>(ret_slot_addr);
			out = marshal_tuple(slot ? slot->get() : nullptr);
			std::destroy_at(slot);
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

		if (const auto size = blittable_size(type->name))
		{
			out = blittable_value(reinterpret_cast<const void*>(ret_slot_addr), size);
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

	template<typename T>
	[[nodiscard]] const T* read_struct_ptr(const InteropVariant& v) noexcept
	{
		return (v.tag == InteropValueTag::Blittable || v.tag == InteropValueTag::Instance) ?
		    reinterpret_cast<const T*>(v.as_instance) :
		    nullptr;
	}
} // namespace rml::dotnet
