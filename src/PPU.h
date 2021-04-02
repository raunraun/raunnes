#include "MemoryMap.h"

namespace raunnes {
class PPU {
public:
    PPU(MemoryMap& memory);
    ~PPU();
    
    uint8_t I() const;

    void WriteRegister(uint16_t address, uint8_t value);
    uint8_t ReadRegister(uint16_t address);


private:
    MemoryMap& m_Map;
    uint8_t m_Control;      // 	$2000 	VPHB SINN 	NMI enable(V), PPU master / slave(P), sprite height(H), background tile select(B), sprite tile select(S), increment mode(I), nametable select(NN)
    uint8_t m_Mask;         // 	$2001 	BGRs bMmG 	color emphasis(BGR), sprite enable(s), background enable(b), sprite left column enable(M), background left column enable(m), greyscale(G)
    uint8_t m_Status;       // 	$2002 	VSO - ----vblank(V), sprite 0 hit(S), sprite overflow(O); read resets write pair for $2005 / $2006
    uint8_t m_OAMAddr;      // 	$2003 	aaaa aaaa 	OAM read / write address
    uint8_t m_OAMData;      // 	$2004 	dddd dddd 	OAM data read / write
    uint8_t m_Scroll;       // 	$2005 	xxxx xxxx 	fine scroll position(two writes : X scroll, Y scroll)
    
    uint8_t m_Addr;         // 	$2006 	aaaa aaaa 	PPU read / write address(two writes : most significant byte, least significant byte)
    bool    m_AddrHighEnable;

    uint8_t m_Data;         // 	$2007 	dddd dddd 	PPU data read / write
    uint8_t m_OMADMA;       // 	$4014 	aaaa aaaa 	OAM DMA high address

    uint8_t m_Pallette[32];
    uint8_t m_VRAM[2048];
    uint8_t m_OAMRAM[256];
};

}