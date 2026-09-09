#pragma once

#include "RobloxModLoader/internal/common.hpp"
#include <unordered_map>

namespace rml::luau
{
	enum class NodeClass : std::uint8_t
	{
		Folder,
		Script,
		ModuleScript,
	};

	[[nodiscard]] std::string_view class_name_of(NodeClass value) noexcept;

	[[nodiscard]] bool class_is_a(NodeClass value, std::string_view queried) noexcept;

	struct ScriptNode
	{
		std::string name;
		std::string logical;
		NodeClass klass{NodeClass::Folder};
		std::filesystem::path source;
		const ScriptNode* parent{nullptr};
		std::vector<const ScriptNode*> children;

		[[nodiscard]] std::string_view class_name() const noexcept { return class_name_of(klass); }

		[[nodiscard]] bool requireable() const noexcept { return klass == NodeClass::ModuleScript; }

		[[nodiscard]] const ScriptNode* child(std::string_view child_name) const noexcept;

		[[nodiscard]] const ScriptNode* descendant(std::string_view child_name) const noexcept;

		[[nodiscard]] const ScriptNode* ancestor(std::string_view ancestor_name) const noexcept;

		[[nodiscard]] bool descends_from(const ScriptNode* other) const noexcept;

		[[nodiscard]] std::string full_name() const;
	};

	class ScriptTree final
	{
	public:
		ScriptTree() = default;

		ScriptTree(const ScriptTree&) = delete;
		ScriptTree& operator=(const ScriptTree&) = delete;
		ScriptTree(ScriptTree&&) = delete;
		ScriptTree& operator=(ScriptTree&&) = delete;

		[[nodiscard]] const ScriptNode* root() const noexcept { return m_root; }

		[[nodiscard]] const ScriptNode* find(std::string_view logical) const noexcept;

		[[nodiscard]] bool owns(const ScriptNode* node) const noexcept;

		[[nodiscard]] std::size_t size() const noexcept { return m_nodes.size(); }

		ScriptNode& adopt(std::unique_ptr<ScriptNode> node);

		void seal(const ScriptNode* root) noexcept { m_root = root; }

	private:
		std::vector<std::unique_ptr<ScriptNode>> m_nodes;
		std::unordered_map<std::string, const ScriptNode*> m_by_logical;
		const ScriptNode* m_root{nullptr};
	};

	using ScriptTreePtr = std::shared_ptr<const ScriptTree>;

	[[nodiscard]] ScriptTreePtr build_script_tree(const std::filesystem::path& scripts_root, std::string root_name,
	                                              std::string_view alias,
	                                              std::span<const std::filesystem::path> entry_scripts);
}
