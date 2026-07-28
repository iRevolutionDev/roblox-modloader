#include "rml/dumper/fetch/http_client.hpp"

namespace rml::dumper::fetch
{
	std::expected<HttpRangeSource, Error> HttpRangeSource::open(const HttpClient& client, std::string url)
	{
		const auto length = client.content_length(url);
		if (!length)
			return std::unexpected(length.error());

		if (*length == 0)
			return std::unexpected(Error::make(ErrorCode::fetch, "{} reports no length", url));

		return HttpRangeSource(client, std::move(url), *length);
	}

	std::expected<std::vector<std::byte>, Error> HttpRangeSource::read(const std::uint64_t offset,
	                                                                   const std::uint64_t length)
	{
		if (length == 0)
			return std::vector<std::byte>{};

		if (offset + length > m_size)
			return std::unexpected(Error::make(ErrorCode::fetch, "{} has {} bytes, cannot read {} at {}", m_url,
			                                   m_size, length, offset));

		auto bytes = m_client->get_range(m_url, offset, offset + length - 1);
		if (!bytes)
			return std::unexpected(bytes.error());

		if (bytes->size() != length)
			return std::unexpected(Error::make(ErrorCode::fetch, "{} returned {} bytes for a {} byte range", m_url,
			                                   bytes->size(), length));

		m_bytes_read += length;

		return bytes;
	}
}
