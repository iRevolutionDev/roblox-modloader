#pragma once
#include "object.hpp"

#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace RBX
{
	using namespace Reflection;

	class Instance;
	class Name;
	class ServiceProvider;
	class AncestorChanged;
	class ScalablePropertySource;
	class ScalablePropertyTarget;

	enum class CreatorRole : std::int32_t;

	namespace Reflection
	{
		class StyledProperties;
	}

	using Instances = std::vector<std::shared_ptr<Instance>>;

	class InstanceProp
	{
	public:
		void* reserved;
	};

	class Instance : public Object
	{
	public:
		virtual bool styled_properties_read() const = 0;

		virtual void set_styled_properties(
		    boost::intrusive_ptr<Reflection::StyledProperties> properties) = 0;

		virtual void set_is_archivable(bool archivable) = 0;

		virtual bool is_property_hidden_in_studio(
		    const Reflection::PropertyDescriptor* descriptor) const = 0;

		virtual bool is_enum_hidden_in_studio(const Reflection::PropertyDescriptor* descriptor,
		    int value) const = 0;

		virtual bool is_instance_ignored_by_change_history() const = 0;

		virtual void* get_override_instances(
		    const Reflection::PropertyDescriptor* descriptor) const = 0;

		virtual void* get_override_instance_properties(
		    const Reflection::PropertyDescriptor* descriptor) const = 0;

		virtual int get_property_status_in_studio_internal() const = 0;

		virtual void destroy() = 0;

		virtual void humanoid_changed() = 0;

		virtual bool has_scalable_properties() = 0;

		virtual void get_scalable_properties(ScalablePropertyTarget& target) = 0;

		virtual void set_scalable_properties(ScalablePropertySource& source) = 0;

		virtual void scale_scalable_properties(ScalablePropertySource& source,
		    ScalablePropertyTarget& target, float scale) = 0;

		virtual std::shared_ptr<Instance> lua_clone() = 0;

		virtual int get_persistent_data_cost_internal() const = 0;

		virtual bool can_serialize_user_data() const = 0;

		virtual bool can_client_create() = 0;

		virtual void on_service_provider(ServiceProvider* old_provider,
		    ServiceProvider* new_provider) = 0;

		virtual std::shared_ptr<Instance> create_child(const Name& class_name, CreatorRole role) = 0;

		virtual bool verify_set_parent(const Instance* new_parent) const = 0;

		virtual bool verify_set_ancestor(const Instance* new_ancestor,
		    const Instance* old_ancestor) const = 0;

		virtual void signal_completeness_change() = 0;

		virtual int get_override_image_index() const = 0;

		virtual bool get_overflow_modified_bit(unsigned int index) const = 0;

		virtual void set_overflow_modified_bit(unsigned int index, bool value) = 0;

		virtual void clear_overflow_modified() = 0;

		virtual bool verify_add_child(const Instance* child) const = 0;

		virtual bool verify_add_descendant(const Instance* descendant,
		    const Instance* ancestor) const = 0;

		virtual bool ask_add_child(const Instance* child) const = 0;

		virtual bool ask_forbid_child(const Instance* child) const = 0;

		virtual bool ask_forbid_parent(const Instance* parent) const = 0;

		virtual bool ask_set_parent(const Instance* parent) const = 0;

		virtual void on_ancestor_changed(const AncestorChanged& change) = 0;

		virtual void pre_on_ancestor_changed(const AncestorChanged& change) = 0;

		virtual void on_descendant_added(Instance* descendant) = 0;

		virtual void on_descendant_removing(const std::shared_ptr<Instance>& descendant) = 0;

		virtual void on_child_added(Instance* child) = 0;

		virtual void on_child_removing(Instance* child) = 0;

		virtual void on_child_removed(Instance* child) = 0;

		virtual void on_child_changed(Instance* child,
		    const Reflection::PropertyDescriptor& descriptor) = 0;

		virtual void on_descendant_changed(const std::shared_ptr<Instance>& descendant,
		    const Reflection::PropertyDescriptor& descriptor) = 0;

		virtual void on_styled_properties_changed(
		    boost::intrusive_ptr<Reflection::StyledProperties> old_properties,
		    boost::intrusive_ptr<Reflection::StyledProperties> new_properties) = 0;

		virtual bool get_styled_impl(const std::string& key,
		    const std::optional<std::string>& modifier) const = 0;

		virtual bool filter_name_callback_for_subclass(const std::string& name) = 0;

		virtual std::string transform_name_callback(std::string name) = 0;

		virtual void pre_equality_check_name_callback_for_subclass(const std::string& name) = 0;

		virtual void pre_prop_signal_name_callback_for_subclass() = 0;

		virtual void post_prop_signal_name_callback_for_subclass() = 0;

		virtual void post_equality_check_name_callback_for_subclass() = 0;

		InstanceProp prop;
		Instance* parent;
		Flyweight<std::string> name;
		std::shared_ptr<std::vector<std::shared_ptr<Instance>>> children;

	private:
		std::byte reserved_88[0x28];

	public:
		template<typename T = Instance>
		T* as()
		{
			return static_cast<T*>(this);
		}

		std::string compute_full_name();
	};

	RML_LAYOUT_DIAGNOSTIC_PUSH()
	RML_ASSERT_OFFSET(Instance, prop, 0x60);
	RML_ASSERT_OFFSET(Instance, parent, 0x68);
	RML_ASSERT_OFFSET(Instance, name, 0x70);
	RML_ASSERT_OFFSET(Instance, children, 0x78);
	RML_ASSERT_SIZE(Instance, 0xB0);
	RML_LAYOUT_DIAGNOSTIC_POP()
}
