#include "6502Core.h"
#include <cassert>
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

    m_Cycles = 7;
}

void CPUCore6502::Execute() {
    uint8_t opcode = m_Memory.Read(m_State.PC);
    InstructionDetails details = g_InstructionDetails[opcode];
    DynamicExecutionInfo info(details, opcode, m_State.PC);


    for (uint32_t i = 1; i < details.InstructionSize; i++) {
        info.InstructionBytes[i] = m_Memory.Read(m_State.PC + i);
    }

    switch (details.AddresingMode) {
    case(AddressingModeAbsolute):
        break;
    case(AddressingModeAbsoluteX): break;
    case(AddressingModeAbsoluteY): break;
    case(AddressingModeAccumulator): break;
    case(AddressingModeImmediate): break;
        break;
    case(AddressingModeImplied): break;
    case(AddressingModeIndexedIndirect): break;
    case(AddressingModeIndirect): break;
    case(AddressingModeIndirectIndexed): break;
    case(AddressingModeRelative): break;
    case(AddressingModeZeroPage): 
        break;
    case(AddressingModeZeroPageX): break;
    case(AddressingModeZeroPageY): break;
    };

    if (m_PreexecutionCallBack != nullptr) {
        std::invoke(m_PreexecutionCallBack, 
            details, 
            info, 
            m_State,
            m_Cycles);
    }

    m_Cycles += details.CycleCount;
    m_State.PC += details.InstructionSize;

    std::invoke(details.Delegate, this, info);
}

void CPUCore6502::Push(uint8_t val) {
    m_Memory.Write(0x100 | m_State.SP, val);
    m_State.SP -= 1;
}

void CPUCore6502::Push16(uint16_t val) {
    uint8_t high = (uint8_t)(val >> 8);
    uint8_t low = (uint8_t)(val & 0xff);

    Push(high);
    Push(low);
}

void CPUCore6502::SetZ(uint8_t val) {
    m_State.Z = val == 0;
}

void CPUCore6502::SetN(uint8_t val) {
    m_State.N = val >> 7;
}

void CPUCore6502::AddBranchCycles(uint16_t oldPC, uint16_t newPC, uint32_t pageCrossCost) {
    uint16_t currentPage = oldPC / 256;
    uint16_t nextPage = newPC / 256;

    m_Cycles += 1;

    if (currentPage != nextPage) {
        m_Cycles += pageCrossCost;
    }
}

void CPUCore6502::Unimplemented(const DynamicExecutionInfo& info) {
    assert(0);
}

void CPUCore6502::BCC(const DynamicExecutionInfo& info) {
    if (m_State.C == 0) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.details.PageCrossCycleCost);
    
        m_State.PC = newPC;
    }
}

void CPUCore6502::BEQ(const DynamicExecutionInfo& info) {
    if (m_State.Z == 1) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.details.PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BCS(const DynamicExecutionInfo& info) {
    if (m_State.C) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.details.PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::CLC(const DynamicExecutionInfo& info) {
    m_State.C = 0;
}

void CPUCore6502::JMP(const DynamicExecutionInfo& info) {
    m_State.PC = info.Address();
}

void CPUCore6502::JSR(const DynamicExecutionInfo& info) {
    uint16_t address = m_State.PC + info.details.InstructionSize - 1;
    Push16(address);
    m_State.PC = info.Address();
}

void CPUCore6502::LDA(const DynamicExecutionInfo& info) {
    if (info.details.AddresingMode == AddressingModeImmediate) {
        m_State.A = info.Immediate();
    }
    else {
        assert(0);
    }
    SetZ(m_State.A);
    SetN(m_State.A);
}
void CPUCore6502::LDX(const DynamicExecutionInfo& info) {
    m_State.X = info.Immediate();
    SetZ(m_State.X);
    SetN(m_State.X);
}

void CPUCore6502::NOP(const DynamicExecutionInfo& info) {
    return;
}
void CPUCore6502::SEC(const DynamicExecutionInfo& info) {
    m_State.C = 1;
}

void CPUCore6502::STX(const DynamicExecutionInfo& info) {
    m_Memory.Write(info.Address(), m_State.X);
}


}