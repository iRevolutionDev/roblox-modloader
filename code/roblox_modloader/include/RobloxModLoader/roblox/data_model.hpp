#pragma once

#include "RobloxModLoader/util/layout_assert.hpp"
#include "data_model_serialize.hpp"
#include "service_provider.hpp"
#include "verb_container.hpp"
#include "workspace.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

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
		std::byte pad_before_provider_map[0x10];
		std::map<std::uintptr_t, std::uintptr_t> provider_map;
		std::byte pad_after_provider_map[0x20];
		void* named_base_vftable;
		std::string reserved_string_0;
		std::string reserved_string_1;
		std::string reserved_string_2;

	public:
		std::shared_ptr<Workspace> workspace;

	private:
		std::byte pad_before_reserved_string_3[0x128];
		std::string reserved_string_3;

	public:
		std::shared_ptr<DataModelSerialize> data_model_serialize;

	private:
		std::byte pad_before_type[0x48];

	public:
		DataModelType type;

	private:
		std::byte pad_before_verb_container[0x4];

	public:
		VerbContainer* verb_container;

		static DataModel* from_job(const DataModelJob* job);

	private:
		RML_LAYOUT_GUARD_BEGIN()
		RML_ASSERT_LAYOUT_OFFSET(DataModel, workspace, 0x160);
		RML_ASSERT_LAYOUT_OFFSET(DataModel, data_model_serialize, 0x2B8);
		RML_ASSERT_LAYOUT_OFFSET(DataModel, type, 0x310);
		RML_ASSERT_LAYOUT_OFFSET(DataModel, verb_container, 0x318);
		RML_LAYOUT_GUARD_END()
	};
}
