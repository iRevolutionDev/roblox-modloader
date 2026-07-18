#pragma once

#include "RobloxModLoader/util/layout_assert.hpp"
#include "data_model_serialize.hpp"
#include "service_provider.hpp"
#include "verb_container.hpp"
#include "workspace.hpp"

#include <cstdint>

namespace RBX
{
	class DataModelJob;

	enum class DataModelType : std::int32_t
	{
		Edit = 0,
		Client = 1,
		Server = 2,
		Standalone = 3,
		Null = 4,
	};

	class DataModel : public ServiceProvider
	{
	public:
		char m_pad_before_workspace[0x150 - sizeof(ServiceProvider)];
		Workspace* workspace;
		char m_pad_before_serialize[0x2A0 - 0x150 - sizeof(void*)];
		DataModelSerialize* data_model_serialize;
		char m_pad_before_type[0x2C4 - 0x2A0 - sizeof(void*)];
		DataModelType type;
		char m_pad_before_verb_container[0x300 - 0x2C4 - sizeof(DataModelType)];
		VerbContainer* verb_container;

		static DataModel* from_job(const DataModelJob* job);

	private:
		RML_LAYOUT_GUARD_BEGIN()
		RML_ASSERT_LAYOUT_OFFSET(DataModel, workspace, 0x150);
		RML_ASSERT_LAYOUT_OFFSET(DataModel, data_model_serialize, 0x2A0);
		RML_ASSERT_LAYOUT_OFFSET(DataModel, type, 0x2C4);
		RML_ASSERT_LAYOUT_OFFSET(DataModel, verb_container, 0x300);
		RML_LAYOUT_GUARD_END()
	};
}
