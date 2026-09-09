#pragma once

namespace RBX
{
	template<typename T>
	class Flyweight
	{
	public:
		const T* data;

		const T& value() const
		{
			return *data;
		}
	};
}
