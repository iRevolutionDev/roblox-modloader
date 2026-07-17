#pragma once

#include <atomic>
#include <utility>

namespace rml::utils
{
	template<typename T, typename Deleter>
	class intrusive_weak_ptr
	{
	public:
		constexpr intrusive_weak_ptr() noexcept = default;
		explicit intrusive_weak_ptr(T* ptr) noexcept :
		    m_ptr(ptr)
		{
		}

		intrusive_weak_ptr(const intrusive_weak_ptr& other) noexcept :
		    m_ptr(other.m_ptr)
		{
			acquire();
		}

		intrusive_weak_ptr(intrusive_weak_ptr&& other) noexcept :
		    m_ptr(std::exchange(other.m_ptr, nullptr))
		{
		}

		intrusive_weak_ptr& operator=(const intrusive_weak_ptr& other) noexcept
		{
			if (m_ptr != other.m_ptr)
			{
				T* previous = m_ptr;
				m_ptr = other.m_ptr;
				acquire();
				release(previous);
			}
			return *this;
		}

		intrusive_weak_ptr& operator=(intrusive_weak_ptr&& other) noexcept
		{
			if (this != &other)
			{
				release(m_ptr);
				m_ptr = std::exchange(other.m_ptr, nullptr);
			}
			return *this;
		}

		~intrusive_weak_ptr()
		{
			release(m_ptr);
		}

		void reset() noexcept
		{
			release(m_ptr);
			m_ptr = nullptr;
		}

		[[nodiscard]] T* get() const noexcept
		{
			return m_ptr;
		}

		explicit operator bool() const noexcept
		{
			return m_ptr != nullptr;
		}

		[[nodiscard]] bool alive() const noexcept
		{
			return m_ptr && m_ptr->strong > 0;
		}

		bool operator==(const intrusive_weak_ptr& other) const noexcept
		{
			return m_ptr == other.m_ptr;
		}
		bool operator!=(const intrusive_weak_ptr& other) const noexcept
		{
			return m_ptr != other.m_ptr;
		}

	private:
		void acquire() const noexcept
		{
			if (m_ptr)
				std::atomic_ref(m_ptr->weak).fetch_add(1, std::memory_order_seq_cst);
		}

		static void release(T* ptr) noexcept
		{
			if (ptr && std::atomic_ref(ptr->weak).fetch_sub(1, std::memory_order_seq_cst) == 1)
				Deleter{}(ptr);
		}

		T* m_ptr = nullptr;
	};
}
