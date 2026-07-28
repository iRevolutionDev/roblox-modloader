#include "cli/runner.hpp"

#include "rml/dumper/emit/emitter.hpp"
#include "rml/dumper/recover/recovery_pipeline.hpp"
#include "rml/dumper/target/target_profile.hpp"
#include "target/pattern_anchor_resolver.hpp"

#include <fstream>
#include <sstream>

#include <spdlog/spdlog.h>

namespace rml::dumper::cli
{
	std::expected<std::filesystem::path, Error> Runner::resolve_studio(const Options& options)
	{
		if (!options.input)
			return std::unexpected(Error::make(
			    ErrorCode::fetch, "fetching a studio build is not wired up yet, pass --input for now"));

		std::error_code ec;
		if (!std::filesystem::exists(*options.input, ec))
			return std::unexpected(Error::make(ErrorCode::fetch, "no such file: {}", options.input->string()));

		return *options.input;
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

		spdlog::info("target {}, studio {}", profile->name, studio->string());

		const auto image = image::ImageLoader::load(*studio, profile->architecture);
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

		if (const auto written = write_outputs(options, *layouts); !written)
			return std::unexpected(written.error());

		spdlog::info("{}", layouts->report.summary());

		if (layouts->report.has_failures())
			return std::unexpected(Error::make(ErrorCode::recovery, "{} fields could not be recovered",
			                                   layouts->report.failures().size()));

		return {};
	}
}
