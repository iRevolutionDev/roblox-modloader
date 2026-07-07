#include "application.hpp"

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/exception/crash_dumper.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/memory/rtti_scanner.hpp"
#include "RobloxModLoader/mod/events.hpp"
#include "RobloxModLoader/qt/qt_integration.hpp"
#include "RobloxModLoader/roblox/job_manager.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/script_subsystem.hpp"
#include "mod/mod_manager.hpp"
#include "pointers.hpp"
#include "utils/directory.hpp"

RML_LOG_SCOPE("Application");

namespace rml
{
	class ConfigSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			const auto config_path = utils::directory::get_mod_loader_directory() / "config.toml";
			if (const auto config_result = config::initialize(config_path, true); !config_result)
			{
				return std::unexpected(SubsystemError{std::string(name()), "Failed to initialize configuration system"});
			}

			return {};
		}

		void shutdown() override
		{
			config::shutdown();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "Config";
		}
	};

	class LoggerSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			::logger::init();
			return {};
		}

		void shutdown() override
		{
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "Logger";
		}
	};

	class CrashDumperSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<exception_filter::CrashDumper>();
			m_instance->enable();
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "CrashDumper";
		}

	private:
		std::unique_ptr<exception_filter::CrashDumper> m_instance;
	};

	class EventManagerSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<events::EventManager>();
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "EventManager";
		}

		[[nodiscard]] events::EventManager& event_manager() const
		{
			return *m_instance;
		}

	private:
		std::unique_ptr<events::EventManager> m_instance;
	};

	class QtIntegrationSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<qt::QtIntegration>();
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "QtIntegration";
		}

	private:
		std::unique_ptr<qt::QtIntegration> m_instance;
	};

	class RttiManagerSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			try
			{
				m_instance = std::make_unique<::memory::rtti::rtti_manager>();
			}
			catch (const std::exception& e)
			{
				return std::unexpected(SubsystemError{std::string(name()), e.what()});
			}

			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "RttiManager";
		}

	private:
		std::unique_ptr<::memory::rtti::rtti_manager> m_instance;
	};

	class PointersSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			try
			{
				m_instance = std::make_unique<::pointers>();
			}
			catch (const std::exception& e)
			{
				return std::unexpected(SubsystemError{std::string(name()), e.what()});
			}

			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "Pointers";
		}

	private:
		std::unique_ptr<::pointers> m_instance;
	};

	class TaskSchedulerSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<RBX::TaskScheduler>();
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "TaskScheduler";
		}

		[[nodiscard]] ITaskScheduler& task_scheduler() const
		{
			return *m_instance;
		}

	private:
		std::unique_ptr<RBX::TaskScheduler> m_instance;
	};

	class JobManagerSubsystem final : public ISubsystem
	{
	public:
		explicit JobManagerSubsystem(TaskSchedulerSubsystem& task_scheduler_subsystem) :
		    m_task_scheduler_subsystem(task_scheduler_subsystem)
		{
		}

		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<jobs::JobManager>(m_task_scheduler_subsystem.task_scheduler());
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "JobManager";
		}

	private:
		TaskSchedulerSubsystem& m_task_scheduler_subsystem;
		std::unique_ptr<jobs::JobManager> m_instance;
	};

	class HookingSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<::hooking>();
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "Hooking";
		}

	private:
		std::unique_ptr<::hooking> m_instance;
	};

	class ScriptSubsystemAdapter final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<ScriptSubsystem>();
			m_instance->initialize();
			return {};
		}

		void shutdown() override
		{
			if (m_instance)
			{
				m_instance->shutdown();
			}

			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "ScriptSubsystem";
		}

	private:
		std::unique_ptr<ScriptSubsystem> m_instance;
	};

	class ModManagerSubsystem final : public ISubsystem
	{
	public:
		explicit ModManagerSubsystem(EventManagerSubsystem& event_manager_subsystem) :
		    m_event_manager_subsystem(event_manager_subsystem)
		{
		}

		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<ModManager>();

			if (const auto result = m_instance->initialize(m_event_manager_subsystem.event_manager()); !result)
			{
				return std::unexpected(SubsystemError{std::string(name()), result.error().message});
			}

			return {};
		}

		void shutdown() override
		{
			if (m_instance)
			{
				m_instance->shutdown();
			}

			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "ModManager";
		}

	private:
		EventManagerSubsystem& m_event_manager_subsystem;
		std::unique_ptr<ModManager> m_instance;
	};

	Application::Application() = default;

	Application::~Application()
	{
		if (!m_shutdown_complete)
		{
			shutdown();
		}
	}

	std::expected<void, SubsystemError> Application::initialize()
	{
		auto event_manager_subsystem = std::make_unique<EventManagerSubsystem>();
		auto& event_manager_subsystem_ref = *event_manager_subsystem;

		auto task_scheduler_subsystem = std::make_unique<TaskSchedulerSubsystem>();
		auto& task_scheduler_subsystem_ref = *task_scheduler_subsystem;

		m_subsystems.push_back(std::make_unique<ConfigSubsystem>());
		m_subsystems.push_back(std::make_unique<LoggerSubsystem>());
		m_subsystems.push_back(std::make_unique<CrashDumperSubsystem>());
		m_subsystems.push_back(std::move(event_manager_subsystem));
		m_subsystems.push_back(std::make_unique<QtIntegrationSubsystem>());
		m_subsystems.push_back(std::make_unique<RttiManagerSubsystem>());
		m_subsystems.push_back(std::make_unique<PointersSubsystem>());
		m_subsystems.push_back(std::move(task_scheduler_subsystem));
		m_subsystems.push_back(std::make_unique<JobManagerSubsystem>(task_scheduler_subsystem_ref));
		m_subsystems.push_back(std::make_unique<HookingSubsystem>());
		m_subsystems.push_back(std::make_unique<ScriptSubsystemAdapter>());
		m_subsystems.push_back(std::make_unique<ModManagerSubsystem>(event_manager_subsystem_ref));

		m_shutdown_complete = false;

		for (const auto& subsystem : m_subsystems)
		{
			RML_INFO("Initializing subsystem: {}", subsystem->name());

			if (auto result = subsystem->initialize(); !result)
			{
				RML_ERROR("Failed to initialize subsystem {}: {}", subsystem->name(), result.error().message);
				return std::unexpected(result.error());
			}

			RML_INFO("Subsystem initialized: {}", subsystem->name());
		}

		g_hooking->enable();
		RML_INFO("Hooking enabled.");

		if (qt::QtIntegration::instance()->ensure_action_hook())
		{
			RML_INFO("Qt action hook installed.");
		}
		else
		{
			RML_WARN("Qt action hook not installed yet (Qt not resolvable); will retry on demand.");
		}

		return {};
	}

	void Application::run()
	{
		g_running = true;

		while (g_running)
		{
			if (const auto qt_integration = qt::QtIntegration::instance(); qt_integration && !qt_integration->is_action_hook_ready())
			{
				qt_integration->ensure_action_hook();
			}

			std::this_thread::sleep_for(1s);
		}
	}

	void Application::shutdown()
	{
		for (auto& subsystem : m_subsystems | std::views::reverse)
		{
			RML_INFO("Shutting down subsystem: {}", subsystem->name());
			subsystem->shutdown();
		}

		m_subsystems.clear();
		m_shutdown_complete = true;
	}
}
