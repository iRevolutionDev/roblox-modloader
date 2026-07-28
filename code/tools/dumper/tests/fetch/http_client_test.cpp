#include <doctest/doctest.h>

#include "rml/dumper/fetch/http_client.hpp"
#include "rml/dumper/fetch/zip_reader.hpp"

#include <cstdlib>

using namespace rml::dumper;
using namespace rml::dumper::fetch;

static bool network_allowed()
{
	const char* value = std::getenv("RML_TEST_NETWORK");
	return value != nullptr && *value == '1';
}

TEST_CASE("the http client reads a length and a byte range from the studio cdn")
{
	if (!network_allowed())
	{
		MESSAGE("RML_TEST_NETWORK is not 1, skipping");
		return;
	}

	const auto client = HttpClient::create();
	REQUIRE(client != nullptr);

	const auto version = client->get("https://clientsettingscdn.roblox.com/v2/client-version/WindowsStudio64");
	REQUIRE(version.has_value());

	const std::string body(reinterpret_cast<const char*>(version->data()), version->size());
	CHECK(body.find("clientVersionUpload") != std::string::npos);
	MESSAGE("version response: ", body);

	const std::string_view manifest = "https://setup.rbxcdn.com/mac/arm64/";

	const auto length = client->content_length("https://setup.rbxcdn.com/version-14d8b191232f4ddd-RobloxStudio.zip");
	if (!length)
	{
		MESSAGE("package head failed: ", length.error().message());
		return;
	}

	CHECK(*length > 0);
	MESSAGE("package is ", *length, " bytes");

	const auto head = client->get_range("https://setup.rbxcdn.com/version-14d8b191232f4ddd-RobloxStudio.zip", 0, 3);
	REQUIRE(head.has_value());
	CHECK(head->size() == 4);
	CHECK(head->at(0) == std::byte{0x50});
	CHECK(head->at(1) == std::byte{0x4B});
}

TEST_CASE("a ranged source over http lists a real studio archive")
{
	if (!network_allowed())
	{
		MESSAGE("RML_TEST_NETWORK is not 1, skipping");
		return;
	}

	const auto client = HttpClient::create();
	auto source = HttpRangeSource::open(*client,
	                                    "https://setup.rbxcdn.com/version-14d8b191232f4ddd-RobloxStudio.zip");
	if (!source)
	{
		MESSAGE("cannot open the package: ", source.error().message());
		return;
	}

	const auto reader = ZipReader::open(*source);
	REQUIRE(reader.has_value());

	MESSAGE("archive is ", source->size(), " bytes with ", reader->entries().size(), " entries, listed after ",
	        source->bytes_read(), " bytes read");

	CHECK(reader->entries().size() > 1);
	CHECK(source->bytes_read() < source->size() / 4);
	CHECK(reader->find("RobloxStudioBeta.exe") != nullptr);
}
