#include <fstream>
#include <iostream>
#include <iomanip>

#include "6502Core.h"
#include "MemoryMap.h"

void log(const raunnes::CPUCore6502::InstructionDetails& info,
    const raunnes::CPUCore6502::DynamicExecutionInfo& details,
    const raunnes::CPUCore6502::CPUCore6502State& state,
    const uint64_t cycles) {

    std::cout << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << state.PC;
    std::cout << "  ";
    
    unsigned i = 0;
    for (; i < info.InstructionSize; i++) {
        std::cout << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)details.InstructionBytes[i] << " ";
    }
    for (; i < 3; i++) {
        std::cout << "   ";
    }
    std::cout << " ";
    std::cout << std::uppercase << info.Name;
    std::cout << " ";

    switch (info.AddresingMode) {
    case raunnes::CPUCore6502::AddressingModeAbsolute:
        std::cout << "$" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << details.Address();
        std::cout <<std::setw(28 - 5) << std::setfill(' ');
        break;
    case raunnes::CPUCore6502::AddressingModeImmediate:
        std::cout << "#$" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)details.Immediate();
        std::cout << std::setw(28 - 4) << std::setfill(' ');
        break;
    default:
        std::cout << "!!!!";
        std::cout << std::setw(28 - 4) << std::setfill(' ');
    }
    std::cout << "  ";
    std::cout << "A:" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)state.A;
    std::cout << " ";
    std::cout << "X:" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)state.X;
    std::cout << " ";
    std::cout << "Y:" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)state.Y;
    std::cout << " ";
    std::cout << "P:" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)state.S;
    std::cout << " ";
    std::cout << "SP:" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)state.SP;
    std::cout << " ";
    std::cout << "PPU:" << std::uppercase << std::setw(7) << std::setfill(' ');
    std::cout << " ";
    std::cout << "CYC:" << cycles;

    std::cout << std::endl;
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

        cpu.Execute();
        cpu.Execute();
        cpu.Execute();
        cpu.Execute();
        cpu.Execute();
        cpu.Execute();


    }
    


    return 0;
}
