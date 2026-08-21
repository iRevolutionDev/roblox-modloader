#include "rml/dumper/fetch/studio_provider.hpp"

#include "rml/dumper/fetch/zip_reader.hpp"

#include <algorithm>
#include <fstream>
#include <spdlog/spdlog.h>

namespace rml::dumper::fetch
{
	std::expected<ResolvedStudio, Error> LocalStudioProvider::resolve()
	{
		std::error_code ec;
		if (!std::filesystem::exists(m_path, ec))
			return std::unexpected(Error::make(ErrorCode::fetch, "no such file: {}", m_path.string()));

		return ResolvedStudio{.executable = m_path, .version = {.version = "local", .guid = "local"}};
	}

	std::expected<ResolvedStudio, Error> RemoteStudioProvider::resolve()
	{
		const auto version = m_deploy.latest_version();
		if (!version)
			return std::unexpected(version.error());

		spdlog::info("latest {} is {} ({})", m_deployment.binary_type, version->version, version->guid);

		const auto destination = m_cache / version->guid / std::filesystem::path(m_deployment.executable).filename();

		std::error_code ec;
		if (std::filesystem::exists(destination, ec))
		{
			spdlog::info("using the cached copy at {}", destination.string());
			return ResolvedStudio{.executable = destination, .version = *version};
		}

		if (m_deployment.uses_package_manifest)
		{
			const auto manifest = m_deploy.package_manifest(*version);
			if (!manifest)
				return std::unexpected(manifest.error());

			const auto listed = std::ranges::any_of(*manifest, [this](const PackageEntry& entry) {
				return entry.name == m_deployment.package;
			});

			if (!listed)
				return std::unexpected(Error::make(ErrorCode::fetch, "{} is not in the package manifest",
				                                   m_deployment.package));
		}

		const auto url = m_deploy.package_url(*version, m_deployment.package);
		spdlog::info("reading {}", url);

		auto source = HttpRangeSource::open(*m_client, url);
		if (!source)
			return std::unexpected(source.error());

		const auto archive = ZipReader::open(*source);
		if (!archive)
			return std::unexpected(archive.error());

		const auto* entry = archive->find(m_deployment.executable);
		if (entry == nullptr)
			return std::unexpected(
			    Error::make(ErrorCode::fetch, "{} is not inside {}", m_deployment.executable, url));

		spdlog::info("extracting {} ({} bytes) after reading {} of {}", entry->name, entry->uncompressed_size,
		             source->bytes_read(), source->size());

		const auto bytes = archive->extract(*entry);
		if (!bytes)
			return std::unexpected(bytes.error());

		std::filesystem::create_directories(destination.parent_path(), ec);

		std::ofstream file(destination, std::ios::binary | std::ios::trunc);
		if (!file)
			return std::unexpected(Error::make(ErrorCode::fetch, "cannot write {}", destination.string()));

		file.write(reinterpret_cast<const char*>(bytes->data()), static_cast<std::streamsize>(bytes->size()));

		spdlog::info("read {} of {} bytes in total", source->bytes_read(), source->size());

		return ResolvedStudio{.executable = destination, .version = *version};
	}
}
