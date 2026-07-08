#pragma once

#include "member.hpp"
#include "type.hpp"

#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace RBX::Reflection
{
	class DescribedBase;
	class Property;

	class PropertyDescriptor : public MemberDescriptor
	{
	public:
		typedef Property ConstMember;
		typedef Property Member;

		enum Functionality : unsigned
		{
			STANDARD = 1 + 2 + 4 + 8 + 16,              // isPublic, canReplicate, canXmlRead, canXmlWrite, isScriptable
			NO_XML_WRITE = 1 + 2 + 4 + 0 + 16,          // isPublic, canReplicate, canXmlRead,              isScriptable
			UI = 1 + 0 + 4 + 0 + 16,                    // isPublic,              canXmlRead,              isScriptable
			SCRIPTING = 1 + 2 + 0 + 0 + 16,             // isPublic, canReplicate,                         isScriptable
			STREAMING = 0 + 2 + 4 + 8 + 0,              //           canReplicate, canXmlRead, canXmlWrite
			CLUSTER = 0 + 0 + 4 + 8 + 0,                //                         canXmlRead, canXmlWrite
			LEGACY = 0 + 0 + 4 + 0 + 0,                 //                         canXmlRead
			REPLICATE_ONLY = 0 + 2 + 0 + 0 + 0,         //           canReplicate
			LEGACY_SCRIPTING = 0 + 0 + 4 + 0 + 16,      //                         canXmlRead,              isScriptable
			HIDDEN_SCRIPTING = 0 + 0 + 0 + 0 + 16,      //                                                  isScriptable
			PUBLIC_SERIALIZED = 1 + 0 + 4 + 8 + 0,      // isPublic,              canXmlRead, canXmlWrite
			REPLICATE_CLONE = 0 + 2 + 0 + 0 + 0 + 32,   //        canReplicate,                        alwaysClone
			STANDARD_NO_REPLICATE = 1 + 0 + 4 + 8 + 16, // isPublic,              canXmlRead, canXmlWrite,  isScriptable
			STANDARD_NO_SCRIPTING = 1 + 2 + 4 + 8 + 0,  // isPublic, canReplicate, canXmlRead, canXmlWrite
			PUBLIC_REPLICATE = 1 + 2 + 0 + 0 + 0,       // isPublic, canReplicate
		};

	private:
		char padding[0x28];

	public:
		const Type& type;
		const bool m_is_enum;

	private:
		unsigned m_is_public : 1;
		unsigned m_is_editable : 1;
		unsigned m_can_replicate : 1;
		unsigned m_can_xml_read : 1;
		unsigned m_can_xml_write : 1;
		unsigned m_is_scriptable : 1;
		unsigned m_always_clone : 1;

	public:
		[[nodiscard]] bool is_public() const
		{
			return m_is_public != 0;
		}

		[[nodiscard]] bool is_editable() const
		{
			return m_is_editable != 0;
		}
		[[nodiscard]] bool can_replicate() const
		{
			return m_can_replicate != 0;
		}

		[[nodiscard]] bool can_xml_read() const
		{
			return m_can_xml_read != 0;
		}

		[[nodiscard]] bool can_xml_write() const
		{
			return m_can_xml_write != 0;
		}

		[[nodiscard]] bool is_scriptable() const
		{
			return m_is_scriptable != 0;
		}

		[[nodiscard]] bool always_clone() const
		{
			return m_always_clone != 0;
		}

		bool operator==(const PropertyDescriptor& other) const
		{
			return this == &other;
		}
		bool operator!=(const PropertyDescriptor& other) const
		{
			return this != &other;
		}

		virtual bool is_read_only() const = 0;

		virtual bool is_write_only() const = 0;
		virtual bool has_variant_accessor() const = 0;
		virtual bool can_interpolate() const = 0;

		virtual void get_from_variant_accessor(DescribedBase* instance, Variant& out) const = 0;

		virtual void get_from_variant_accessor(const DescribedBase* instance, Variant& out) const = 0;
		virtual void set_via_variant_accessor(DescribedBase* instance, const Variant& value) const = 0;

		virtual int get_data_size(const DescribedBase* instance) const = 0;

		virtual int _vt09_unknown_() const = 0;
		virtual bool _vt10_unknown_() const = 0;
		virtual bool _vt11_unknown_() const = 0;
		virtual int _vt12_unknown_() const = 0;
		virtual void _vt13_unknown_() const = 0;

		virtual bool equal_values(const DescribedBase* a, const DescribedBase* b) const = 0;

		virtual bool equals_typed_value(const DescribedBase* instance) const = 0;

		virtual void get_variant(const DescribedBase* instance, Variant& out) const = 0;
		virtual void get_variant_xml(const DescribedBase* instance, Variant& out) const = 0;
		virtual void set_variant(DescribedBase* instance, const Variant& value) const = 0;
		virtual void set_variant_xml(DescribedBase* instance, const Variant& value) const = 0;

		virtual void copy_value(const DescribedBase* source, DescribedBase* destination) const = 0;

		virtual int get_raw_data_size(const DescribedBase* instance) const = 0;
		virtual bool is_xml_serializable() const = 0;
		virtual bool has_string_value() const = 0;
		virtual Name get_string_value(const DescribedBase* instance) const;
		virtual bool set_string_value(DescribedBase* instance, const std::string& text) const;
		virtual bool is_type(const Type& type) const = 0;

		virtual void notify_xml_change(const DescribedBase* instance) const = 0;
		virtual void _vt28_unknown_() const = 0;
		virtual void serialize(DescribedBase* instance, unsigned format, void* ctx) const = 0;

		virtual void lua_get(lua_State* L, const DescribedBase* instance) const = 0;
		virtual void lua_set(lua_State* L, DescribedBase* instance) const = 0;

	private:
		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(PropertyDescriptor, 0x78);
			RML_ASSERT_LAYOUT_OFFSET(PropertyDescriptor, padding, 0x40);
			RML_ASSERT_LAYOUT_OFFSET(PropertyDescriptor, m_is_enum, 0x70);
		RML_LAYOUT_GUARD_END()
	};

	template<typename V>
	class TypedPropertyDescriptor : public PropertyDescriptor
	{
	public:
		class GetSet
		{
		public:
			virtual ~GetSet() = default;
			[[nodiscard]] virtual bool is_read_only() const = 0;
			[[nodiscard]] virtual bool is_write_only() const = 0;
			virtual V get(const DescribedBase* instance) const = 0;
			virtual void set(DescribedBase* instance, const V& value) const = 0;
			virtual bool equal_values(const DescribedBase* a, const DescribedBase* b) const = 0;
			virtual bool equals_value(const DescribedBase* instance, const V& value) const = 0;
		};

		class VariantAccessor
		{
		public:
			virtual ~VariantAccessor() = default;
			virtual V get(DescribedBase* instance) const = 0;
			virtual V get(const DescribedBase* instance) const = 0;
			virtual void set(DescribedBase* instance, const V& v) const = 0;
			virtual int data_size(const DescribedBase* instance) const = 0;
			virtual bool feature_check() const = 0;
		};

		class XmlLuaAccessor
		{
		public:
			virtual ~XmlLuaAccessor() = default;
			virtual bool capability_check() const = 0;
			virtual void notify(const DescribedBase* instance) const = 0;
			virtual void serialize(DescribedBase* instance, unsigned fmt, void* ctx) const = 0;
		};

	private:
		char padding[0x18];

	protected:
		std::unique_ptr<GetSet> get_set;
		std::unique_ptr<VariantAccessor> m_variant_accessor;
		std::unique_ptr<XmlLuaAccessor> m_xml_accessor;

	public:
		[[nodiscard]] bool is_read_only() const override
		{
			return get_set ? get_set->is_read_only() : true;
		}
		[[nodiscard]] bool is_write_only() const override
		{
			return get_set ? get_set->is_write_only() : true;
		}

		V get(const DescribedBase* instance) const
		{
			return get_set->get(instance);
		}

		void set(DescribedBase* instance, const V& value) const
		{
			get_set->set(instance, value);
		}
	};

	class ConstProperty
	{
	protected:
		const PropertyDescriptor* descriptor;
		const DescribedBase* instance;

	public:
		ConstProperty() :
		    descriptor(nullptr),
		    instance(nullptr)
		{
		}
		ConstProperty(const PropertyDescriptor& descriptor, const DescribedBase* instance) :
		    descriptor(&descriptor),
		    instance(instance)
		{
		}

		ConstProperty(const ConstProperty& other) = default;

		[[nodiscard]] const DescribedBase* get_instance() const
		{
			return instance;
		}

		[[nodiscard]] const PropertyDescriptor& get_descriptor() const
		{
			return *descriptor;
		}

		ConstProperty& operator=(const ConstProperty& other) = default;

		bool operator==(const ConstProperty& other) const
		{
			return (this->descriptor == other.descriptor) && (this->instance == other.instance);
		}

		[[nodiscard]] const Name& name() const
		{
			return descriptor->name;
		}

		template<typename V>
		V get() const
		{
			return static_cast<const TypedPropertyDescriptor<V>*>(descriptor)->get(instance);
		}

		[[nodiscard]] bool has_string_value() const
		{
			return descriptor->has_string_value();
		}
		[[nodiscard]] std::string get_string_value() const
		{
			return descriptor->get_string_value(instance).c_str();
		}
	};

	class Property : public ConstProperty
	{
	public:
		inline Property(const PropertyDescriptor& descriptor, const DescribedBase* instance) :
		    ConstProperty(descriptor, instance)
		{
		}

		inline Property(const Property& other) :
		    ConstProperty(*other.descriptor, other.instance)
		{
		}
		inline Property& operator=(const Property& other)
		{
			this->descriptor = other.descriptor;
			this->instance = other.instance;
			return *this;
		}

		inline bool operator==(const Property& other) const
		{
			return this->descriptor == other.descriptor && this->instance == other.instance;
		}

		inline bool operator!=(const Property& other) const
		{
			return this->descriptor != other.descriptor || this->instance != other.instance;
		}

		DescribedBase* get_instance() const
		{
			return const_cast<DescribedBase*>(instance);
		}

		template<typename V>
		inline void set(const V& value)
		{
			static_cast<const TypedPropertyDescriptor<V>*>(descriptor)->set(const_cast<DescribedBase*>(instance), value);
		}

		inline bool set_string_value(const std::string& text) const
		{
			return descriptor->set_string_value(const_cast<DescribedBase*>(instance), text);
		}
	};

	class RefPropertyDescriptor : public PropertyDescriptor
	{
		typedef PropertyDescriptor Super;

	public:
		virtual DescribedBase* get_ref_value(const DescribedBase* instance) const = 0;
		virtual void set_ref_value(DescribedBase* instance, DescribedBase* value) const = 0;
		virtual void set_ref_value_unsafe(DescribedBase* instance, DescribedBase* value) const = 0;

		static bool has_string_value()
		{
			return false;
		}

		Name get_string_value(const DescribedBase* instance) const
		{
			return Super::get_string_value(instance);
		}

		bool set_string_value(DescribedBase* instance, const std::string& text) const
		{
			return Super::set_string_value(instance, text);
		}


		static bool is_ref_property_descriptor(const Type& type)
		{
			return type.tag == "Ref";
		}

		static bool is_ref_property_descriptor(const PropertyDescriptor& descriptor)
		{
			return is_ref_property_descriptor(descriptor.type);
		}
	};

	class InstanceHandle
	{
		std::shared_ptr<DescribedBase> m_target;

	public:
		InstanceHandle() = default;
		explicit InstanceHandle(DescribedBase* target);
		explicit InstanceHandle(const std::shared_ptr<DescribedBase>& target) :
		    m_target(target)
		{
		}
		InstanceHandle(const InstanceHandle& other) = default;

		InstanceHandle& operator=(const InstanceHandle& value) = default;
		InstanceHandle& operator=(const std::shared_ptr<DescribedBase>& value)
		{
			m_target = value;
			return *this;
		}

		[[nodiscard]] std::shared_ptr<DescribedBase> target() const
		{
			return m_target;
		}
	};

	class IIDREF
	{
		friend class IReferenceBinder;
		virtual void assign(DescribedBase* propertyOwner, const InstanceHandle& handle) const = 0;
	};
}