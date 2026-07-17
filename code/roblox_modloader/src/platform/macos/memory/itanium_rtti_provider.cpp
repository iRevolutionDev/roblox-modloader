#include "RobloxModLoader/memory/i_rtti_provider.hpp"

namespace rml::memory
{
	class ItaniumRttiProvider final : public IRttiProvider
	{
	public:
		std::optional<void**> find_class_vtable(std::string_view) override
		{
			return std::nullopt;
		}
	};

	std::unique_ptr<IRttiProvider> create_rtti_provider()
	{
		return std::make_unique<ItaniumRttiProvider>();
	}
}
