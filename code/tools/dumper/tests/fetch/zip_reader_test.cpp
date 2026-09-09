#include <doctest/doctest.h>

#include "rml/dumper/fetch/zip_reader.hpp"
#include "support/temporary_file.hpp"
#include "support/zip_builder.hpp"

using namespace rml::dumper;
using namespace rml::dumper::fetch;

static std::vector<std::byte> pattern_of(const std::size_t length)
{
	std::vector<std::byte> payload(length);
	for (std::size_t i = 0; i < length; ++i)
		payload[i] = static_cast<std::byte>(i * 7 + i / 251);

	return payload;
}

TEST_CASE("the zip reader lists entries from the central directory")
{
	tests::ZipBuilder builder;
	builder.add_deflated("RobloxStudio.app/Contents/MacOS/RobloxStudio", pattern_of(4096));
	builder.add_stored("Info.plist", pattern_of(2));

	FileSource source(builder.write_to_temporary_file());
	const auto reader = ZipReader::open(source);

	REQUIRE(reader.has_value());
	CHECK(reader->entries().size() == 2);
	CHECK(reader->find("Contents/MacOS/RobloxStudio") != nullptr);
	CHECK(reader->find("Info.plist") != nullptr);
	CHECK(reader->find("nope") == nullptr);
}

TEST_CASE("the zip reader inflates a deflated entry")
{
	const auto payload = pattern_of(8192);

	tests::ZipBuilder builder;
	builder.add_deflated("payload.bin", payload);

	FileSource source(builder.write_to_temporary_file());
	const auto reader = ZipReader::open(source);
	REQUIRE(reader.has_value());

	const auto* entry = reader->find("payload.bin");
	REQUIRE(entry != nullptr);
	CHECK(entry->method == 8);
	CHECK(entry->compressed_size < entry->uncompressed_size);

	const auto extracted = reader->extract(*entry);
	REQUIRE(extracted.has_value());
	CHECK(*extracted == payload);
}

TEST_CASE("the zip reader returns a stored entry untouched")
{
	const auto payload = pattern_of(64);

	tests::ZipBuilder builder;
	builder.add_stored("small.bin", payload);

	FileSource source(builder.write_to_temporary_file());
	const auto reader = ZipReader::open(source);
	REQUIRE(reader.has_value());

	const auto* entry = reader->find("small.bin");
	REQUIRE(entry != nullptr);
	CHECK(entry->method == 0);
	CHECK(*reader->extract(*entry) == payload);
}

TEST_CASE("the zip reader only touches the bytes it needs")
{
	tests::ZipBuilder builder;
	builder.add_stored("big.bin", pattern_of(4 << 20));
	builder.add_stored("target.bin", pattern_of(8));

	FileSource source(builder.write_to_temporary_file());
	const auto reader = ZipReader::open(source);
	REQUIRE(reader.has_value());

	const auto before = source.bytes_read();
	const auto extracted = reader->extract(*reader->find("target.bin"));
	REQUIRE(extracted.has_value());

	CHECK(source.bytes_read() - before < 1024);
	CHECK(source.bytes_read() < source.size() / 2);
}

TEST_CASE("the zip reader rejects a file with no directory record")
{
	const auto path = tests::TemporaryFile::write(pattern_of(1024));
	FileSource source(path);

	const auto reader = ZipReader::open(source);

	REQUIRE_FALSE(reader.has_value());
	CHECK(reader.error().code() == ErrorCode::fetch);
}

TEST_CASE("a file source refuses a read past the end")
{
	const auto path = tests::TemporaryFile::write(pattern_of(100));
	FileSource source(path);

	CHECK(source.size() == 100);
	CHECK(source.read(0, 100).has_value());
	CHECK_FALSE(source.read(50, 100).has_value());
}
