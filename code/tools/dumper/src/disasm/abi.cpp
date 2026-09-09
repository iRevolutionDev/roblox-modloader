#include "rml/dumper/disasm/abi.hpp"

#include <array>

namespace rml::dumper::disasm
{
	template<std::size_t N>
	class RegisterAbi : public Abi
	{
	public:
		constexpr RegisterAbi(const std::string_view name, const std::array<Register, N> arguments,
		                      const Register return_register) :
		    m_name(name),
		    m_arguments(arguments),
		    m_return(return_register)
		{
		}

		[[nodiscard]] std::string_view name() const override { return m_name; }
		[[nodiscard]] Register return_value() const override { return m_return; }

		[[nodiscard]] Register argument(const std::size_t index) const override
		{
			return index < m_arguments.size() ? m_arguments[index] : Register::none;
		}

	private:
		std::string_view m_name;
		std::array<Register, N> m_arguments;
		Register m_return;
	};

	const Abi& Abi::windows_x64()
	{
		static const RegisterAbi<4> abi("windows-x64",
		                                {Register::rcx, Register::rdx, Register::r8, Register::r9}, Register::rax);
		return abi;
	}

	const Abi& Abi::system_v_x64()
	{
		static const RegisterAbi<6> abi(
		    "system-v-x64", {Register::rdi, Register::rsi, Register::rdx, Register::rcx, Register::r8, Register::r9},
		    Register::rax);
		return abi;
	}

	const Abi& Abi::aapcs64()
	{
		static const RegisterAbi<8> abi("aapcs64",
		                                {Register::x0, Register::x1, Register::x2, Register::x3, Register::x4,
		                                 Register::x5, Register::x6, Register::x7},
		                                Register::x0);
		return abi;
	}
}
