#include "rml/dumper/fetch/http_client.hpp"

#include <curl/curl.h>

#include <format>
#include <mutex>
#include <string>

namespace rml::dumper::fetch
{
	class CurlHttpClient final : public HttpClient
	{
	public:
		CurlHttpClient()
		{
			static std::once_flag once;
			std::call_once(once, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
		}

		[[nodiscard]] std::expected<std::vector<std::byte>, Error> get(const std::string_view url) const override
		{
			return request(url, {}, false);
		}

		[[nodiscard]] std::expected<std::vector<std::byte>, Error> get_range(const std::string_view url,
		                                                                     const std::uint64_t from,
		                                                                     const std::uint64_t to) const override
		{
			return request(url, std::format("{}-{}", from, to), false);
		}

		[[nodiscard]] std::expected<std::uint64_t, Error> content_length(const std::string_view url) const override
		{
			const auto probed = request(url, {}, true);
			if (!probed)
				return std::unexpected(probed.error());

			return m_last_length;
		}

	private:
		struct HandleDeleter
		{
			void operator()(CURL* handle) const
			{
				if (handle != nullptr)
					curl_easy_cleanup(handle);
			}
		};

		static std::size_t collect(char* data, const std::size_t size, const std::size_t count, void* target)
		{
			auto* body = static_cast<std::vector<std::byte>*>(target);
			const auto total = size * count;

			const auto offset = body->size();
			body->resize(offset + total);
			std::memcpy(body->data() + offset, data, total);

			return total;
		}

		[[nodiscard]] std::expected<std::vector<std::byte>, Error> request(const std::string_view url,
		                                                                   const std::string& range,
		                                                                   const bool head_only) const
		{
			const std::unique_ptr<CURL, HandleDeleter> handle(curl_easy_init());
			if (!handle)
				return std::unexpected(Error::make(ErrorCode::fetch, "cannot create a curl handle"));

			const std::string target(url);
			std::vector<std::byte> body;

			curl_easy_setopt(handle.get(), CURLOPT_URL, target.c_str());
			curl_easy_setopt(handle.get(), CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(handle.get(), CURLOPT_USERAGENT, "rml-dumper/1.0");
			curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION, &CurlHttpClient::collect);
			curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, &body);

			if (head_only)
				curl_easy_setopt(handle.get(), CURLOPT_NOBODY, 1L);
			if (!range.empty())
				curl_easy_setopt(handle.get(), CURLOPT_RANGE, range.c_str());

			if (const auto result = curl_easy_perform(handle.get()); result != CURLE_OK)
				return std::unexpected(
				    Error::make(ErrorCode::fetch, "{} failed: {}", url, curl_easy_strerror(result)));

			long status = 0;
			curl_easy_getinfo(handle.get(), CURLINFO_RESPONSE_CODE, &status);

			if (status != 200 && status != 206)
				return std::unexpected(Error::make(ErrorCode::fetch, "{} answered {}", url, status));

			if (!range.empty() && status != 206)
				return std::unexpected(
				    Error::make(ErrorCode::fetch, "{} ignored the range and answered {}", url, status));

			curl_off_t length = 0;
			curl_easy_getinfo(handle.get(), CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &length);
			m_last_length = length > 0 ? static_cast<std::uint64_t>(length) : 0;

			return body;
		}

		mutable std::uint64_t m_last_length{};
	};

	std::unique_ptr<HttpClient> HttpClient::create()
	{
		return std::make_unique<CurlHttpClient>();
	}
}
