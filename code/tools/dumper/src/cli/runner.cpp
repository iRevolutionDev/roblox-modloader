#include "cli/runner.hpp"

#include "rml/dumper/emit/emitter.hpp"
#include "rml/dumper/recover/recovery_pipeline.hpp"
#include "rml/dumper/target/target_profile.hpp"
#include "target/pattern_anchor_resolver.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

#include <spdlog/spdlog.h>

namespace rml::dumper::cli
{
	static std::optional<std::filesystem::path> studio_beside_the_loader(const std::string_view target)
	{
		const char* root = std::getenv("ROBLOX_STUDIO_PATH");
		if (root == nullptr || *root == '\0')
			return std::nullopt;

		const std::filesystem::path base(root);
		std::vector<std::filesystem::path> candidates;

		if (target.starts_with("macos"))
		{
			candidates.push_back(base / "Contents" / "MacOS" / "RobloxStudio");
			candidates.push_back(base / "RobloxStudio");
		}
		else
		{
			candidates.push_back(base / "RobloxStudioBeta.exe");
		}

		candidates.push_back(base);

		for (const auto& candidate : candidates)
			if (std::error_code ec; std::filesystem::is_regular_file(candidate, ec))
				return candidate;

		return std::nullopt;
	}

	std::expected<fetch::ResolvedStudio, Error> Runner::resolve_studio(const Options& options)
	{
		if (options.input)
			return fetch::LocalStudioProvider(*options.input).resolve();

		if (const auto deployed = studio_beside_the_loader(options.target))
		{
			spdlog::info("studio taken from ROBLOX_STUDIO_PATH, {}", deployed->string());
			return fetch::LocalStudioProvider(*deployed).resolve();
		}

		const auto* deployment = fetch::deployment_for(options.target);
		if (deployment == nullptr)
			return std::unexpected(
			    Error::make(ErrorCode::fetch, "no deployment is known for {}", options.target));

		auto client = fetch::HttpClient::create();
		if (!client)
			return std::unexpected(Error::make(ErrorCode::fetch, "cannot create an http client"));

		const auto cache = options.cache.value_or(std::filesystem::temp_directory_path() / "rml_dumper_cache");

		fetch::DeployClient deploy(*client, options.channel, *deployment);
		fetch::RemoteStudioProvider provider(*client, std::move(deploy), *deployment, cache);

		return provider.resolve();
	}

	std::expected<void, Error> Runner::write_outputs(const Options& options, const schema::LayoutSet& layouts)
	{
		std::error_code ec;
		std::filesystem::create_directories(options.out, ec);

		const emit::EmitterRegistry registry;

		std::vector<std::unique_ptr<emit::Emitter>> emitters;
		if (options.emitters.empty())
			emitters = registry.create_all();
		else
			for (const auto& id : options.emitters)
				emitters.push_back(registry.create(id));

		for (const auto& emitter : emitters)
		{
			const auto path = options.out / emitter->default_filename();

			std::ostringstream buffer;
			if (const auto emitted = emitter->emit(layouts, buffer); !emitted)
			{
				spdlog::warn("skipping {}: {}", emitter->default_filename(), emitted.error().message());
				continue;
			}

			std::ofstream file(path, std::ios::binary | std::ios::trunc);
			if (!file)
				return std::unexpected(Error::make(ErrorCode::usage, "cannot write {}", path.string()));

			file << buffer.str();
			spdlog::info("wrote {}", path.string());
		}

		return {};
	}

	std::expected<void, Error> Runner::run(const Options& options) const
	{
		const auto* profile = target::TargetRegistry::find(options.target);
		if (profile == nullptr)
			return std::unexpected(Error::make(ErrorCode::usage, "unknown target {}", options.target));

		if (profile->anchors.empty())
			return std::unexpected(Error::make(ErrorCode::anchor, "profile {}: 0 of {} anchors defined",
			                                   profile->name, target::anchor_count));

		const auto studio = resolve_studio(options);
		if (!studio)
			return std::unexpected(studio.error());

		spdlog::info("target {}, studio {}", profile->name, studio->executable.string());

		const auto image = image::ImageLoader::load(studio->executable, profile->architecture);
		if (!image)
			return std::unexpected(image.error());

		spdlog::info("{} image, base 0x{:X}, {} sections, {} functions", to_string(image->format()),
		             image->preferred_base(), image->sections().size(), image->functions().size());

		const auto anchors = target::PatternAnchorResolver().resolve(*image, profile->anchors);
		if (!anchors)
			return std::unexpected(anchors.error());

		spdlog::info("{} anchors resolved", profile->anchors.size());

		const auto decoder = disasm::Decoder::create(profile->architecture);
		if (!decoder)
			return std::unexpected(decoder.error());

		schema::Report report;
		recover::RecoveryContext context(*image, **decoder, *profile->abi, *anchors, report);

		auto layouts = recover::RecoveryPipeline::make_default().run(context);
		if (!layouts)
			return std::unexpected(layouts.error());

		layouts->target = profile->name;
		layouts->studio_version = studio->version.version;
		layouts->studio_guid = studio->version.guid;

		if (const auto written = write_outputs(options, *layouts); !written)
			return std::unexpected(written.error());

		spdlog::info("{}", layouts->report.summary());

		if (layouts->report.has_failures())
			return std::unexpected(Error::make(ErrorCode::recovery, "{} fields could not be recovered",
			                                   layouts->report.failures().size()));

		return {};
	}
}
