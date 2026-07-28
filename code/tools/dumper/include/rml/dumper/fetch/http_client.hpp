#pragma once

#include "rml/dumper/fetch/byte_source.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace rml::dumper::fetch
{
	class HttpClient
	{
	public:
		virtual ~HttpClient() = default;

		[[nodiscard]] virtual std::expected<std::vector<std::byte>, Error> get(std::string_view url) const = 0;
		[[nodiscard]] virtual std::expected<std::vector<std::byte>, Error> get_range(std::string_view url,
		                                                                             std::uint64_t from,
		                                                                             std::uint64_t to) const = 0;
		[[nodiscard]] virtual std::expected<std::uint64_t, Error> content_length(std::string_view url) const = 0;

		[[nodiscard]] static std::unique_ptr<HttpClient> create();
	};

	class HttpRangeSource final : public ByteSource
	{
	public:
		[[nodiscard]] static std::expected<HttpRangeSource, Error> open(const HttpClient& client, std::string url);

		[[nodiscard]] std::uint64_t size() const override { return m_size; }
		[[nodiscard]] std::expected<std::vector<std::byte>, Error> read(std::uint64_t offset,
		                                                               std::uint64_t length) override;

		[[nodiscard]] std::uint64_t bytes_read() const { return m_bytes_read; }

	private:
		HttpRangeSource(const HttpClient& client, std::string url, const std::uint64_t size) :
		    m_client(&client),
		    m_url(std::move(url)),
		    m_size(size)
		{
		}

		const HttpClient* m_client;
		std::string m_url;
		std::uint64_t m_size{};
		std::uint64_t m_bytes_read{};
	};
}
