#include "RobloxModLoader/internal/common.hpp"
#include "luau_waiting_script_job.hpp"

#include "RobloxModLoader/luau/script_host.hpp"
#include "RobloxModLoader/luau/script_runtime.hpp"
#include "RobloxModLoader/roblox/data_model.hpp"
#include "RobloxModLoader/roblox/waiting_hybrid_scripts_job.hpp"

#include <unordered_map>

RML_LOG_SCOPE("LuauWaitingScriptJob");

namespace rml::jobs
{
	LuauWaitingScriptJob::LuauWaitingScriptJob() noexcept
		: JobBase(JOB_NAME, JobPriority::High, JobKind::WaitingHybridScripts, true)
	{
	}

	static luau::ScriptHost* host_for_job(const JobExecutionContext& context)
	{
		auto* runtime = luau::script_runtime();
		if (!runtime)
		{
			return nullptr;
		}

		const auto data_model = RBX::DataModel::from_job(context.job_as<RBX::DataModelJob>());
		if (!data_model)
		{
			return nullptr;
		}

		return runtime->host(data_model->type);
	}

	static void report_gate(const char* reason, const int data_model_type)
	{
		static std::unordered_map<std::string, bool> seen;
		if (seen.emplace(std::format("{}:{}", reason, data_model_type), true).second)
		{
			RML_WARN("gate: {} (DataModel type {})", reason, data_model_type);
		}
	}

	static void report_heartbeat(const int data_model_type)
	{
		static std::atomic<std::uint64_t> calls{0};
		const auto count = calls.fetch_add(1, std::memory_order_relaxed) + 1;

		if (count % 200 == 0)
		{
			RML_INFO("gate: still being stepped, {} calls so far, latest DataModel type {}", count, data_model_type);
		}
	}

	bool LuauWaitingScriptJob::should_execute_impl(const JobExecutionContext& context) noexcept
	{
		auto* runtime = luau::script_runtime();
		if (!runtime)
		{
			report_gate("script_runtime() is null", -1);
			return false;
		}

		const auto data_model = RBX::DataModel::from_job(context.job_as<RBX::DataModelJob>());
		if (!data_model)
		{
			report_gate("from_job gave no DataModel", -1);
			return false;
		}

		const auto type = static_cast<int>(data_model->type);
		report_heartbeat(type);

		auto* host = runtime->host(data_model->type);
		if (!host)
		{
			report_gate("no host bound", type);
			return false;
		}

		if (!host->has_pending())
		{
			return false;
		}

		RML_INFO("gate: passing, {} item(s) pending for DataModel type {}", host->dispatcher().pending_count(), type);
		return true;
	}

	void LuauWaitingScriptJob::execute_impl(const JobExecutionContext& context)
	{
		if (auto* host = host_for_job(context))
		{
			host->pump(luau::Budget{});
		}
	}

	void LuauWaitingScriptJob::destroy_impl() noexcept
	{
	}
}
