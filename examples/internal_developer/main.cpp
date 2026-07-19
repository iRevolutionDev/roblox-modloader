#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"
#include "internal_pointers.hpp"

RML_LOG_SCOPE("InternalDeveloper")

namespace internal_developer
{
	static bool is_internal()
	{
		const auto& pointers = engine_pointers();

		*pointers.channel_flag = true;
		*pointers.internal_flag = true;

		return rml::Hooking::get_original<&is_internal>()();
	}
}

class InternalDeveloperMod final : public ModBase
{
public:
	InternalDeveloperMod()
	{
		name = "Internal Developer Mod";
		version = "2.0.0";
		author = "Revolution";
		description = "Enables Roblox Studio internal developer features.";
	}

	void on_load() override
	{
		if (!internal_developer::resolve_engine_pointers())
			return;

		rml::Hooking::DetourHookHelper::add<&internal_developer::is_internal>("IsInternal",
		    internal_developer::engine_pointers().is_internal);

		RML_INFO("Internal developer features enabled");
	}

	void on_unload() override
	{
		RML_INFO("Internal developer features disabled");
	}
};

extern "C"
{
	RML_MOD_ABI_EXPORT ModBase* start_mod()
	{
		return new InternalDeveloperMod();
	}

	RML_MOD_ABI_EXPORT void uninstall_mod(const ModBase* mod)
	{
		delete mod;
	}
}

RML_EXPORT_MOD_ABI_VERSION()
