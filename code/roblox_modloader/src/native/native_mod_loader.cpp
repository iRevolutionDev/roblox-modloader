#include "native_mod_loader.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/memory/module.hpp"
#include "mod/mod_kind.hpp"

RML_LOG_SCOPE("NativeModLoader");

namespace rml::native
{
	std::filesystem::path mod_root_for(const std::filesystem::path& dll_path)
	{
		std::filesystem::path folder = dll_path.parent_path();
		if (mod_kind_from_folder(folder.filename().string()).has_value())
		{
			return folder.parent_path();
		}
		return folder;
	}

	NativeModLoader::~NativeModLoader()
	{
		unload_all();
	}

	std::expected<void, std::string> NativeModLoader::load(const std::filesystem::path& path)
	{
		if (m_registry.contains(path))
			return {};

		using start_fn_t = ModBase::start_type;
		using uninstall_fn_t = void (*)(const ModBase*);

		auto module_ptr = std::make_unique<memory::module>(path);
		if (auto r = module_ptr->attach(); !r)
		{
			return std::unexpected(std::format("Failed to attach module '{}' : {}", path.string(), r.error()));
		}

		auto start_h = module_ptr->get_export("start_mod");
		auto uninstall_h = module_ptr->get_export("uninstall_mod");
		auto abi_version_h = module_ptr->get_export("rml_abi_version");

		if (!start_h)
		{
			module_ptr->detach();
			return std::unexpected("Failed to find 'start_mod' export in native mod: " + path.string());
		}

		if (!uninstall_h)
		{
			module_ptr->detach();
			return std::unexpected("Failed to find 'uninstall_mod' export in native mod: " + path.string());
		}

		if (!abi_version_h)
		{
			module_ptr->detach();
			return std::unexpected(std::format(
			    "Native mod '{}' does not export 'rml_abi_version' (expected RML_ABI_VERSION={}); rebuild it against the current RobloxModLoader SDK",
			    path.string(), RML_ABI_VERSION));
		}

		if (const int mod_abi_version = abi_version_h.as<rml_abi_version_type>()(); mod_abi_version != RML_ABI_VERSION)
		{
			module_ptr->detach();
			return std::unexpected(std::format(
			    "Native mod '{}' was built against RML_ABI_VERSION={} but the loader is RML_ABI_VERSION={}; rebuild the mod",
			    path.string(), mod_abi_version, RML_ABI_VERSION));
		}

		auto* start = start_h.as<start_fn_t>();
		auto* uninstall = uninstall_h.as<uninstall_fn_t>();

		ModBase* instance = nullptr;
		try
		{
			instance = start();
		}
		catch (...)
		{
			module_ptr->detach();
			return std::unexpected("Exception while calling 'start_mod' for: " + path.string());
		}

		if (!instance)
		{
			module_ptr->detach();
			return std::unexpected("start_mod returned null for: " + path.string());
		}

		instance->set_paths(rml::mod::ModPaths(mod_root_for(path)));
		instance->set_event_manager(m_event_manager);

		try
		{
			instance->on_load();
		}
		catch (const std::exception& e)
		{
			uninstall(instance);
			module_ptr->detach();
			return std::unexpected(std::format("Exception while calling 'on_load' for {}: {}", path.string(), e.what()));
		}
		catch (...)
		{
			uninstall(instance);
			module_ptr->detach();
			return std::unexpected(std::format("Unknown exception while calling 'on_load' for {}", path.string()));
		}

		ModRegistry::Entry entry{};
		entry.module = std::move(module_ptr);
		entry.instance = instance;
		entry.uninstall = uninstall;

		m_registry.insert(path, std::move(entry));
		return {};
	}

	std::expected<void, std::string> NativeModLoader::unload(const std::filesystem::path& path)
	{
		auto extracted = m_registry.extract(path);
		if (!extracted.has_value())
			return {};

		auto& lm = *extracted;
		if (lm.instance)
		{
			try
			{
				lm.instance->on_unload();
			}
			catch (const std::exception& e)
			{
				RML_ERROR("Exception during on_unload for {}: {}", path.string(), e.what());
			}
			catch (...)
			{
				RML_ERROR("Unknown exception during on_unload for {}", path.string());
			}

			if (lm.uninstall)
			{
				try
				{
					lm.uninstall(lm.instance);
				}
				catch (...)
				{
					RML_ERROR("Unknown exception during uninstall_mod for {}", path.string());
				}
			}
			else
			{
				delete lm.instance;
				lm.instance = nullptr;
			}
		}

		if (lm.module)
		{
			if (auto r = lm.module->detach(); !r)
				return std::unexpected(std::format("Failed to unload native mod: {} : {}", path.string(), r.error()));
		}

		return {};
	}

	std::expected<void, std::string> NativeModLoader::reload(const std::filesystem::path& path)
	{
		if (auto r = unload(path); !r)
			return r;
		return load(path);
	}

	void NativeModLoader::unload_all()
	{
		for (auto& [module, instance, uninstall] : m_registry.extract_all())
		{
			if (instance)
			{
				try
				{
					instance->on_unload();
				}
				catch (const std::exception& e)
				{
					RML_ERROR("Exception during on_unload (unload_all): {}", e.what());
				}
				catch (...)
				{
					RML_ERROR("Unknown exception during on_unload (unload_all)");
				}
				if (uninstall)
				{
					try
					{
						uninstall(instance);
					}
					catch (...)
					{
						RML_ERROR("Unknown exception during uninstall_mod (unload_all)");
					}
				}
				else
				{
					delete instance;
				}
			}
			if (module)
			{
				module->detach();
			}
		}
	}

} // namespace rml::native
