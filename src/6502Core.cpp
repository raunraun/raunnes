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

    A() = 0;
    X() = 0;
    Y() = 0;

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
            m_Memory,
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

uint8_t CPUCore6502::Pop() {
    m_State.SP += 1;
    uint8_t val = m_Memory.Read(0x100 | m_State.SP);

    return val;
}

uint16_t CPUCore6502::Pop16() {
    uint16_t low = Pop();
    uint16_t high = Pop();

    uint16_t val = high << 8 | low;

    return val;
}

void CPUCore6502::SetC(uint8_t val) {
    m_State.C = val;
}

void CPUCore6502::SetN(uint8_t val) {
    m_State.N = val >> 7;
}

void CPUCore6502::SetZ(uint8_t val) {
    m_State.Z = val == 0;
}

void CPUCore6502::SetZ(bool val) {
    m_State.Z = val;
}

void CPUCore6502::SetV(bool val) {
    m_State.V = val;
}

uint8_t& CPUCore6502::A() {
    return m_State.A;
}

uint8_t& CPUCore6502::X() {
    return m_State.X;
}

uint8_t& CPUCore6502::Y() {
    return m_State.Y;
}

uint16_t& CPUCore6502::PC() {
    return m_State.PC;
}

uint16_t& CPUCore6502::SP() {
    return m_State.SP;
}

uint8_t CPUCore6502::Value(const DynamicExecutionInfo& info) {
    uint8_t val = 0;

    if (info.details.AddresingMode == AddressingModeImmediate) {
        val = info.Immediate();
    }
    else {
        val = m_Memory.Read(info.Address());
    }

    return val;
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
void CPUCore6502::ADC(const DynamicExecutionInfo& info) {
    uint8_t a = A();
    uint8_t b = Value(info);
    uint8_t c = m_State.C;

    A() = a + b + c;

    SetC(A() < a);
    SetN(A());
    SetZ(A());

    // http://www.righto.com/2013/01/a-small-part-of-6502-chip-explained.html
    // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
    // V = not (((A7 NOR B7) and C6) NOR ((A7 NAND B7) NOR C6))
    bool v = (b ^ A()) & (a ^ A()) & 0x80;
    SetV(v);
}

void CPUCore6502::AND(const DynamicExecutionInfo& info) {
    A() &= Value(info);

    SetZ(A());
    SetN(A());
}

void CPUCore6502::BCC(const DynamicExecutionInfo& info) {
    if (m_State.C == 0) {
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

void CPUCore6502::BEQ(const DynamicExecutionInfo& info) {
    if (m_State.Z == 1) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.details.PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BIT(const DynamicExecutionInfo& info) {
    uint8_t m = m_Memory.Read(info.Address());

    m_State.Z = (A() & m) == 0;
    m_State.V = ((m & 0x40) >> 6) == 1;
    m_State.N = (m >> 7) == 1;
}

void CPUCore6502::BMI(const DynamicExecutionInfo& info) {
    if (m_State.N == 1) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.details.PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BNE(const DynamicExecutionInfo& info) {
    if (m_State.Z == 0) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.details.PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BPL(const DynamicExecutionInfo& info) {
    if (m_State.N == 0) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.details.PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BVC(const DynamicExecutionInfo& info) {
    if (m_State.V == 0) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.details.PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BVS(const DynamicExecutionInfo& info) {
    if (m_State.V == 1) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.details.PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::CLC(const DynamicExecutionInfo& info) {
    m_State.C = 0;
}

void CPUCore6502::CLD(const DynamicExecutionInfo& info) {
    m_State.D = 0;
}

void CPUCore6502::CLV(const DynamicExecutionInfo& info) {
    m_State.V = 0;
}

void CPUCore6502::CPX(const DynamicExecutionInfo& info) {
    uint8_t value = Value(info);

    SetZ(X() == value);
    SetC(X() >= value);
    SetN(X() - value);
}

void CPUCore6502::CPY(const DynamicExecutionInfo& info) {
    uint8_t value = Value(info);

    SetZ(Y() == value);
    SetC(Y() >= value);
    SetN(Y() - value);
}

void CPUCore6502::CMP(const DynamicExecutionInfo& info) {
    uint8_t value = Value(info);

    SetZ(A() == value);
    SetC(A() >= value);
    SetN(A() - value);
}

void CPUCore6502::DEY(const DynamicExecutionInfo& info) {
    Y() -= 1;

    SetZ(Y());
    SetN(Y());
}

void CPUCore6502::EOR(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);

    A() ^= val;

    SetZ(A());
    SetN(A());
}

void CPUCore6502::INX(const DynamicExecutionInfo& info) {
    X() += 1;

    SetZ(X());
    SetN(X());
}

void CPUCore6502::INY(const DynamicExecutionInfo& info) {
    Y() += 1;

    SetZ(Y());
    SetN(Y());
}

void CPUCore6502::JMP(const DynamicExecutionInfo& info) {
    m_State.PC = info.Address();
}

void CPUCore6502::JSR(const DynamicExecutionInfo& info) {
    uint16_t address = m_State.PC - 1;
    Push16(address);
    m_State.PC = info.Address();
}

void CPUCore6502::LDA(const DynamicExecutionInfo& info) {
    if (info.details.AddresingMode == AddressingModeImmediate) {
        A() = info.Immediate();
    }
    else {
        assert(0);
    }
    SetZ(A());
    SetN(A());
}
void CPUCore6502::LDX(const DynamicExecutionInfo& info) {
    X() = info.Immediate();
    SetZ(X());
    SetN(X());
}

void CPUCore6502::LDY(const DynamicExecutionInfo& info) {
    Y() = Value(info);
    SetZ(Y());
    SetN(Y());
}

void CPUCore6502::NOP(const DynamicExecutionInfo& info) {
    return;
}

void CPUCore6502::ORA(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);

    A() |= val;

    SetZ(A());
    SetN(A());
}

void CPUCore6502::PHA(const DynamicExecutionInfo& info) {
    Push(A());
}

void CPUCore6502::PHP(const DynamicExecutionInfo& info) {
    // B Flag is always pushed as set
    Push(m_State.P | 0x10);
}

void CPUCore6502::PLA(const DynamicExecutionInfo& info) {
    A() = Pop();

    SetZ(A());
    SetN(A());
}

void CPUCore6502::PLP(const DynamicExecutionInfo& info) {
    // Ignores bits 4, 5
    // http://wiki.nesdev.com/w/index.php/Status_flags#I:_Interrupt_Disable
    // http://visual6502.org/wiki/index.php?title=6502_BRK_and_B_bit
    m_State.P = Pop() & 0xEF | 0x20;
}

void CPUCore6502::RTS(const DynamicExecutionInfo& info) {
    uint16_t newPC = Pop16() + 1;

    m_State.PC = newPC;
}

void CPUCore6502::SBC(const DynamicExecutionInfo& info) {
    uint8_t a = A();
    uint8_t b = ~Value(info);
    uint8_t c = m_State.C;

    A() = a + b + c;

    SetC(A() < a);
    SetN(A());
    SetZ(A());

    // http://www.righto.com/2013/01/a-small-part-of-6502-chip-explained.html
    // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
    // V = not (((A7 NOR B7) and C6) NOR ((A7 NAND B7) NOR C6))
    bool v = (b ^ A()) & (a ^ A()) & 0x80;
    SetV(v);
}

void CPUCore6502::SEC(const DynamicExecutionInfo& info) {
    m_State.C = 1;
}

void CPUCore6502::SEI(const DynamicExecutionInfo& info) {
    m_State.I = 1;
}

void CPUCore6502::SED(const DynamicExecutionInfo& info) {
    m_State.D = 1;
}

void CPUCore6502::STA(const DynamicExecutionInfo& info) {
    m_Memory.Write(info.Address(), A());
}

void CPUCore6502::STX(const DynamicExecutionInfo& info) {
    m_Memory.Write(info.Address(), X());
}


}