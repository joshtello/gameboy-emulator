#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdint>

int main() {
    std::ifstream file("Pokemon - Red Version (USA, Europe) (SGB Enhanced).gb", std::ios::binary);
    if (!file) {
        std::cout << "Failed to open ROM file" << std::endl;
        return 1;
    }
    
    // Read first 32 bytes
    uint8_t buffer[32];
    file.read(reinterpret_cast<char*>(buffer), 32);
    
    std::cout << "First 32 bytes of ROM:" << std::endl;
    for (int i = 0; i < 32; i++) {
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)buffer[i] << " ";
        if ((i + 1) % 16 == 0) std::cout << std::endl;
    }
    
    // Read bytes around 0x4000
    file.seekg(0x4000);
    file.read(reinterpret_cast<char*>(buffer), 16);
    
    std::cout << "\nBytes at 0x4000-0x400F:" << std::endl;
    for (int i = 0; i < 16; i++) {
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)buffer[i] << " ";
    }
    std::cout << std::endl;
    
    // Check cartridge type at 0x0147
    file.seekg(0x0147);
    uint8_t cartType;
    file.read(reinterpret_cast<char*>(&cartType), 1);
    std::cout << "\nCartridge type at 0x0147: 0x" << std::hex << (int)cartType << std::endl;
    
    return 0;
}
