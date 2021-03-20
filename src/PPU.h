#include "MemoryMap.h"
namespace raunnes {
class PPU {
public:
    PPU();
    ~PPU();

private:
    MemoryMap m_Map;
    uint8_t m_PPUCTRL;      // 	$2000 	VPHB SINN 	NMI enable(V), PPU master / slave(P), sprite height(H), background tile select(B), sprite tile select(S), increment mode(I), nametable select(NN)
    uint8_t m_PPUMASK;      // 	$2001 	BGRs bMmG 	color emphasis(BGR), sprite enable(s), background enable(b), sprite left column enable(M), background left column enable(m), greyscale(G)
    uint8_t m_PPUSTATUS;    // 	$2002 	VSO - ----vblank(V), sprite 0 hit(S), sprite overflow(O); read resets write pair for $2005 / $2006
    uint8_t m_OAMADDR;      // 	$2003 	aaaa aaaa 	OAM read / write address
    uint8_t m_OAMDATA;      // 	$2004 	dddd dddd 	OAM data read / write
    uint8_t m_PPUSCROLL;    // 	$2005 	xxxx xxxx 	fine scroll position(two writes : X scroll, Y scroll)
    uint8_t m_PPUADDR;      // 	$2006 	aaaa aaaa 	PPU read / write address(two writes : most significant byte, least significant byte)
    uint8_t m_PPUDATA;      // 	$2007 	dddd dddd 	PPU data read / write
    uint8_t m_OAMDMA;       // 	$4014 	aaaa aaaa 	OAM DMA high address
};

}