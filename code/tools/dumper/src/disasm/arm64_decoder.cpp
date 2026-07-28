#include "disasm/arm64_decoder.hpp"

#include <capstone/capstone.h>

#include <array>

namespace rml::dumper::disasm
{
	class Arm64Decoder::Session
	{
	public:
		Session() { m_opened = cs_open(CS_ARCH_ARM64, CS_MODE_LITTLE_ENDIAN, &m_handle) == CS_ERR_OK; }

		~Session()
		{
			if (m_opened)
				cs_close(&m_handle);
		}

		Session(const Session&) = delete;
		Session& operator=(const Session&) = delete;

		[[nodiscard]] bool opened() const { return m_opened; }
		[[nodiscard]] csh handle() const { return m_handle; }

		void enable_details() const
		{
			if (m_opened)
				cs_option(m_handle, CS_OPT_DETAIL, CS_OPT_ON);
		}

	private:
		csh m_handle{};
		bool m_opened{false};
	};

	struct FoldedBase
	{
		Register base{Register::none};
		std::int64_t offset{};
	};

	class Arm64Folds
	{
	public:
		static constexpr std::size_t slots = 31;

		[[nodiscard]] static std::size_t index_of(const Register value)
		{
			const auto raw = static_cast<std::size_t>(value);
			const auto first = static_cast<std::size_t>(Register::x0);

			if (raw < first || raw > static_cast<std::size_t>(Register::x30))
				return slots;

			return raw - first;
		}

		void forget(const Register value)
		{
			if (const auto index = index_of(value); index < slots)
				m_folds[index].reset();
		}

		void remember(const Register destination, const FoldedBase& fold)
		{
			if (const auto index = index_of(destination); index < slots)
				m_folds[index] = fold;
		}

		[[nodiscard]] FoldedBase resolve(const Register value) const
		{
			if (const auto index = index_of(value); index < slots && m_folds[index])
				return *m_folds[index];

			return {value, 0};
		}

	private:
		std::array<std::optional<FoldedBase>, slots> m_folds;
	};

	static Register to_register(const arm64_reg value)
	{
		if (value >= ARM64_REG_X0 && value <= ARM64_REG_X28)
			return arm64_general_register(static_cast<std::uint8_t>(value - ARM64_REG_X0));
		if (value >= ARM64_REG_W0 && value <= ARM64_REG_W30)
			return arm64_general_register(static_cast<std::uint8_t>(value - ARM64_REG_W0));

		switch (value)
		{
		case ARM64_REG_FP: return Register::x29;
		case ARM64_REG_LR: return Register::x30;
		case ARM64_REG_SP:
		case ARM64_REG_WSP: return Register::sp;
		case ARM64_REG_XZR:
		case ARM64_REG_WZR: return Register::zr;
		default: return Register::none;
		}
	}

	static bool is_word_register(const arm64_reg value)
	{
		return (value >= ARM64_REG_W0 && value <= ARM64_REG_W30) || value == ARM64_REG_WZR ||
		       value == ARM64_REG_WSP;
	}

	static std::uint8_t vector_register_width(const arm64_reg value)
	{
		if (value >= ARM64_REG_B0 && value <= ARM64_REG_B31)
			return 1;
		if (value >= ARM64_REG_H0 && value <= ARM64_REG_H31)
			return 2;
		if (value >= ARM64_REG_S0 && value <= ARM64_REG_S31)
			return 4;
		if (value >= ARM64_REG_D0 && value <= ARM64_REG_D31)
			return 8;
		if (value >= ARM64_REG_Q0 && value <= ARM64_REG_Q31)
			return 16;

		return 0;
	}

	static bool is_store(const unsigned int id)
	{
		switch (id)
		{
		case ARM64_INS_STR:
		case ARM64_INS_STRB:
		case ARM64_INS_STRH:
		case ARM64_INS_STUR:
		case ARM64_INS_STURB:
		case ARM64_INS_STURH:
		case ARM64_INS_STP:
		case ARM64_INS_STNP:
			return true;
		default:
			return false;
		}
	}

	static bool is_memory_instruction(const unsigned int id)
	{
		switch (id)
		{
		case ARM64_INS_LDR:
		case ARM64_INS_LDRB:
		case ARM64_INS_LDRH:
		case ARM64_INS_LDRSB:
		case ARM64_INS_LDRSH:
		case ARM64_INS_LDRSW:
		case ARM64_INS_LDUR:
		case ARM64_INS_LDURB:
		case ARM64_INS_LDURH:
		case ARM64_INS_LDURSB:
		case ARM64_INS_LDURSH:
		case ARM64_INS_LDURSW:
		case ARM64_INS_LDP:
		case ARM64_INS_LDNP:
			return true;
		default:
			return is_store(id);
		}
	}

	static std::uint8_t access_width(const unsigned int id, const arm64_reg value)
	{
		switch (id)
		{
		case ARM64_INS_LDRB:
		case ARM64_INS_STRB:
		case ARM64_INS_LDRSB:
		case ARM64_INS_LDURB:
		case ARM64_INS_STURB:
		case ARM64_INS_LDURSB:
			return 1;
		case ARM64_INS_LDRH:
		case ARM64_INS_STRH:
		case ARM64_INS_LDRSH:
		case ARM64_INS_LDURH:
		case ARM64_INS_STURH:
		case ARM64_INS_LDURSH:
			return 2;
		case ARM64_INS_LDRSW:
		case ARM64_INS_LDURSW:
			return 4;
		default:
			return is_word_register(value) ? 4 : 8;
		}
	}

	static bool is_pair(const unsigned int id)
	{
		return id == ARM64_INS_LDP || id == ARM64_INS_STP || id == ARM64_INS_LDNP || id == ARM64_INS_STNP;
	}

	Arm64Decoder::Arm64Decoder() :
	    m_session(std::make_unique<Session>())
	{
		m_session->enable_details();
	}

	Arm64Decoder::~Arm64Decoder() = default;
	Arm64Decoder::Arm64Decoder(Arm64Decoder&&) noexcept = default;
	Arm64Decoder& Arm64Decoder::operator=(Arm64Decoder&&) noexcept = default;

	std::expected<Trace, Error> Arm64Decoder::trace(const std::span<const std::byte> code, const Rva begin) const
	{
		if (!m_session->opened())
			return std::unexpected(Error::make(ErrorCode::recovery, "cannot initialise the arm64 decoder"));

		cs_insn* instruction = cs_malloc(m_session->handle());
		if (instruction == nullptr)
			return std::unexpected(Error::make(ErrorCode::recovery, "cannot allocate an arm64 instruction"));

		Trace trace;
		trace.begin = begin;
		trace.end = begin;

		Arm64Folds folds;
		std::size_t sequence = 0;

		const auto* cursor = reinterpret_cast<const std::uint8_t*>(code.data());
		std::size_t remaining = code.size();
		std::uint64_t address = begin;

		while (cs_disasm_iter(m_session->handle(), &cursor, &remaining, &address, instruction))
		{
			if (instruction->id == ARM64_INS_RET)
			{
				trace.end = static_cast<Rva>(address);
				break;
			}

			const auto here = static_cast<Rva>(instruction->address);
			const auto& detail = instruction->detail->arm64;

			if (instruction->id == ARM64_INS_BL || instruction->id == ARM64_INS_BLR)
			{
				CallSite call;
				call.sequence = sequence++;
				call.address = here;

				if (instruction->id == ARM64_INS_BL && detail.op_count > 0 &&
				    detail.operands[0].type == ARM64_OP_IMM)
					call.target = static_cast<Rva>(detail.operands[0].imm);

				trace.calls.push_back(call);
			}

			if (is_memory_instruction(instruction->id))
			{
				for (std::uint8_t i = 0; i < detail.op_count; ++i)
				{
					const auto& operand = detail.operands[i];
					if (operand.type != ARM64_OP_MEM)
						continue;

					const auto raw_base = to_register(static_cast<arm64_reg>(operand.mem.base));
					if (raw_base == Register::none || raw_base == Register::sp)
						break;

					const auto fold = folds.resolve(raw_base);
					const auto pair = is_pair(instruction->id);
					const std::uint8_t values = pair ? 2 : 1;

					for (std::uint8_t v = 0; v < values; ++v)
					{
						const auto value_operand = detail.operands[v];
						if (value_operand.type != ARM64_OP_REG)
							continue;

						const auto value = static_cast<arm64_reg>(value_operand.reg);
						const auto vector_width = vector_register_width(value);
						const auto width = vector_width != 0 ? vector_width : access_width(instruction->id, value);

						MemoryAccess access;
						access.sequence = sequence++;
						access.address = here;
						access.base = fold.base;
						access.index = to_register(static_cast<arm64_reg>(operand.mem.index));
						access.width = width;
						access.displacement = operand.mem.disp + fold.offset + v * width;
						access.is_write = is_store(instruction->id);
						access.value_register = to_register(value);

						trace.accesses.push_back(access);
					}

					break;
				}
			}

			for (std::uint8_t i = 0; i < detail.op_count; ++i)
			{
				const auto& operand = detail.operands[i];
				if (operand.type == ARM64_OP_REG && (operand.access & CS_AC_WRITE) != 0)
					folds.forget(to_register(static_cast<arm64_reg>(operand.reg)));
			}

			if ((instruction->id == ARM64_INS_ADD || instruction->id == ARM64_INS_SUB) && detail.op_count == 3 &&
			    detail.operands[0].type == ARM64_OP_REG && detail.operands[1].type == ARM64_OP_REG &&
			    detail.operands[2].type == ARM64_OP_IMM)
			{
				auto amount = detail.operands[2].imm;
				if (detail.operands[2].shift.type == ARM64_SFT_LSL)
					amount <<= detail.operands[2].shift.value;
				if (instruction->id == ARM64_INS_SUB)
					amount = -amount;

				const auto source = folds.resolve(to_register(static_cast<arm64_reg>(detail.operands[1].reg)));
				folds.remember(to_register(static_cast<arm64_reg>(detail.operands[0].reg)),
				               {source.base, source.offset + amount});
			}
			else if (instruction->id == ARM64_INS_MOV && detail.op_count == 2 &&
			         detail.operands[0].type == ARM64_OP_REG && detail.operands[1].type == ARM64_OP_REG)
			{
				const auto source = folds.resolve(to_register(static_cast<arm64_reg>(detail.operands[1].reg)));
				folds.remember(to_register(static_cast<arm64_reg>(detail.operands[0].reg)), source);
			}

			trace.end = static_cast<Rva>(address);
		}

		cs_free(instruction, 1);

		return trace;
	}
}
