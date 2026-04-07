#pragma once

#include "../util/name.hpp"
#include "RobloxModLoader/roblox/memory/noncopyable.hpp"

#include <memory>

namespace RBX::Reflection
{
	class Descriptor : public rml::memory::Noncopyable
	{
	public:
		struct Attributes
		{
			bool is_deprecated;
			Attributes() :
			    is_deprecated(false)
			{
			}

			static Attributes deprecated()
			{
				Attributes result;
				result.is_deprecated = true;
				return result;
			}
		};

		static bool locked_down;

		const Name& name;
		std::unique_ptr<bool> is_replicable;
		std::unique_ptr<bool> is_outdated;

		const Attributes attributes;

		virtual ~Descriptor()
		{
		}
	};

	static_assert(sizeof(Descriptor) == 0x28, "Descriptor binary layout mismatch: expected 40 bytes.");
}