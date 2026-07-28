#include <doctest/doctest.h>

#include "rml/dumper/fetch/deploy_client.hpp"
#include "rml/dumper/fetch/studio_provider.hpp"
#include "support/temporary_file.hpp"

using namespace rml::dumper;
using namespace rml::dumper::fetch;

class NullHttpClient final : public HttpClient
{
public:
	[[nodiscard]] std::expected<std::vector<std::byte>, Error> get(std::string_view) const override
	{
		return std::unexpected(Error::make(ErrorCode::fetch, "no network in this test"));
	}

	[[nodiscard]] std::expected<std::vector<std::byte>, Error> get_range(std::string_view, std::uint64_t,
	                                                                     std::uint64_t) const override
	{
		return std::unexpected(Error::make(ErrorCode::fetch, "no network in this test"));
	}

	[[nodiscard]] std::expected<std::uint64_t, Error> content_length(std::string_view) const override
	{
		return std::unexpected(Error::make(ErrorCode::fetch, "no network in this test"));
	}
};

TEST_CASE("a version response is parsed as json")
{
	const auto version = DeployClient::parse_version_response(
	    R"({"version":"0.731.0.7310942","clientVersionUpload":"version-14d8b191232f4ddd","bootstrapperVersion":""})");

	REQUIRE(version.has_value());
	CHECK(version->version == "0.731.0.7310942");
	CHECK(version->guid == "version-14d8b191232f4ddd");
}

TEST_CASE("a malformed version response is refused")
{
	CHECK_FALSE(DeployClient::parse_version_response("not json").has_value());
	CHECK_FALSE(DeployClient::parse_version_response(R"({"version":"1.0"})").has_value());
	CHECK(DeployClient::parse_version_response(R"({"clientVersionUpload":"version-abc"})")->version ==
	      "version-abc");
}

TEST_CASE("a package manifest is parsed into entries")
{
	const auto manifest = DeployClient::parse_manifest(
	    "v0\r\nRobloxStudio.zip\r\nabc123\r\n1000\r\n2000\r\nLibraries.zip\r\ndef456\r\n30\r\n40\r\n");

	REQUIRE(manifest.has_value());
	REQUIRE(manifest->size() == 2);
	CHECK((*manifest)[0].name == "RobloxStudio.zip");
	CHECK((*manifest)[0].compressed_size == 1000);
	CHECK((*manifest)[0].uncompressed_size == 2000);
	CHECK((*manifest)[1].name == "Libraries.zip");
	CHECK((*manifest)[1].uncompressed_size == 40);
}

TEST_CASE("an empty manifest is refused")
{
	CHECK_FALSE(DeployClient::parse_manifest("").has_value());
	CHECK_FALSE(DeployClient::parse_manifest("v0\r\n").has_value());
}

TEST_CASE("every target knows where its deployment lives")
{
	REQUIRE(deployment_for("windows-x64") != nullptr);
	REQUIRE(deployment_for("macos-x64") != nullptr);
	REQUIRE(deployment_for("macos-arm64") != nullptr);
	CHECK(deployment_for("solaris-sparc") == nullptr);

	CHECK(deployment_for("windows-x64")->binary_type == "WindowsStudio64");
	CHECK(deployment_for("windows-x64")->uses_package_manifest);
	CHECK(deployment_for("macos-x64")->binary_type == "MacStudio");
	CHECK_FALSE(deployment_for("macos-arm64")->uses_package_manifest);
	CHECK(deployment_for("macos-arm64")->path_prefix == "mac/arm64/");
}

TEST_CASE("live urls follow the plain layout")
{
	const NullHttpClient client;
	const DeployClient deploy(client, "LIVE", *deployment_for("windows-x64"));
	const StudioVersion version{.version = "0.731.0", .guid = "version-abc"};

	CHECK(deploy.version_url() == "https://clientsettingscdn.roblox.com/v2/client-version/WindowsStudio64");
	CHECK(deploy.package_url(version, "RobloxStudio.zip") ==
	      "https://setup.rbxcdn.com/version-abc-RobloxStudio.zip");
}

TEST_CASE("the mac arm64 deployment sits under its own prefix")
{
	const NullHttpClient client;
	const DeployClient deploy(client, "LIVE", *deployment_for("macos-arm64"));
	const StudioVersion version{.version = "0.731.0", .guid = "version-abc"};

	CHECK(deploy.version_url() == "https://clientsettingscdn.roblox.com/v2/client-version/MacStudio");
	CHECK(deploy.package_url(version, "RobloxStudioApp.zip") ==
	      "https://setup.rbxcdn.com/mac/arm64/version-abc-RobloxStudioApp.zip");
}

TEST_CASE("a channel is prefixed onto both urls")
{
	const NullHttpClient client;
	const DeployClient deploy(client, "ZIntegration", *deployment_for("macos-x64"));
	const StudioVersion version{.version = "0.731.0", .guid = "version-abc"};

	CHECK(deploy.version_url() ==
	      "https://clientsettingscdn.roblox.com/v2/client-version/MacStudio/channel/ZIntegration");
	CHECK(deploy.package_url(version, "RobloxStudioApp.zip") ==
	      "https://setup.rbxcdn.com/channel/ZIntegration/mac/version-abc-RobloxStudioApp.zip");
}

TEST_CASE("the local provider hands back the path it was given")
{
	const auto path = tests::TemporaryFile::write(std::vector<std::byte>{std::byte{0x4D}, std::byte{0x5A}});

	LocalStudioProvider provider(path);
	const auto resolved = provider.resolve();

	REQUIRE(resolved.has_value());
	CHECK(resolved->executable == path);
}

TEST_CASE("the local provider refuses a missing file")
{
	LocalStudioProvider provider("does/not/exist.exe");

	const auto resolved = provider.resolve();

	REQUIRE_FALSE(resolved.has_value());
	CHECK(resolved.error().code() == ErrorCode::fetch);
}
