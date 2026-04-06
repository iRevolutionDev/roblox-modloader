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
			const Descriptor* preferred; // used if isDeprecated
			Attributes() :
			    is_deprecated(false),
			    preferred(nullptr)
			{
			}

			static Attributes deprecated(const Descriptor& preferred)
			{
				Attributes result;
				result.is_deprecated = true;
				result.preferred     = &preferred;
				return result;
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
		char _descriptor_pad[16];

		virtual ~Descriptor()
		{
		}
	};

	static_assert(sizeof(Descriptor) == 0x40, "Descriptor binary layout mismatch: expected 64 bytes.");
}