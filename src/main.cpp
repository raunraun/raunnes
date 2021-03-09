#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "6502Core.h"
#include "MemoryMap.h"

void log(const raunnes::CPUCore6502::InstructionDetails& info,
    const raunnes::CPUCore6502::DynamicExecutionInfo& details,
    const raunnes::CPUCore6502::CPUCore6502State& state,
    const raunnes::MemoryMap& map,
    const uint64_t cycles) {

    std::stringstream s;

    s << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << state.PC;
    s << "  ";
    
    unsigned i = 0;
    for (; i < info.InstructionSize; i++) {
        s << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)details.InstructionBytes[i] << " ";
    }
    for (; i < 3; i++) {
        s << "   ";
    }
    s << " ";
    s << std::uppercase << info.Name;
    s << " ";

    switch (info.AddresingMode) {
    case raunnes::CPUCore6502::AddressingModeAbsolute:
        if (std::strncmp(info.Name, "JMP", 3) == 0 ||
            std::strncmp(info.Name, "JSR", 3) == 0) {
            s << "$" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << details.Address();
            s << std::setw(28 - 5) << std::setfill(' ');
        }
        else {
            s << "$" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << details.Address();
            s << " = ";
            s << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)map.Read(details.Address());
            s << std::setw(28 - 10) << std::setfill(' ');
        }
        break;
    case raunnes::CPUCore6502::AddressingModeAbsoluteX:
        s << "$" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << details.Address();
        s << " = ";
        s << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)map.Read(details.Address());
        s << std::setw(28 - 8) << std::setfill(' ');
        break;
    case raunnes::CPUCore6502::AddressingModeAccumulator:
        s << "A";
        s << std::setw(28 - 1) << std::setfill(' ');
        break;
    case raunnes::CPUCore6502::AddressingModeImmediate:
        s << "#$" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)details.Immediate();
        s << std::setw(28 - 4) << std::setfill(' ');
        break;
    case raunnes::CPUCore6502::AddressingModeZeroPage:
        s << "$" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << details.Address();
        s << " = ";
        s << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)map.Read(details.Address());
        s << std::setw(28 - 8) << std::setfill(' ');
        break;
    case raunnes::CPUCore6502::AddressingModeImplied:
        s << std::setw(28) << std::setfill(' ');
        break;
    case raunnes::CPUCore6502::AddressingModeIndexedIndirect:
    {
        uint16_t addr1 = (state.X + details.Immediate()) & 0xFF;

        uint16_t addr2_low = map.Read(addr1);
        uint16_t addr2_high = map.Read((addr1 + 1) & 0xFF);
        uint16_t addr2 = (addr2_high << 8) | addr2_low;

        s << "($" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)details.Immediate() << ",X)";
        s << " @ " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << addr1;
        s << " = " << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << addr2;
        s << " = " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)map.Read(addr2);
        s << std::setw(28 - 24) << std::setfill(' ');
    }
        break;
    case raunnes::CPUCore6502::AddressingModeRelative:
        s << "$" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (state.PC + details.Immediate() + (uint8_t)info.InstructionSize);
        s << std::setw(28 - 5) << std::setfill(' ');
        break;
    default:
        s << "!!!!";
        s << std::setw(28 - 4) << std::setfill(' ');
    }
    s << "  ";
    s << "A:" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)state.A;
    s << " ";
    s << "X:" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)state.X;
    s << " ";
    s << "Y:" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)state.Y;
    s << " ";
    s << "P:" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)state.P;
    s << " ";
    s << "SP:" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)state.SP;
    s << " ";
    s << "PPU:" << std::uppercase << std::setw(7) << std::setfill(' ');
    s << " ";
    s << "CYC:" << std::dec << cycles;

    s << std::endl;

    std::cout << s.str();

    static std::fstream f("raunnes.log", std::ios::out);

    if (f.good()) {
        f << s.str();
    }
}

int main(int argc, char** argv) {
    
    std::fstream romFile("../tests/nestest/nestest.nes", std::ios::in | std::ios::binary);

    struct _NESHeader{
        uint32_t magic;
        uint8_t  sizePRG;
        uint8_t  sizeCHR;
        uint8_t  F1;
        uint8_t  F2;
        uint8_t  F3;
        uint8_t  F4;
        uint8_t  F5;
        uint32_t unused;
    };

    if (romFile.good()) {
        std::cout << "WOO\n";

        _NESHeader header;

        romFile.read((char*)&header, sizeof(header));

        uint8_t buffer[16384 * 2] = { 0 };
        uint32_t prgSize = header.sizePRG * 16384;
        romFile.read((char*)buffer, prgSize);

        raunnes::MemoryMap mem(buffer, prgSize);
        raunnes::CPUCore6502 cpu(mem);
        cpu.InstallPreExecutionCallBack(log);

        for (int c = 0; c < 3000; c++) {
            cpu.Execute();
        }
    }
    


    return 0;
}
