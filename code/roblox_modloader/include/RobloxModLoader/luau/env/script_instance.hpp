#pragma once

#include "RobloxModLoader/luau/script/script_tree.hpp"
#include "RobloxModLoader/luau/vm/lua_ref.hpp"
#include "RobloxModLoader/luau/vm/vm_error.hpp"

namespace rml::luau
{
	class ScriptEnv;

	class NodeCache final
	{
	public:
		NodeCache() = default;

		NodeCache(const NodeCache&) = delete;
		NodeCache& operator=(const NodeCache&) = delete;
		NodeCache(NodeCache&&) = delete;
		NodeCache& operator=(NodeCache&&) = delete;

		bool push(ScriptEnv& env, lua_State* L, const ScriptNode* node);

		void release() noexcept;

	private:
		bool ensure_shared(ScriptEnv& env, lua_State* L);

		void push_metatable(const ScriptEnv& env, lua_State* L, const ScriptNode* node) const;

		std::unordered_map<const ScriptNode*, vm::Ref> m_nodes;
		vm::Ref m_methods;
		vm::Ref m_index;
		vm::Ref m_newindex;
		vm::Ref m_tostring;
	};

	[[nodiscard]] const ScriptNode* to_script_node(lua_State* L, int index, const ScriptEnv* env = nullptr) noexcept;

	[[nodiscard]] std::expected<void, vm::VmError> load_chunk_for(ScriptEnv& env, lua_State* L,
	                                                              std::string_view chunk_name,
	                                                              std::span<const std::byte> bytecode,
	                                                              std::uint64_t capabilities, const ScriptNode* node);
}
