#include "rml/dumper/fetch/byte_source.hpp"

#include <fstream>

namespace rml::dumper::fetch
{
	FileSource::FileSource(std::filesystem::path path) :
	    m_path(std::move(path))
	{
		std::error_code ec;
		m_size = std::filesystem::file_size(m_path, ec);
		if (ec)
			m_size = 0;
	}

	std::expected<std::vector<std::byte>, Error> FileSource::read(const std::uint64_t offset,
	                                                              const std::uint64_t length)
	{
		if (offset + length > m_size)
			return std::unexpected(Error::make(ErrorCode::fetch, "{} has {} bytes, cannot read {} at {}",
			                                   m_path.string(), m_size, length, offset));

		std::ifstream stream(m_path, std::ios::binary);
		if (!stream)
			return std::unexpected(Error::make(ErrorCode::fetch, "cannot open {}", m_path.string()));

		stream.seekg(static_cast<std::streamoff>(offset));

		std::vector<std::byte> bytes(static_cast<std::size_t>(length));
		stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(length));
		if (!stream)
			return std::unexpected(Error::make(ErrorCode::fetch, "short read from {}", m_path.string()));

		m_bytes_read += length;

		return bytes;
	}
}
