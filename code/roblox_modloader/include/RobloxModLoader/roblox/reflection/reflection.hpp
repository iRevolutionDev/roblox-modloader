#pragma once

#include "object.hpp"

#include <memory>

namespace RBX::Reflection
{
	template<class Class, typename V>
	class PropDescriptor
	{
		template<typename Get, typename Set>
		class GetSetImpl : public TypedPropertyDescriptor<V>::GetSet
		{
			Get m_get;
			Set m_set;

		public:
			GetSetImpl(Get get, Set set) :
			    m_get(get),
			    m_set(set)
			{
			}

			[[nodiscard]] bool is_read_only() const override
			{
				return false;
			}
			[[nodiscard]] bool is_write_only() const override
			{
				return false;
			}

			V get(const DescribedBase* instance) const override
			{
				return (static_cast<const Class*>(instance)->*m_get)();
			}

			void set(DescribedBase* instance, const V& value) const override
			{
				(static_cast<Class*>(instance)->*m_set)(value);
			}

			bool equal_values(const DescribedBase* a, const DescribedBase* b) const override
			{
				return get(a) == get(b);
			}

			bool equals_value(const DescribedBase* instance, const V& value) const override
			{
				return get(instance) == value;
			}
		};

		template<typename Get>
		class GetImpl : public TypedPropertyDescriptor<V>::GetSet
		{
			Get m_get;

		public:
			explicit GetImpl(Get get) :
			    m_get(get)
			{
			}

			[[nodiscard]] bool is_read_only() const override
			{
				return true;
			}
			[[nodiscard]] bool is_write_only() const override
			{
				return false;
			}

			V get(const DescribedBase* instance) const override
			{
				return (static_cast<const Class*>(instance)->*m_get)();
			}

			void set(DescribedBase*, const V&) const override
			{
			}

			bool equal_values(const DescribedBase* a, const DescribedBase* b) const override
			{
				return get(a) == get(b);
			}

			bool equals_value(const DescribedBase* instance, const V& value) const override
			{
				return get(instance) == value;
			}
		};

		template<typename Set>
		class SetImpl : public TypedPropertyDescriptor<V>::GetSet
		{
			Set m_set;

		public:
			explicit SetImpl(Set set) :
			    m_set(set)
			{
			}

			[[nodiscard]] bool is_read_only() const override
			{
				return false;
			}
			[[nodiscard]] bool is_write_only() const override
			{
				return true;
			}

			V get(const DescribedBase*) const override
			{
				return V{};
			}

			void set(DescribedBase* instance, const V& value) const override
			{
				(static_cast<Class*>(instance)->*m_set)(value);
			}

			bool equal_values(const DescribedBase*, const DescribedBase*) const override
			{
				return false;
			}

			bool equals_value(const DescribedBase*, const V&) const override
			{
				return false;
			}
		};

	public:
		template<typename Get, typename Set>
		static std::unique_ptr<typename TypedPropertyDescriptor<V>::GetSet> make_getset(Get get, Set set)
		{
			return std::make_unique<GetSetImpl<Get, Set>>(get, set);
		}

		template<typename Get>
		static std::unique_ptr<typename TypedPropertyDescriptor<V>::GetSet> make_getset(Get get, std::nullptr_t)
		{
			return std::make_unique<GetImpl<Get>>(get);
		}

		template<typename Set>
		static std::unique_ptr<typename TypedPropertyDescriptor<V>::GetSet> make_getset(std::nullptr_t, Set set)
		{
			return std::make_unique<SetImpl<Set>>(set);
		}
	};
    
	template<class Class, typename RefClass>
	class RefPropDescriptor : public RefPropertyDescriptor, public IIDREF
	{
		std::unique_ptr<typename TypedPropertyDescriptor<RefClass*>::GetSet> m_getset;

	public:
		[[nodiscard]] bool is_read_only() const override
		{
			return m_getset ? m_getset->is_read_only() : true;
		}

		[[nodiscard]] bool is_write_only() const override
		{
			return m_getset ? m_getset->is_write_only() : true;
		}

		bool equal_values(const DescribedBase* a, const DescribedBase* b) const override
		{
			return get_value(a) == get_value(b);
		}

		void copy_value(const DescribedBase* source, DescribedBase* destination) const override
		{
			set_value(destination, get_value(source));
		}

		[[nodiscard]] bool is_xml_serializable() const override
		{
			return true;
		}

		[[nodiscard]] bool equals_typed_value(const DescribedBase*) const override
		{
			return false;
		}

		[[nodiscard]] bool is_type(const Type& t) const override
		{
			return RefPropertyDescriptor::is_ref_property_descriptor(t);
		}

		void notify_xml_change(const DescribedBase* /*instance*/) const override
		{
		}

		DescribedBase* get_ref_value(const DescribedBase* instance) const override
		{
			return get_value(instance);
		}

		void set_ref_value(DescribedBase* instance, DescribedBase* value) const override
		{
			set_value(instance, value ? static_cast<RefClass*>(value) : nullptr);
		}

		void set_ref_value_unsafe(DescribedBase* instance, DescribedBase* value) const override
		{
			set_value(instance, static_cast<RefClass*>(value));
		}

		RefClass* get_value(const DescribedBase* object) const
		{
			return m_getset->get(object);
		}

		void set_value(DescribedBase* object, RefClass* value) const
		{
			m_getset->set(object, value);
		}

		template<typename Get, typename Set>
		static std::unique_ptr<typename TypedPropertyDescriptor<RefClass*>::GetSet>
		make_getset(Get get, Set set)
		{
			return PropDescriptor<Class, RefClass*>::make_getset(get, set);
		}

		template<typename Get>
		static std::unique_ptr<typename TypedPropertyDescriptor<RefClass*>::GetSet>
		make_getset(Get get, std::nullptr_t)
		{
			return PropDescriptor<Class, RefClass*>::make_getset(get, nullptr);
		}

		template<typename Set>
		static std::unique_ptr<typename TypedPropertyDescriptor<RefClass*>::GetSet>
		make_getset(std::nullptr_t, Set set)
		{
			return PropDescriptor<Class, RefClass*>::make_getset(nullptr, set);
		}

	private:
		void assign(DescribedBase* owner, const InstanceHandle& handle) const override
		{
			set_value(owner, static_cast<RefClass*>(handle.target().get()));
		}
	};

} // namespace RBX::Reflection