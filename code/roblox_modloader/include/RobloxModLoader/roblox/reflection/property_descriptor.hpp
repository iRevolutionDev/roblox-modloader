#pragma once

#include "member.hpp"
#include "type.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace RBX::Reflection
{
	class DescribedBase;
	class Property;

	class PropertyDescriptor : public MemberDescriptor
	{
		unsigned m_is_public : 1;
		unsigned m_is_editable : 1;
		unsigned m_can_replicate : 1;
		unsigned m_can_xml_read : 1;
		unsigned m_can_xml_write : 1;
		unsigned m_is_scriptable : 1;
		unsigned m_always_clone : 1;

	public:
		typedef Property ConstMember;
		typedef Property Member;

		enum Functionality : unsigned
		{
			STANDARD              = 1 + 2 + 4 + 8 + 16, // isPublic, canReplicate, canXmlRead, canXmlWrite, isScriptable
			NO_XML_WRITE          = 1 + 2 + 4 + 0 + 16, // isPublic, canReplicate, canXmlRead,              isScriptable
			UI                    = 1 + 0 + 4 + 0 + 16, // isPublic,              canXmlRead,              isScriptable
			SCRIPTING             = 1 + 2 + 0 + 0 + 16, // isPublic, canReplicate,                         isScriptable
			STREAMING             = 0 + 2 + 4 + 8 + 0,  //           canReplicate, canXmlRead, canXmlWrite
			CLUSTER               = 0 + 0 + 4 + 8 + 0,  //                         canXmlRead, canXmlWrite
			LEGACY                = 0 + 0 + 4 + 0 + 0,  //                         canXmlRead
			REPLICATE_ONLY        = 0 + 2 + 0 + 0 + 0,  //           canReplicate
			LEGACY_SCRIPTING      = 0 + 0 + 4 + 0 + 16, //                         canXmlRead,              isScriptable
			HIDDEN_SCRIPTING      = 0 + 0 + 0 + 0 + 16, //                                                  isScriptable
			PUBLIC_SERIALIZED     = 1 + 0 + 4 + 8 + 0,  // isPublic,              canXmlRead, canXmlWrite
			REPLICATE_CLONE       = 0 + 2 + 0 + 0 + 0 + 32, //        canReplicate,                        alwaysClone
			STANDARD_NO_REPLICATE = 1 + 0 + 4 + 8 + 16, // isPublic,              canXmlRead, canXmlWrite,  isScriptable
			STANDARD_NO_SCRIPTING = 1 + 2 + 4 + 8 + 0,  // isPublic, canReplicate, canXmlRead, canXmlWrite
			PUBLIC_REPLICATE      = 1 + 2 + 0 + 0 + 0,  // isPublic, canReplicate
		};

		const Type& type;
		const bool m_is_enum;

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

		virtual bool is_read_only() const  = 0;
		virtual bool is_write_only() const = 0;

		virtual bool equal_values(const DescribedBase* a, const DescribedBase* b) const = 0;

		virtual void get_variant(const DescribedBase* instance, Variant& value) const = 0;
		virtual void set_variant(DescribedBase* instance, const Variant& value) const = 0;

		virtual void copy_value(const DescribedBase* source, DescribedBase* destination) const = 0;

		virtual int get_data_size(const DescribedBase* instance) const = 0;

		virtual bool has_string_value() const                                                 = 0;
		virtual std::string get_string_value(const DescribedBase* instance) const             = 0;
		virtual bool set_string_value(DescribedBase* instance, const std::string& text) const = 0;
	};

	template<typename V>
	class TypedPropertyDescriptor : public PropertyDescriptor
	{
	public:
		class GetSet
		{
		public:
			virtual ~GetSet()                                                               = default;
			[[nodiscard]] virtual bool is_read_only() const                                 = 0;
			[[nodiscard]] virtual bool is_write_only() const                                = 0;
			virtual V get(const DescribedBase* object) const                                = 0;
			virtual void set(DescribedBase* object, const V& value) const                   = 0;
			virtual bool equal_values(const DescribedBase* a, const DescribedBase* b) const = 0;
			virtual bool equals_value(const DescribedBase* instance, const V& value) const  = 0;
		};

	private:
		char padding[0x20];

	protected:
		std::unique_ptr<GetSet> get_set;

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
			return descriptor->get_string_value(instance);
		}
	};

	class Property : public ConstProperty
	{
	public:
		inline Property(const PropertyDescriptor& descriptor, DescribedBase* instance) :
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
			this->instance   = other.instance;
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
}