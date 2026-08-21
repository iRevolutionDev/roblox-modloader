#pragma once

namespace rbx::signals
{
	class slots_holder;
}

namespace boost
{
	template<typename T>
	class intrusive_ptr
	{
	public:
		T* px;
	};
}
