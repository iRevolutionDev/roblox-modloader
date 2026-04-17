#pragma once
#include "type.hpp"

#include <functional>
#include <map>

namespace RBX::Reflection
{
	class EnumDescriptor : public Type
	{
	public:
		static std::vector<const EnumDescriptor*>::const_iterator enum_begin();
		static std::vector<const EnumDescriptor*>::const_iterator enum_end();
		static size_t size()
		{
			return all_enums().size();
		}

		class Item : public Descriptor
		{
		public:
			const EnumDescriptor& owner;
			const int value;
			const size_t index;

			bool convert_to(Variant& value) const
			{
				return owner.convert(index, value);
			}

			bool convert_to_string(std::string& value) const
			{
				return owner.convert_to_string(index, value);
			}
		};

		typedef std::map<const Name*, const EnumDescriptor*> EnumNameTable;

	private:
		static EnumNameTable& all_enums_name_lookup();
		static std::vector<const EnumDescriptor*>& all_enums();

		static bool equalValue(const Item* item, const int int_value)
		{
			return item->value == int_value;
		}

		static int count;

	protected:
		std::vector<const Item*> all_items;
		size_t enum_count;
		size_t enum_count_msb;
		EnumDescriptor(const char* typeName);
		~EnumDescriptor();

	public:
		size_t get_enum_count() const
		{
			return enum_count;
		}
		size_t get_enum_count_msb() const
		{
			return enum_count_msb;
		}
		std::vector<const Item*>::const_iterator begin() const
		{
			return all_items.begin();
		}
		std::vector<const Item*>::const_iterator end() const
		{
			return all_items.end();
		}
		static const EnumDescriptor* lookup_descriptor(const Name& name)
		{
			if (const EnumNameTable::const_iterator iter = all_enums_name_lookup().find(&name);
			    iter != all_enums_name_lookup().end())
				return iter->second;
			return nullptr;
		}
		static const EnumDescriptor* lookup_descriptor(const Type& type)
		{
			if (type.is_enum)
				return dynamic_cast<const EnumDescriptor*>(&type);

			return nullptr;
		}

		[[nodiscard]] bool is_value(int intValue) const
		{
			return std::ranges::find_if(all_items, [intValue]<typename T0>(T0&& PH1) {
				return equalValue(std::forward<T0>(PH1), intValue);
			}) != all_items.end();
		}

		virtual const Item* lookup(const char* text) const                     = 0;
		[[nodiscard]] virtual const Item* lookup(const Variant& value) const   = 0;
		virtual bool convert(size_t index, Variant& value) const               = 0;
		virtual bool convert_to_string(size_t index, std::string& value) const = 0;
	};
}