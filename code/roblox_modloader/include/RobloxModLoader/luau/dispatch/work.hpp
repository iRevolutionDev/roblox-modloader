#pragma once

#include "RobloxModLoader/luau/dispatch/value.hpp"
#include "RobloxModLoader/luau/script/mod_manifest.hpp"
#include "RobloxModLoader/luau/vm/vm_error.hpp"

namespace rml::luau
{
	using WorkResult = std::expected<Value, vm::VmError>;

	struct RunChunk
	{
		std::string chunk_name;
		std::vector<std::byte> bytecode;
		ModManifestPtr owner;
		bool want_result{false};
	};

	struct CallRef
	{
		RefId target{kInvalidRef};
		std::vector<Value> args;
	};

	struct IndexRef
	{
		RefId target{kInvalidRef};
		std::string key;
	};

	struct ReleaseRef
	{
		RefId target{kInvalidRef};
	};

	using Work = std::variant<RunChunk, CallRef, IndexRef, ReleaseRef>;
}
