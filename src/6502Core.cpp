#include "6502Core.h"
#include <cstdint>

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