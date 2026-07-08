#pragma once

#include <utility>

namespace rml::qt
{
	template<typename T>
	class [[nodiscard]] QtOwned
	{
	public:
		QtOwned() noexcept = default;

		explicit QtOwned(T* instance) noexcept :
		    m_instance(instance)
		{
		}

		QtOwned(const QtOwned&) = delete;
		QtOwned& operator=(const QtOwned&) = delete;

		QtOwned(QtOwned&& other) noexcept :
		    m_instance(std::exchange(other.m_instance, nullptr))
		{
		}

		QtOwned& operator=(QtOwned&& other) noexcept
		{
			if (this != &other)
				reset(std::exchange(other.m_instance, nullptr));
			return *this;
		}

		~QtOwned()
		{
			reset();
		}

		void reset(T* instance = nullptr) noexcept
		{
			T* const previous = std::exchange(m_instance, instance);
			if (previous)
				T::destroy(previous);
		}

		[[nodiscard]] T* release() noexcept
		{
			return std::exchange(m_instance, nullptr);
		}

		[[nodiscard]] T* get() const noexcept
		{
			return m_instance;
		}

		[[nodiscard]] T* operator->() const noexcept
		{
			return m_instance;
		}

		[[nodiscard]] T& operator*() const noexcept
		{
			return *m_instance;
		}

		[[nodiscard]] explicit operator bool() const noexcept
		{
			return m_instance != nullptr;
		}

	private:
		T* m_instance{nullptr};
	};
}
