#pragma once

#include "rml/dumper/fetch/http_client.hpp"

#include <optional>

namespace rml::dumper::fetch
{
	struct Deployment
	{
		std::string_view binary_type;
		std::string_view path_prefix;
		std::string_view package;
		std::string_view executable;
		bool uses_package_manifest{};
	};

	struct StudioVersion
	{
		std::string version;
		std::string guid;
	};

	struct PackageEntry
	{
		std::string name;
		std::uint64_t compressed_size{};
		std::uint64_t uncompressed_size{};
	};

	[[nodiscard]] const Deployment* deployment_for(std::string_view target);

	class DeployClient
	{
	public:
		DeployClient(const HttpClient& client, std::string channel, Deployment deployment) :
		    m_client(&client),
		    m_channel(std::move(channel)),
		    m_deployment(deployment)
		{
		}

		[[nodiscard]] std::string version_url() const;
		[[nodiscard]] std::string package_url(const StudioVersion& version, std::string_view file) const;

		[[nodiscard]] std::expected<StudioVersion, Error> latest_version() const;
		[[nodiscard]] std::expected<std::vector<PackageEntry>, Error> package_manifest(
		    const StudioVersion& version) const;

		[[nodiscard]] static std::expected<StudioVersion, Error> parse_version_response(std::string_view body);
		[[nodiscard]] static std::expected<std::vector<PackageEntry>, Error> parse_manifest(std::string_view body);

	private:
		const HttpClient* m_client;
		std::string m_channel;
		Deployment m_deployment;
	};
}
