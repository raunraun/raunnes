#include "6502Core.h"
#include <cstdint>
#include <functional>

namespace raunnes {

static  CPUCore6502::InstructionDetails g_InstructionDetails[256] = {
	#include "InstructionDetails.csv"
};

 CPUCore6502::CPUCore6502(MemoryMap& mem) : 
	 // m_State(),
	 m_Cycles(0),
	 m_Memory(mem),
	 m_PreexecutionCallBack(nullptr),
	 m_PostexecutionCallBack(nullptr) {
	
	 Reset();
}

 CPUCore6502::~CPUCore6502() {
 }

 void CPUCore6502::InstallPreExecutionCallBack(ExecutionCallBack cb) {
	 m_PreexecutionCallBack = cb;
 }

 void CPUCore6502::InstallPostExecutionCallBack(ExecutionCallBack cb) {
	 m_PostexecutionCallBack = cb;
 }

void CPUCore6502::Reset() {
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

	if (m_PreexecutionCallBack != nullptr) {
		std::invoke(m_PreexecutionCallBack, details, info, m_State);
	}

	std::invoke(details.Delegate, this, info);
}

void CPUCore6502::JMP(DynamicExecutionInfo& info) {
	m_State.PC = info.Address;
}

}