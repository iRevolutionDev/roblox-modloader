#include "rml/dumper/target/target_profile.hpp"

#include "target/targets/profiles.hpp"

#include <algorithm>
#include <vector>

namespace rml::dumper::target
{
	static const std::vector<TargetProfile>& profiles()
	{
		static const std::vector<TargetProfile> registered{windows_x64_profile(), macos_x64_profile(),
		                                                   macos_arm64_profile()};
		return registered;
	}

	const TargetProfile* TargetRegistry::find(const std::string_view name)
	{
		const auto found = std::ranges::find(profiles(), name, &TargetProfile::name);
		return found != profiles().end() ? &*found : nullptr;
	}

	std::span<const TargetProfile> TargetRegistry::all()
	{
		return profiles();
	}

	std::string TargetRegistry::names()
	{
		std::string text;

		for (const auto& profile : profiles())
		{
			if (!text.empty())
				text += ", ";
			text += profile.name;
		}

		return text;
	}
}
