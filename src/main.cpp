#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "6502Core.h"
#include "MemoryMap.h"
#include "PPU.h"

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
        s << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)details.InstructionBytes()[i] << " ";
    }
    for (; i < 3; i++) {
        s << "   ";
    }
    s << "";
    s << std::uppercase << info.Name;
    s << " ";

    switch (info.AddresingMode) {
    case raunnes::CPUCore6502::AddressingModeAbsolute:
        if (std::strncmp(info.Name+1, "JMP", 3) == 0 ||
            std::strncmp(info.Name+1, "JSR", 3) == 0) {
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
        s << "$" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << details.AddressAbsolute() << ",X";
        s << " @ " << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << details.Address();
        s << " = " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)map.Read(details.Address());
        s << std::setw(28 -19) << std::setfill(' ');
        break;
    case raunnes::CPUCore6502::AddressingModeAbsoluteY:
        s << "$" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << details.AddressAbsolute() << ",Y";
        s << " @ " << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << details.Address();
        s << " = " << std::uppercase << std::setw(2) << std::setfill('0') << (uint32_t)map.Read(details.Address());
        s << std::setw(28 - 19) << std::setfill(' ');
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
    case raunnes::CPUCore6502::AddressingModeIndirect:
    {
        uint16_t addr1 = details.AddressIndirect(); 
        
        uint16_t addr_low = addr1;
        uint16_t addr_high = (addr_low & 0xFF00) | ((addr_low + 1) & 0x00FF);
        uint16_t low = map.Read(addr_low);
        uint16_t high = map.Read(addr_high);

        uint16_t addr2 = (high << 8) | low;

        s << "($" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << addr1 << ")";
        s << " = " << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << addr2;
        s << std::setw(28 - 14) << std::setfill(' ');
    }
    break;
    case raunnes::CPUCore6502::AddressingModeIndirectIndexed:
    {
        uint16_t addr1 = details.Immediate();

        uint16_t addr2_low = map.Read(addr1);
        uint16_t addr2_high = map.Read((addr1 + 1) & 0xFF);
        uint16_t addr2 = ((addr2_high << 8) | addr2_low);

        uint16_t addr3 = addr2 + state.Y;

        s << "($" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)details.Immediate() << "),Y";
        s << " = " << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << addr2;
        s << " @ " << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << addr3;
        s << " = " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)map.Read(addr3);
        s << std::setw(28 - 26) << std::setfill(' ');
    }
        break;
    case raunnes::CPUCore6502::AddressingModeRelative:
        s << "$" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (details.Address() + (uint8_t)info.InstructionSize);
        s << std::setw(28 - 5) << std::setfill(' ');
        break;
    case raunnes::CPUCore6502::AddressingModeZeroPageX:
    {
        uint8_t addr1 = (uint32_t)details.Immediate();
        uint8_t addr2 = addr1 + state.X;

        s << "$" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)addr1 << ",X";
        s << " @ " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)addr2;
        s << " = " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)map.Read(addr2);
        s << std::setw(28 - 15) << std::setfill(' ');
    }
        break;
    case raunnes::CPUCore6502::AddressingModeZeroPageY:
    {
        uint8_t addr1 = (uint32_t)details.Immediate();
        uint8_t addr2 = addr1 + state.Y;

        s << "$" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)addr1 << ",Y";
        s << " @ " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)addr2;
        s << " = " << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)map.Read(addr2);
        s << std::setw(28 - 15) << std::setfill(' ');
    }
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

    sf::RenderWindow window(sf::VideoMode(800, 600), "My window");
    sf::RenderWindow ppuDebugger(sf::VideoMode(800, 600), "PPU Debugger");

    sf::Font monoFont;
    if(monoFont.loadFromFile("/home/raun/Code/raunnes/deps/SpaceMono-Regular.ttf") == false) {
        exit(0);
    }

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

        uint8_t chrRom[8192] = { 0 };
        uint32_t chrSize = header.sizeCHR * 8192;
        romFile.read((char*)chrRom, chrSize);

        raunnes::MemoryMap mem(buffer, prgSize, chrRom, chrSize);
        raunnes::CPUCore6502 cpu(mem);
        raunnes::PPU ppu(mem);

        cpu.InstallPreExecutionCallBack(log);

        bool quit = false;

        while(window.isOpen()) {

            sf::Event event;

            while(window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            // NTSC CPU == 1.79MHz
            cpu.Execute();
            ppu.Execute();

            window.clear(sf::Color::Red);
            window.display();

            ppuDebugger.clear(sf::Color::Cyan);
            std::stringstream ss;
            for(int x=0; x<32; x++) {
                ss << std::setw(2) << std::setfill('0') << std::hex << x << " ";
            }
            ss << "\n";

            for(int x=0; x<32*30; x++) {
                if(x % 32 == 0) {
                    ss << "\n";
                }
                ss << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)mem.ReadPPU(0x2000+x);
                ss << ' ';
            }

            sf::Text text;
            text.setFont(monoFont);
            text.setString(ss.str());
            text.setCharacterSize(12);
            text.setFillColor(sf::Color::Black);
            ppuDebugger.draw(text);
            ppuDebugger.display();
        }
    }
    
    return 0;
}
