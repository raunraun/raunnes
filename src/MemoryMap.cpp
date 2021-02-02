#include "MemoryMap.h"

#include <algorithm>
#include <cassert>

namespace raunnes {
MemoryMap::MemoryMap() {
	m_Bytes.resize(0x10000);
}

MemoryMap::~MemoryMap() {

}

uint16_t MemoryMap::Read16(uint16_t address) {
	if (address+1 < m_Bytes.size()) {
		uint16_t val = ((uint16_t)m_Bytes[address]) | (uint16_t(m_Bytes[address+1]) >> 8);
		return val;
	}
	return 0;
}
	
uint16_t MemoryMap::Read(uint16_t address) {
	if (address < m_Bytes.size()) {
		return m_Bytes[address];
	}
	return 0;
}
	
void MemoryMap::Write(uint16_t address, uint8_t value) {
	if (address < m_Bytes.size()) {
		m_Bytes[address] = value;
	}
}

}