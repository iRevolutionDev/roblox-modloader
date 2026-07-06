#pragma once

#include "rbx_str.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace RBX
{
	class Name
	{
	public:
		rbx_str const str;

		static std::size_t size();
		static std::size_t approximate_memory_usage();

		static const Name& get_null_name();

		static const Name& declare(const char* s_name);
		static const Name& declare(std::string_view s_name)
		{
			return declare(s_name.data());
		}

		static const Name& lookup(const char* s_name);
		static const Name& lookup(std::string_view s_name)
		{
			return lookup(s_name.data());
		}

		[[nodiscard]] bool empty() const
		{
			return get_null_name() == *this;
		}
		static bool empty(const Name* name)
		{
			return name == nullptr || *name == get_null_name();
		}

		[[nodiscard]] std::string_view to_string() const noexcept
		{
			return str.sv();
		}
		[[nodiscard]] const char* c_str() const
		{
			return str.c_str();
		}

		static int compare(const Name& a, const Name& b)
		{
			return static_cast<int>(a.sort_index) - static_cast<int>(b.sort_index);
		}
		int compare(const Name& other) const
		{
			return static_cast<int>(sort_index) - static_cast<int>(other.sort_index);
		}

		bool operator<(const Name& other) const
		{
			return sort_index < other.sort_index;
		}
		bool operator>(const Name& other) const
		{
			return sort_index > other.sort_index;
		}

		bool operator==(const Name& other) const
		{
			return this == &other;
		}
		bool operator!=(const Name& other) const
		{
			return this != &other;
		}

		bool operator==(const std::string& s) const
		{
			return str == s;
		}
		bool operator!=(const std::string& s) const
		{
			return str != s;
		}

		bool operator==(const char* s) const
		{
			return str == s;
		}
		bool operator!=(const char* s) const
		{
			return str != s;
		}

	private:
		std::atomic<std::uint32_t> sort_index{0};

		explicit Name(const char* s_name);
		void set_order_index();

		static std::mutex& get_mutex();

		struct StrHash
		{
			std::size_t operator()(const char* s) const noexcept
			{
				return std::hash<std::string_view>()(std::string_view(s));
			}
		};
		struct StrEqual
		{
			bool operator()(const char* a, const char* b) const noexcept
			{
				return std::strcmp(a, b) == 0;
			}
		};

		using NameMap = std::unordered_map<const char*, Name*, StrHash, StrEqual>;

		static NameMap& get_map();
	};

	std::ostream& operator<<(std::ostream& os, const Name& name);

	class INamed
	{
	public:
		virtual ~INamed()                    = default;
		virtual const Name& get_name() const = 0;
	};
} // namespace RBX
