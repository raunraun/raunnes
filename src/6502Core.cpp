#include "6502Core.h"
#include <cstdint>

namespace raunnes {
struct InstructionDetails {
	uint32_t AddresingMode;
	uint32_t InstructionSize;
	uint32_t CycleCount;
	uint32_t PageCrossCycleCost;
	char* Name;
};

static  InstructionDetails g_InstructionDetails[256] = {
	#include "InstructionDetails.csv"
};

 CPUCore6502::CPUCore6502(MemoryMap& mem) : m_Memory(mem) {
	 reset();
}

void CPUCore6502::reset() {
	m_State.SP = 0xFD;
	m_State.PC = 0xc000;

	m_State.A = 0;
	m_State.X = 0;
	m_State.Y = 0;

	m_State.SetFlags(0x24);

}

void CPUCore6502::Execute() {

}

}