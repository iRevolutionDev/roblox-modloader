#include "dumper/common.hpp"
#include "dumper/luau_dumper.hpp"
#include "dumper/memory/module.hpp"
#include "dumper/offset_generator.hpp"
#include "dumper/pointers.hpp"

#include <filesystem>

namespace
{
	uintptr_t map_image(const std::string& path)
	{
		const HMODULE h = LoadLibraryExA(path.c_str(), nullptr, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
		if (!h)
			return 0;
		return reinterpret_cast<uintptr_t>(h) & ~static_cast<uintptr_t>(0xF);
	}

	void setup_logging()
	{
		const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		const auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("dumper.log", true);
		std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
		const auto logger = std::make_shared<spdlog::logger>("dumper", sinks.begin(), sinks.end());
		logger->set_level(spdlog::level::debug);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(logger);
	}
}

int main(const int argc, char** argv)
{
	try
	{
		setup_logging();

		const std::string studio_path = argc > 1 ? argv[1] : "RobloxStudioBeta.exe";
		spdlog::info("=== Roblox ModLoader Luau Analyser ===");
		spdlog::info("Target: {}", studio_path);

		if (!std::filesystem::exists(studio_path))
		{
			spdlog::error("Target file does not exist: {}", studio_path);
			return 2;
		}

		const auto image_base = map_image(studio_path);
		if (!image_base)
		{
			spdlog::error("Failed to map target image (LoadLibraryEx error {})", GetLastError());
			return 3;
		}

		const auto region = memory::module(memory::handle(reinterpret_cast<void*>(image_base)));
		spdlog::info("Mapped at 0x{:X}, size 0x{:X}", image_base, region.size());

		spdlog::info("Scanning for Luau anchor functions...");
		auto pointers_instance = std::make_unique<dumper::pointers>(region);

		dumper::LuauDumper dumper(pointers_instance, image_base);
		if (!dumper.analyze())
		{
			spdlog::error("Failed to analyze Luau structures");
			return 4;
		}

		const auto output_dir = std::filesystem::current_path() / "dumper_output";
		std::filesystem::create_directories(output_dir);
		spdlog::info("Output directory: {}", output_dir.string());

		dumper::OffsetGenerator generator;
		generator.generate(dumper.get_structures(), output_dir / "luau_offsets.hpp", dumper::OutputFormat::CPP_Header);
		generator.generate(dumper.get_structures(), output_dir / "luau_structs.hpp", dumper::OutputFormat::CPP_Struct);
		generator.generate(dumper.get_structures(), output_dir / "luau_layout.json", dumper::OutputFormat::JSON);
		generator.generate(dumper.get_structures(), output_dir / "luau_shuffle_layout.h", dumper::OutputFormat::LuauShuffle);
		return 0;
	}
	catch (const std::exception& e)
	{
		spdlog::error("Exception: {}", e.what());
		return 1;
	}
	catch (...)
	{
		spdlog::error("Unknown exception");
		return 1;
	}
}
