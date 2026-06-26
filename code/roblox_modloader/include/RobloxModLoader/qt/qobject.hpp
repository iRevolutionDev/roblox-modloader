#pragma once

namespace rml::qt
{
	class QObject
	{
	public:
		[[nodiscard]] void* handle() const
		{
			return m_this;
		}

		[[nodiscard]] bool valid() const
		{
			return m_this != nullptr;
		}

	protected:
		QObject() = default;

		explicit QObject(void* instance) :
		    m_this(instance)
		{
		}

		~QObject() = default;
		
		void* m_this = nullptr;
	};
}
