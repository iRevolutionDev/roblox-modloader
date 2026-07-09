#pragma once
#include "dumper/memory/batch.hpp"
#include "dumper/memory/module.hpp"
#include "dumper/util/compile_time_helpers.hpp"
#include "luau.hpp"

namespace dumper
{
	class pointers
	{
		static constexpr auto get_luau_batch();

		template<cstxpr_str batch_name, size_t N>
		bool run_batch(const memory::batch<N>& batch, const memory::module& mem_region)
		{
			return memory::batch_runner::run(batch, mem_region);
		}

	public:
		explicit pointers(const memory::module& region);
		~pointers();

		luau m_luau_functions;
	};

	inline pointers* g_dumper_pointers{nullptr};

}
