#include <cstdint>

#include <string>

namespace raunnes {

class Logger {

public:
	Logger();
	~Logger();

    void LogCurrentState(
        uint16_t Address,
        uint8_t Byte0,
        uint8_t Byte1,
        uint8_t Byte2,
        const std::string& OpCodeDsiassemble,
        uint8_t A,
        uint8_t X,
        uint8_t Y,
        uint8_t P,
        uint8_t SP,
        uint16_t PPU,
        uint64_t cycle);

private:

};

}