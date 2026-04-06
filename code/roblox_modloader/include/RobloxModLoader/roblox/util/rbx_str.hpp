#pragma once

#include <cstdint>
#include <string_view>

namespace RBX
{
	class rbx_str
	{
	public:
		static constexpr std::size_t sso_capacity = 15u;

	private:
		static constexpr std::size_t buf_size = sso_capacity + 1u;

		union {
			char* m_ptr{};
			char m_sso[buf_size];
		};
		std::uint64_t m_size;
		std::uint64_t m_cap;

		[[nodiscard]] bool is_sso() const noexcept
		{
			return m_cap == sso_capacity;
		}

		void assign(const char* s, const std::size_t n)
		{
			m_size = n;
			if (n <= sso_capacity)
			{
				m_cap = sso_capacity;
				if (n > 0)
					std::memcpy(m_sso, s, n);
				m_sso[n] = '\0';
			}
			else
			{
				m_cap = static_cast<std::uint64_t>(n);
				m_ptr = new char[n + 1];
				std::memcpy(m_ptr, s, n);
				m_ptr[n] = '\0';
			}
		}

	public:
		rbx_str() noexcept :
		    m_size{0},
		    m_cap{sso_capacity}
		{
			m_sso[0] = '\0';
		}

		explicit rbx_str(const char* s)
		{
			assign(s, std::strlen(s));
		}
		explicit rbx_str(const std::string_view sv)
		{
			assign(sv.data(), sv.size());
		}

		~rbx_str() noexcept
		{
			if (!is_sso())
				delete[] m_ptr;
		}

		rbx_str(const rbx_str&)            = delete;
		rbx_str& operator=(const rbx_str&) = delete;
		rbx_str(rbx_str&&)                 = delete;
		rbx_str& operator=(rbx_str&&)      = delete;

		[[nodiscard]] const char* c_str() const noexcept
		{
			return is_sso() ? m_sso : m_ptr;
		}
		[[nodiscard]] const char* data() const noexcept
		{
			return c_str();
		}
		[[nodiscard]] std::size_t size() const noexcept
		{
			return m_size;
		}
		[[nodiscard]] bool empty() const noexcept
		{
			return m_size == 0;
		}

		[[nodiscard]] std::string_view sv() const noexcept
		{
			return {c_str(), size()};
		}

		bool operator<(const rbx_str& o) const noexcept
		{
			return sv() < o.sv();
		}
		bool operator==(const rbx_str& o) const noexcept
		{
			return sv() == o.sv();
		}
		bool operator!=(const rbx_str& o) const noexcept
		{
			return sv() != o.sv();
		}

		bool operator==(const std::string_view s) const noexcept
		{
			return sv() == s;
		}
		bool operator!=(const std::string_view s) const noexcept
		{
			return sv() != s;
		}

		bool operator==(const char* s) const noexcept
		{
			return s && sv() == std::string_view(s);
		}
		bool operator!=(const char* s) const noexcept
		{
			return !(*this == s);
		}
	};
} // namespace RBX
