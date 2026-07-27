#pragma once

#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace RBX
{
	class DataModel;

	struct Stats
	{
		double now;
		double last_step;
		double delta_time;
	};

	struct Error
	{
		double error;
	};

	class TaskSchedulerJob
	{
	protected:
		virtual ~TaskSchedulerJob() = default;

		virtual void unknown1() = 0;

		virtual void unknown2() = 0;

		virtual void unknown3() = 0;

		virtual void unknown4() = 0;

	public:
		std::shared_ptr<TaskSchedulerJob> self;
		std::string name;
		std::shared_ptr<DataModel> data_model;

		virtual void destroy(bool delete_after) = 0;

	private:
		RML_LAYOUT_GUARD_BEGIN()
		RML_ASSERT_LAYOUT_OFFSET(TaskSchedulerJob, self, 0x8);
		RML_ASSERT_LAYOUT_OFFSET(TaskSchedulerJob, name, 0x18);
#if defined(RML_WINDOWS)
		RML_ASSERT_LAYOUT_OFFSET(TaskSchedulerJob, data_model, 0x38);
		RML_ASSERT_LAYOUT_SIZE(TaskSchedulerJob, 0x48);
#else
		RML_ASSERT_LAYOUT_OFFSET(TaskSchedulerJob, data_model, 0x30);
		RML_ASSERT_LAYOUT_SIZE(TaskSchedulerJob, 0x40);
#endif
		RML_LAYOUT_GUARD_END()
	};
}
