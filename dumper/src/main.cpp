#include "dumper/common.hpp"
#include "dumper/luau_dumper.hpp"
#include "dumper/offset_generator.hpp"
#include "dumper/pointers.hpp"

void dumper_main()
{
	try
	{
		AllocConsole();

		FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);
		freopen_s(&f, "CONOUT$", "w", stderr);
		SetConsoleTitleA("Roblox ModLoader Dumper");

		const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		const auto file_sink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>("dumper.log", true);

		std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

		const auto logger = std::make_shared<spdlog::logger>("dumper", sinks.begin(), sinks.end());
		logger->set_level(spdlog::level::debug);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(logger);

		spdlog::info("=== Roblox ModLoader Dumper ===");

		spdlog::info("Scanning for Luau structures...");
		auto pointers_instance = std::make_unique<dumper::pointers>();

		dumper::LuauDumper dumper(pointers_instance);
		if (!dumper.analyze())
		{
			spdlog::error("Failed to analyze Luau structures");
			return;
		}

		char module_path[MAX_PATH];
		GetModuleFileNameA(nullptr, module_path, MAX_PATH);
		const auto exe_dir = std::filesystem::path(module_path).parent_path();

		const auto output_dir = exe_dir / "dumper_output";
		std::filesystem::create_directories(output_dir);

		spdlog::info("Output directory: {}", output_dir.string());

		dumper::OffsetGenerator generator;

		spdlog::info("Generating offsets in C++ header format...");
		if (generator.generate(dumper.get_structures(), output_dir / "luau_offsets.hpp", dumper::OutputFormat::CPP_Header))
		{
			spdlog::info("C++ header offsets generated successfully");
		}

		spdlog::info("Generating structs in C++ format...");
		if (generator.generate(dumper.get_structures(), output_dir / "luau_structs.hpp", dumper::OutputFormat::CPP_Struct))
		{
			spdlog::info("C++ structs generated successfully");
		}

		spdlog::info("Generating offsets in Genny format...");
		if (generator.generate(dumper.get_structures(), output_dir / "luau_offsets.genny", dumper::OutputFormat::Genny))
		{
			spdlog::info("Genny offsets generated successfully");
		}

		spdlog::info("=== Dump completed successfully! ===");
		spdlog::info("Files saved in: {}", output_dir.string());

		spdlog::info("Press Enter to close...");
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cin.get();

		FreeConsole();
	}
	catch (const std::exception& e)
	{
		spdlog::error("Exception occurred: {}", e.what());
		spdlog::info("Press Enter to close...");
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cin.get();
		FreeConsole();
	}
	catch (...)
	{
		spdlog::error("Unknown exception occurred");
		spdlog::info("Press Enter to close...");
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cin.get();
		FreeConsole();
	}
}

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD ul_reason_for_call, LPVOID lp_reserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);

		CreateThread(
		    nullptr,
		    0,
		    [](LPVOID) -> DWORD {
			    dumper_main();
			    return 0;
		    },
		    nullptr,
		    0,
		    nullptr);
		break;

	case DLL_PROCESS_DETACH: spdlog::info("Dumper DLL unloaded"); break;
	case DLL_THREAD_DETACH: break;
	default: break;
	}
	return TRUE;
}
