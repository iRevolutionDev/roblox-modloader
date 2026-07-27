#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/internal/memory/rtti_scanner.hpp"
#include "RobloxModLoader/memory/i_rtti_provider.hpp"

namespace rml::memory
{
	class MsvcRttiProvider final : public IRttiProvider
	{
	public:
		std::optional<void**> find_class_vtable(const std::string_view class_name) override
		{
			const auto* info = rtti::RTTIManager::get_class_rtti(class_name);
			if (!info)
				return std::nullopt;

			auto* vtable = info->get_virtual_function_table();
			if (!vtable)
				return std::nullopt;

			return vtable;
		}

	private:
		rtti::RTTIManager m_manager;
	};

	std::unique_ptr<IRttiProvider> create_rtti_provider()
	{
		return std::make_unique<MsvcRttiProvider>();
	}
}
