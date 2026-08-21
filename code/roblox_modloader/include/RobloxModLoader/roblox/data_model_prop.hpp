#pragma once

#include "slots_holder.hpp"
#include "workspace.hpp"

#include "RobloxModLoader/util/layout_assert.hpp"

#include <memory>
#include <string>

namespace RBX
{
	class DataModelProp
	{
	public:
		std::string place_id_string;
		std::string universe_id_string;
		std::string job_id;
		std::shared_ptr<Workspace> workspace;
		boost::intrusive_ptr<rbx::signals::slots_holder> loaded_slots;
		boost::intrusive_ptr<rbx::signals::slots_holder> graphics_quality_slots;
	};

	RML_LAYOUT_DIAGNOSTIC_PUSH()
	RML_ASSERT_OFFSET(DataModelProp, place_id_string, 0x00);
#if defined(RML_WINDOWS)
	RML_ASSERT_OFFSET(DataModelProp, universe_id_string, 0x20);
	RML_ASSERT_OFFSET(DataModelProp, job_id, 0x40);
	RML_ASSERT_OFFSET(DataModelProp, workspace, 0x60);
	RML_ASSERT_SIZE(DataModelProp, 0x80);
#else
	RML_ASSERT_OFFSET(DataModelProp, universe_id_string, 0x18);
	RML_ASSERT_OFFSET(DataModelProp, job_id, 0x30);
	RML_ASSERT_OFFSET(DataModelProp, workspace, 0x48);
	RML_ASSERT_SIZE(DataModelProp, 0x68);
#endif
	RML_LAYOUT_DIAGNOSTIC_POP()
}
