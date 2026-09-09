#include "rml/dumper/fetch/http_client.hpp"

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif

#include <windows.h>
#include <winhttp.h>

#include <format>
#include <memory>

namespace rml::dumper::fetch
{
	class WinHttpClient final : public HttpClient
	{
	public:
		WinHttpClient()
		{
			m_session = WinHttpOpen(L"rml-dumper/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
			                        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		}

		~WinHttpClient() override
		{
			if (m_session != nullptr)
				WinHttpCloseHandle(m_session);
		}

		WinHttpClient(const WinHttpClient&) = delete;
		WinHttpClient& operator=(const WinHttpClient&) = delete;

		[[nodiscard]] std::expected<std::vector<std::byte>, Error> get(const std::string_view url) const override
		{
			return request(url, {}, false);
		}

		[[nodiscard]] std::expected<std::vector<std::byte>, Error> get_range(const std::string_view url,
		                                                                     const std::uint64_t from,
		                                                                     const std::uint64_t to) const override
		{
			return request(url, std::format(L"Range: bytes={}-{}", from, to), false);
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
			void operator()(void* handle) const
			{
				if (handle != nullptr)
					WinHttpCloseHandle(handle);
			}
		};

		using Handle = std::unique_ptr<void, HandleDeleter>;

		[[nodiscard]] static std::wstring widen(const std::string_view text)
		{
			if (text.empty())
				return {};

			const auto length = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
			                                        nullptr, 0);
			std::wstring wide(static_cast<std::size_t>(length), L'\0');
			MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wide.data(), length);

			return wide;
		}

		[[nodiscard]] std::expected<std::vector<std::byte>, Error> request(const std::string_view url,
		                                                                   const std::wstring& headers,
		                                                                   const bool head_only) const
		{
			if (m_session == nullptr)
				return std::unexpected(Error::make(ErrorCode::fetch, "cannot open a winhttp session"));

			const auto wide = widen(url);

			URL_COMPONENTS components{};
			components.dwStructSize = sizeof(components);
			components.dwHostNameLength = static_cast<DWORD>(-1);
			components.dwUrlPathLength = static_cast<DWORD>(-1);
			components.dwExtraInfoLength = static_cast<DWORD>(-1);

			if (!WinHttpCrackUrl(wide.c_str(), 0, 0, &components))
				return std::unexpected(Error::make(ErrorCode::fetch, "cannot parse {}", url));

			const std::wstring host(components.lpszHostName, components.dwHostNameLength);
			std::wstring path(components.lpszUrlPath, components.dwUrlPathLength);
			path.append(components.lpszExtraInfo, components.dwExtraInfoLength);

			const Handle connection(WinHttpConnect(m_session, host.c_str(), components.nPort, 0));
			if (!connection)
				return std::unexpected(Error::make(ErrorCode::fetch, "cannot connect to {}", url));

			const auto secure = components.nScheme == INTERNET_SCHEME_HTTPS;
			const Handle request(WinHttpOpenRequest(connection.get(), head_only ? L"HEAD" : L"GET", path.c_str(),
			                                        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
			                                        secure ? WINHTTP_FLAG_SECURE : 0u));
			if (!request)
				return std::unexpected(Error::make(ErrorCode::fetch, "cannot open a request for {}", url));

			if (!headers.empty())
				WinHttpAddRequestHeaders(request.get(), headers.c_str(), static_cast<DWORD>(-1),
				                         WINHTTP_ADDREQ_FLAG_ADD);

			if (!WinHttpSendRequest(request.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0,
			                        0) ||
			    !WinHttpReceiveResponse(request.get(), nullptr))
				return std::unexpected(
				    Error::make(ErrorCode::fetch, "request to {} failed with {}", url, GetLastError()));

			DWORD status = 0;
			DWORD status_size = sizeof(status);
			WinHttpQueryHeaders(request.get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			                    WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size, WINHTTP_NO_HEADER_INDEX);

			if (status != 200 && status != 206)
				return std::unexpected(Error::make(ErrorCode::fetch, "{} answered {}", url, status));

			if (headers.empty() && !head_only && status == 206)
				return std::unexpected(
				    Error::make(ErrorCode::fetch, "{} answered 206 for a request with no range", url));

			if (!headers.empty() && status != 206)
				return std::unexpected(
				    Error::make(ErrorCode::fetch, "{} ignored the range and answered {}", url, status));

			ULONGLONG length = 0;
			DWORD length_size = sizeof(length);
			if (WinHttpQueryHeaders(request.get(), WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER64,
			                        WINHTTP_HEADER_NAME_BY_INDEX, &length, &length_size, WINHTTP_NO_HEADER_INDEX))
				m_last_length = length;
			else
				m_last_length = 0;

			std::vector<std::byte> body;
			if (head_only)
				return body;

			for (;;)
			{
				DWORD available = 0;
				if (!WinHttpQueryDataAvailable(request.get(), &available) || available == 0)
					break;

				const auto offset = body.size();
				body.resize(offset + available);

				DWORD read = 0;
				if (!WinHttpReadData(request.get(), body.data() + offset, available, &read))
					return std::unexpected(Error::make(ErrorCode::fetch, "cannot read the body of {}", url));

				body.resize(offset + read);
			}

			return body;
		}

		HINTERNET m_session{};
		mutable std::uint64_t m_last_length{};
	};

	std::unique_ptr<HttpClient> HttpClient::create()
	{
		return std::make_unique<WinHttpClient>();
	}
}
