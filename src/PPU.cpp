#include "PPU.h"

#include <cassert>

namespace raunnes {

PPU::PPU(MemoryMap& memory) :
    m_Map(memory),
    m_Cycle(0),
    m_Control(0x0),
    m_Mask(0x0),
    m_Status(0x0),
    m_OAMAddr(0x0),
    m_OAMData(0x0),
    m_Scroll(0x0),
    m_Addr(0x0),
    m_AddrHighEnable(true),
    m_Data(0x0),
    m_OMADMA(0x0) {
}

PPU::~PPU() {
}

uint8_t PPU::I() const {
    return (m_Control << 5) >> 7;
}

void PPU::WriteRegister(uint16_t address, uint8_t value) {
    switch (address) {
    case 0x2000:
        m_Control = value;
        break;
    case 0x2001:
        m_Mask = value;
        break;
    case 0x2003:
        if (m_OAMAddrHighEnable) {
            m_OAMAddr = 0;
            m_OAMAddr = value;
            m_OAMAddr <<= 8;
        }
        else {
            m_OAMAddr |= value;
        }
        m_OAMAddrHighEnable = !m_OAMAddrHighEnable;
        break;
    case 0x2005:
        m_Scroll = value;
        break;
    case 0x2006:
        if (m_AddrHighEnable) {
            m_Addr = 0;
            m_Addr = value;
            m_Addr <<= 8;
        }
        else {
            m_Addr |= value;
        }
        m_AddrHighEnable = !m_AddrHighEnable;
        break;
    }
}

uint8_t PPU::ReadRegister(uint16_t address) {
    switch (address) {
    case 0x2000:
        return m_Control;
        break;
    case 0x2002:
        return m_Status;
        break;
    case 0x2007:
    {
        uint8_t ret = m_Data;
        m_Data = Read(m_Addr);

        if (I() == 0) {
            m_Addr += 1;
        }
        else {
            m_Addr += 32;
        }

        return m_Data;

        break;
    }
    default:
        assert(0 && "Unimplemented PPU Register Read");
    }
    return 0;
}

uint8_t PPU::Read(uint16_t address) {
    if (address <= 0x1fff) {
        //if (address < m_ChrRom.size()) {
        {
            uint8_t buffered = m_Data;
            m_Data = m_Map.ReadPPU(address);
            return  buffered;
        }
        assert(0 && "bad chr rom address");
    }
    else if (address <= 0x2fff) {
        uint8_t buffered = m_Data;
        m_Data = m_VRAM[address - 0x2000];
        return  buffered;
    }
    else if (address <= 0x3eff) {
        assert(0 && "todo");
    }
    else if (address <= 0x3fff) {
        return m_Pallette[address - 0x3f00];

    }

    assert(0 && "bad PPU address");
    return 0;
}

void PPU::Execute() {
    struct ExecutionState {
        uint32_t n;
        uint32_t m;    
        uint32_t secondaryOAMIndex;
        uint32_t scanLine;
        uint32_t nameTableAddress;
        uint32_t atttibTableAddress;
        uint32_t chrAddress;
    };

    static ExecutionState execState = { 0 };
    execState.nameTableAddress = 0x2000;
    execState.atttibTableAddress = 0x23c0;
    execState.chrAddress = 0;

    // Lets do 1 full scanline per call for now
    // 262 scanlines per frame
    // 341 PPU clocks per scanline.  1 CPU clock == 3 PPU clocks
    //
    // https://www.nesdev.org/wiki/PPU_rendering
    //

    // OAM == sprite
    //  4 bytes == [x, y, color, tile] for each of the 64 sprites
    //

    // nametable == background == [tile, x, y]
    //   8x8 pixels per file
    //   256x240 pixels == 32x30 tiles
    //

    // palette == 32 bytes
    //  4 background palettes
    //  4 sprite palettes
    //  3 colors each + transparency, PPU can drawn 25 unique colors


    uint8_t nameTableIndex = 0;
    uint8_t attributeTableIndex = 0;
    uint8_t bgLow = 0;
    uint8_t bgHigh = 0;

    for(m_Cycle=0; m_Cycle<341; m_Cycle++) {
        // There are 3 cycles of the PPU for each CPU cycle

        if(m_Cycle % 1 == 0) {
            uint32_t c = m_Cycle % 8;

            switch(c) {
            case 0:
                break;
            case 1:
                nameTableIndex = Read(execState.nameTableAddress);
                execState.nameTableAddress += 1;
                break;
            case 2:
                // 2nd cycle of nametable address read
                break;
            case 3:
                attributeTableIndex = Read(execState.atttibTableAddress);
                execState.atttibTableAddress += 1;
                break;
            case 4:
                // 2nd cycle of attribute table fetch;
                break;
            case 5:
                execState.chrAddress = nameTableIndex;
                bgLow = m_Map.ReadPPU(execState.chrAddress);
                execState.chrAddress += 1;
                break;
            case 6:
                // 2nd cycle of bgLow read
                break;
            case 7:
                bgHigh = m_Map.ReadPPU(execState.chrAddress);
                execState.chrAddress += 1;
                break;
            case 8:


                // TODO: inc horiv
                break;
            }
        }
    }

/*
        } else if((m_Cycle >= 1) && (m_Cycle <= 64) {
            // Initialize secondary OAM to 0xff - 32 bytes over 64 clocks
            uint32_t index = ((m_Cycle-1) / 2);
            m_SecondaryOAMRAM[index] = 0xff;
        
        } else if((m_Cycle >= 65) && (m_Cycle <= 300)) {
            uint16_t nametableAddr = m_Cycle - 65;

            // Nametable
            uint32_t entry = Read(0x2000 + nametableAddr);


            // On odd cycles data is read from OAM
            // On even cycles data is secondary OAM

            // Copy Y coordinate into secondary OAM
            m_SecondaryOAMRAM[execState.secondaryOAMIndex] = m_OAMRAM[execState.n * 64 + 0];

            if( true Y coordinate is in range...) {
                // Copy remaining attributes
                m_SecondaryOAMRAM[execState.secondaryOAMIndex] = m_OAMRAM[execState.n * 64 + 1];
            }

            execState.n += 1;
        }
    }
*/
}

}