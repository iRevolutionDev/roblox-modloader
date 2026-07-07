#pragma once
#include "fwddec.hpp"
#include "handle.hpp"

#include <optional>

namespace rml::memory
{
	class RML_EXPORT range
	{
	public:
		range(handle base, std::size_t size);

		handle begin() const;

		handle end() const;

		std::size_t size() const;

		bool contains(handle h) const;

		std::optional<handle> scan(pattern const& sig) const;

	protected:
		handle m_base;
		std::size_t m_size;
	};
}
