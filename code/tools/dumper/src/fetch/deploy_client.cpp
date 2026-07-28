#include "rml/dumper/fetch/deploy_client.hpp"

#include <format>
#include <nlohmann/json.hpp>
#include <sstream>

namespace rml::dumper::fetch
{
	static constexpr std::string_view client_settings = "https://clientsettingscdn.roblox.com/v2/client-version";
	static constexpr std::string_view setup = "https://setup.rbxcdn.com";

	static constexpr Deployment windows_studio{
	    .binary_type = "WindowsStudio64",
	    .path_prefix = "",
	    .package = "RobloxStudio.zip",
	    .executable = "RobloxStudioBeta.exe",
	    .uses_package_manifest = true,
	};

	static constexpr Deployment macos_studio{
	    .binary_type = "MacStudio",
	    .path_prefix = "mac/",
	    .package = "RobloxStudioApp.zip",
	    .executable = "Contents/MacOS/RobloxStudio",
	    .uses_package_manifest = false,
	};

	static constexpr Deployment macos_arm64_studio{
	    .binary_type = "MacStudio",
	    .path_prefix = "mac/arm64/",
	    .package = "RobloxStudioApp.zip",
	    .executable = "Contents/MacOS/RobloxStudio",
	    .uses_package_manifest = false,
	};

	const Deployment* deployment_for(const std::string_view target)
	{
		if (target == "windows-x64")
			return &windows_studio;
		if (target == "macos-x64")
			return &macos_studio;
		if (target == "macos-arm64")
			return &macos_arm64_studio;

		return nullptr;
	}

	std::string DeployClient::version_url() const
	{
		if (m_channel.empty() || m_channel == "LIVE")
			return std::format("{}/{}", client_settings, m_deployment.binary_type);

		return std::format("{}/{}/channel/{}", client_settings, m_deployment.binary_type, m_channel);
	}

	std::string DeployClient::package_url(const StudioVersion& version, const std::string_view file) const
	{
		if (m_channel.empty() || m_channel == "LIVE")
			return std::format("{}/{}{}-{}", setup, m_deployment.path_prefix, version.guid, file);

		return std::format("{}/channel/{}/{}{}-{}", setup, m_channel, m_deployment.path_prefix, version.guid, file);
	}

	std::expected<StudioVersion, Error> DeployClient::parse_version_response(const std::string_view body)
	{
		const auto document = nlohmann::json::parse(body, nullptr, false);
		if (document.is_discarded())
			return std::unexpected(Error::make(ErrorCode::fetch, "the version response is not json"));

		const auto guid = document.find("clientVersionUpload");
		if (guid == document.end() || !guid->is_string())
			return std::unexpected(
			    Error::make(ErrorCode::fetch, "the version response carries no clientVersionUpload"));

		StudioVersion version;
		version.guid = guid->get<std::string>();

		if (const auto name = document.find("version"); name != document.end() && name->is_string())
			version.version = name->get<std::string>();
		else
			version.version = version.guid;

		return version;
	}

	std::expected<std::vector<PackageEntry>, Error> DeployClient::parse_manifest(const std::string_view body)
	{
		std::istringstream stream{std::string(body)};
		std::string line;

		if (!std::getline(stream, line))
			return std::unexpected(Error::make(ErrorCode::fetch, "the package manifest is empty"));

		const auto number = [](std::string text) -> std::uint64_t {
			if (!text.empty() && text.back() == '\r')
				text.pop_back();

			try
			{
				return std::stoull(text);
			}
			catch (const std::exception&)
			{
				return 0;
			}
		};

		std::vector<PackageEntry> packages;

		while (std::getline(stream, line))
		{
			if (!line.empty() && line.back() == '\r')
				line.pop_back();
			if (line.empty())
				continue;

			std::string checksum;
			std::string compressed;
			std::string uncompressed;
			if (!std::getline(stream, checksum) || !std::getline(stream, compressed) ||
			    !std::getline(stream, uncompressed))
				break;

			packages.push_back({line, number(compressed), number(uncompressed)});
		}

		if (packages.empty())
			return std::unexpected(Error::make(ErrorCode::fetch, "the package manifest lists nothing"));

		return packages;
	}

	std::expected<StudioVersion, Error> DeployClient::latest_version() const
	{
		const auto body = m_client->get(version_url());
		if (!body)
			return std::unexpected(body.error());

		return parse_version_response(std::string_view(reinterpret_cast<const char*>(body->data()), body->size()));
	}

	std::expected<std::vector<PackageEntry>, Error> DeployClient::package_manifest(
	    const StudioVersion& version) const
	{
		const auto body = m_client->get(package_url(version, "rbxPkgManifest.txt"));
		if (!body)
			return std::unexpected(body.error());

		return parse_manifest(std::string_view(reinterpret_cast<const char*>(body->data()), body->size()));
	}
}
