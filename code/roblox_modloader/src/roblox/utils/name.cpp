#include "RobloxModLoader/roblox/util/name.hpp"

#include <cassert>
#include <vector>

namespace RBX
{
	std::mutex& Name::get_mutex()
	{
		static std::mutex s_mutex;
		return s_mutex;
	}

	Name::NameMap& Name::get_map()
	{
		static NameMap s_map;
		return s_map;
	}

	Name::Name(const char* s_name) :
	    str(s_name)
	{
		set_order_index();
	}

	void Name::set_order_index()
	{
		static std::vector<Name*> s_ordered;

		std::size_t i = s_ordered.size();
		while (i > 0 && str < s_ordered[i - 1]->str)
		{
			s_ordered[i - 1]->sort_index.fetch_add(1, std::memory_order_relaxed);
			--i;
		}
		sort_index.store(static_cast<std::uint32_t>(i), std::memory_order_relaxed);
		s_ordered.insert(s_ordered.begin() + static_cast<std::ptrdiff_t>(i), this);

#ifdef _DEBUG
		for (std::size_t j = 0; j < s_ordered.size(); ++j)
			assert(s_ordered[j]->sort_index.load(std::memory_order_relaxed) == static_cast<std::uint32_t>(j));
#endif
	}

	const Name& Name::get_null_name()
	{
		static const Name& s_null = declare("");
		return s_null;
	}

	const Name& Name::declare(const char* s_name)
	{
		if (s_name == nullptr)
			return get_null_name();

		std::lock_guard<std::mutex> lock(get_mutex());

		auto& mp = get_map();
		auto it  = mp.find(s_name);
		if (it != mp.end())
			return *it->second;

		Name* name        = new Name(s_name);
		mp[name->c_str()] = name;
		return *name;
	}

	const Name& Name::lookup(const char* s_name)
	{
		if (s_name == nullptr)
			return get_null_name();

		const Name* result = nullptr;
		{
			std::lock_guard<std::mutex> lock(get_mutex());

			const auto& mp = get_map();
			auto it        = mp.find(s_name);
			if (it != mp.end())
				result = it->second;
		}

		return result != nullptr ? *result : get_null_name();
	}

	std::size_t Name::size()
	{
		std::lock_guard<std::mutex> lock(get_mutex());
		return get_map().size();
	}

	std::size_t Name::approximate_memory_usage()
	{
		std::lock_guard<std::mutex> lock(get_mutex());
		return 64u * get_map().size();
	}

	std::ostream& operator<<(std::ostream& os, const Name& name)
	{
		return os << name.c_str();
	}

} // namespace RBX
