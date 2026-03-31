#include "native_mod_loader.hpp"

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/memory/module.hpp"

namespace rml::native
{
	NativeModLoader::~NativeModLoader()
	{
		unload_all();
	}

	std::expected<void, std::string> NativeModLoader::load(const std::filesystem::path& path)
	{
		if (m_loaded_mods.contains(path))
			return {};

		using start_fn_t     = mod_base::start_type;
		using uninstall_fn_t = void (*)(const mod_base*);

		auto module_ptr = std::make_unique<memory::module>(path);
		if (auto r = module_ptr->attach(); !r)
		{
			return std::unexpected(std::format("Failed to attach module '{}' : {}", path.string(), r.error()));
		}

		auto start_h     = module_ptr->get_export("start_mod");
		auto uninstall_h = module_ptr->get_export("uninstall_mod");

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

		auto* start     = start_h.as<start_fn_t>();
		auto* uninstall = uninstall_h.as<uninstall_fn_t>();

		mod_base* instance = nullptr;
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

		try
		{
			instance->on_load();
		}
		catch (std::exception& e)
		{
			uninstall(instance);
			module_ptr->detach();
			return std::unexpected(std::format("Exception while calling 'on_load' for {}: {}", path.string(), e.what()));
		}
		catch (...)
		{
		}

		LoadedMod lm{};
		lm.module    = std::move(module_ptr);
		lm.instance  = instance;
		lm.uninstall = uninstall;

		m_loaded_mods[path] = std::move(lm);
		return {};
	}

	std::expected<void, std::string> NativeModLoader::unload(const std::filesystem::path& path)
	{
		const auto it = m_loaded_mods.find(path);
		if (it == m_loaded_mods.end())
			return {};

		auto& lm = it->second;
		if (lm.instance)
		{
			try
			{
				lm.instance->on_unload();
			}
			catch (...)
			{
			}

			if (lm.uninstall)
			{
				try
				{
					lm.uninstall(lm.instance);
				}
				catch (...)
				{
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

		m_loaded_mods.erase(it);
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
		for (auto& [module, instance, uninstall] : m_loaded_mods | std::views::values)
		{
			if (instance)
			{
				try
				{
					instance->on_unload();
				}
				catch (...)
				{
				}
				if (uninstall)
				{
					try
					{
						uninstall(instance);
					}
					catch (...)
					{
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
		m_loaded_mods.clear();
	}

} // namespace rml::native
