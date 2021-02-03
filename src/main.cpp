#include <fstream>
#include <iostream>

#include "6502Core.h"
#include "MemoryMap.h"

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

        cpu.Execute();
        cpu.Execute();
        cpu.Execute();
        cpu.Execute();
        cpu.Execute();
        cpu.Execute();


    }
    


    return 0;
}
