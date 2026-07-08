#pragma once
#include "type.hpp"

#include <vector>

namespace RBX::Reflection
{
	typedef std::vector<Variant> EventArguments;

	class SystemAddress
	{
		unsigned int m_binary_address;
		unsigned short m_port;


	public:
		SystemAddress() :
		    m_binary_address(0xFFFFFFFF),
		    m_port(0xFFFF)
		{
		}

		SystemAddress(const unsigned int binary_address, const unsigned short port) :
		    m_binary_address(binary_address),
		    m_port(port)
		{
		}

		bool empty() const
		{
			return m_binary_address == 0xFFFFFFFF && m_port == 0xFFFF;
		}

		void clear()
		{
			m_binary_address = 0xFFFFFFFF;
			m_port           = 0xFFFF;
		}

		[[nodiscard]] unsigned int address() const
		{
			return m_binary_address;
		}
		
		[[nodiscard]] unsigned short port() const
		{
			return m_port;
		}

		bool operator==(const SystemAddress& right) const;
		bool operator!=(const SystemAddress& right) const;
		bool operator>(const SystemAddress& right) const;
		bool operator<(const SystemAddress& right) const;
	};

	class EventDescriptor;
	class EventSource
	{
	public:
		virtual ~EventSource()
		{
		}

		virtual void process_remote_event(const EventDescriptor& descriptor, const EventArguments& args, const SystemAddress& source);
		virtual void raise_event_invocation(const EventDescriptor& descriptor, const EventArguments& args, const SystemAddress* target = NULL);
		virtual bool use_submit_task_for_lua_listeners() const
		{
			return false;
		}
	};
}