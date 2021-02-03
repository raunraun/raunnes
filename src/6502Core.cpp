#include "6502Core.h"
#include <cstdint>
#include <functional>

namespace raunnes {

typedef void (CPUCore6502::*ExecutionDelegate)(CPUCore6502::DynamicExecutionInfo&);

struct InstructionDetails {
	uint32_t AddresingMode;
	uint32_t InstructionSize;
	uint32_t CycleCount;
	uint32_t PageCrossCycleCost;
	char* Name;
	ExecutionDelegate Delegate;
};

static  InstructionDetails g_InstructionDetails[256] = {
	#include "InstructionDetails.csv"
};

 CPUCore6502::CPUCore6502(MemoryMap& mem) : m_Memory(mem) {
	 reset();
}

 CPUCore6502::~CPUCore6502() {

 }

void CPUCore6502::reset() {
	m_State.SP = 0xFD;
	m_State.PC = m_Memory.Read16(0xfffc);
	m_State.PC = 0xc000;

	m_State.A = 0;
	m_State.X = 0;
	m_State.Y = 0;

	m_State.SetFlags(0x24);

	m_Cycles = 0;

}

void CPUCore6502::Execute() {
	uint8_t opcode = m_Memory.Read(m_State.PC);
	InstructionDetails details = g_InstructionDetails[opcode];
	DynamicExecutionInfo info = { 0 };

	m_State.PC += 1;

    switch (details.AddresingMode) {
    case(AddressingModeAbsolute):
		info.Address = m_Memory.Read16(m_State.PC);
		break;
    case(AddressingModeAbsoluteX): break;
    case(AddressingModeAbsoluteY): break;
    case(AddressingModeAccumulator): break;
    case(AddressingModeImmediate): break;
    case(AddressingModeImplied): break;
    case(AddressingModeIndexedIndirect): break;
    case(AddressingModeIndirect): break;
    case(AddressingModeIndirectIndexed): break;
    case(AddressingModeRelative): break;
    case(AddressingModeZeroPage): break;
    case(AddressingModeZeroPageX): break;
    case(AddressingModeZeroPageY): break;
    };

	m_Cycles += details.CycleCount;
	m_State.PC += details.InstructionSize;

	std::invoke(details.Delegate, this, info);
}

void CPUCore6502::JMP(DynamicExecutionInfo& info) {
	m_State.PC = info.Address;
}

}