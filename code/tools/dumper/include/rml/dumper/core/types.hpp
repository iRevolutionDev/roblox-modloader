#pragma once

#include <cstdint>
#include <string_view>

namespace rml::dumper
{
	using Rva = std::uint32_t;
	using Va = std::uint64_t;

	enum class Architecture : std::uint8_t
	{
		x86_64,
		arm64,
	};

	enum class ImageFormat : std::uint8_t
	{
		pe,
		mach_o,
	};

	[[nodiscard]] constexpr std::string_view to_string(const Architecture value)
	{
		switch (value)
		{
		case Architecture::x86_64: return "x86_64";
		case Architecture::arm64: return "arm64";
		}
		return "unknown";
	}

	[[nodiscard]] constexpr std::string_view to_string(const ImageFormat value)
	{
		switch (value)
		{
		case ImageFormat::pe: return "pe";
		case ImageFormat::mach_o: return "mach-o";
		}
		return "unknown";
	}
}
