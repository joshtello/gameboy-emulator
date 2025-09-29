#pragma once
#include <array>
#include <vector>
#include <cstdint>  // for uint8_t, uint16_t
#include <fstream>  // for File handling
#include <string>   // for std::string
#include <stdexcept> // for std::runtime_error
#include <iostream>  // for error messages

class Memory {
private:
    // Game Boy has 64KB of addressable memory (0x0000-0xFFFF)
    std::array<uint8_t, 0x10000> memory;  // 64KB = 0x10000 bytes
    
    // MBC1 Bank Switching Support
    std::vector<uint8_t> romData;  // Full ROM data
    uint8_t currentRomBank = 1;    // Current ROM bank (1-31)
    uint8_t currentRamBank = 0;    // Current RAM bank (0-3)
    bool ramEnabled = false;       // RAM enable flag
    bool bankingMode = false;      // Banking mode (false=ROM, true=RAM)
    
    // Debug counter for memory accesses
    mutable int debugAccessCount = 0;

    // Memory map regions
    static constexpr uint16_t ROM_BANK_0_START    = 0x0000;  // ROM Bank 0 (16KB)
    static constexpr uint16_t ROM_BANK_0_END      = 0x3FFF;
    static constexpr uint16_t ROM_BANK_N_START    = 0x4000;  // ROM Bank N (16KB)
    static constexpr uint16_t ROM_BANK_N_END      = 0x7FFF;
    static constexpr uint16_t VRAM_START          = 0x8000;  // Video RAM (8KB)
    static constexpr uint16_t VRAM_END            = 0x9FFF;
    static constexpr uint16_t EXT_RAM_START       = 0xA000;  // External RAM (8KB)
    static constexpr uint16_t EXT_RAM_END         = 0xBFFF;
    static constexpr uint16_t WORK_RAM_START      = 0xC000;  // Work RAM (8KB)
    static constexpr uint16_t WORK_RAM_END        = 0xDFFF;
    static constexpr uint16_t ECHO_RAM_START      = 0xE000;  // Echo RAM (7.5KB)
    static constexpr uint16_t ECHO_RAM_END        = 0xFDFF;
    static constexpr uint16_t OAM_START           = 0xFE00;  // Sprite Attribute Table (160B)
    static constexpr uint16_t OAM_END             = 0xFE9F;
    static constexpr uint16_t IO_REG_START        = 0xFF00;  // I/O Registers (128B)
    static constexpr uint16_t IO_REG_END          = 0xFF7F;
    static constexpr uint16_t HRAM_START          = 0xFF80;  // High RAM (127B)
    static constexpr uint16_t HRAM_END            = 0xFFFE;
    static constexpr uint16_t IE_REGISTER         = 0xFFFF;  // Interrupt Enable Register

public:
    Memory() {
        // Initialize all memory to 0
        memory.fill(0);
        
        // Set post-BIOS defaults for I/O registers
        memory[0xFF40] = 0x91;  // LCDC - Display enabled, tile map 0x9800, tile data 0x8000
        memory[0xFF47] = 0xFC;  // BGP - Background palette
        memory[0xFF42] = 0x00;  // SCY - Scroll Y
        memory[0xFF43] = 0x00;  // SCX - Scroll X
        memory[0xFF44] = 0x00;  // LY - LCD Y coordinate
        memory[0xFF45] = 0x00;  // LYC - LY compare
        memory[0xFF46] = 0xFF;  // DMA - DMA transfer
        memory[0xFF48] = 0xFF;  // OBP0 - Object palette 0
        memory[0xFF49] = 0xFF;  // OBP1 - Object palette 1
        memory[0xFF4A] = 0x00;  // WY - Window Y
        memory[0xFF4B] = 0x00;  // WX - Window X
    }

    // Read a byte from memory
    uint8_t read(uint16_t address) const {
        // Debug logging for specific memory ranges (limit to first 200 accesses)
        if (debugAccessCount < 200 && ((address >= 0x0000 && address <= 0x7FFF) || 
            (address >= 0xFF00 && address <= 0xFF7F) ||
            (address >= 0xFF40 && address <= 0xFF4B))) {
            std::cout << "READ 0x" << std::hex << address << " = 0x" << static_cast<int>(memory[address]) << std::endl;
            debugAccessCount++;
        }
        
        // MBC1 ROM Bank Switching
        if (address >= ROM_BANK_0_START && address <= ROM_BANK_0_END) {
            // ROM Bank 0 (always accessible)
            if (address < romData.size()) {
                return romData[address];
            }
            return 0xFF;  // Return 0xFF for unmapped ROM
        }
        else if (address >= ROM_BANK_N_START && address <= ROM_BANK_N_END) {
            // ROM Bank N (switchable)
            uint32_t romOffset = (currentRomBank * 0x4000) + (address - ROM_BANK_N_START);
            if (romOffset < romData.size()) {
                return romData[romOffset];
            }
            return 0xFF;  // Return 0xFF for unmapped ROM
        }
        
        // Special handling for echo RAM
        if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
            // Echo RAM mirrors Work RAM
            return memory[address - 0x2000];  // 0x2000 is the difference between Echo and Work RAM
        }
        return memory[address];
    }

    // Write a byte to memory
    void write(uint16_t address, uint8_t value) {
        // Debug logging for specific memory ranges (limit to first 200 accesses)
        if (debugAccessCount < 200 && ((address >= 0x0000 && address <= 0x7FFF) || 
            (address >= 0xFF00 && address <= 0xFF7F) ||
            (address >= 0xFF40 && address <= 0xFF4B))) {
            std::cout << "WRITE 0x" << std::hex << address << " = 0x" << static_cast<int>(value) << std::endl;
            debugAccessCount++;
        }
        
        // MBC1 Register Handling
        if (address >= 0x0000 && address <= 0x1FFF) {
            // RAM Enable Register (0x0000-0x1FFF)
            ramEnabled = (value & 0x0A) == 0x0A;
            std::cout << "MBC1 RAM Enable: " << (ramEnabled ? "ON" : "OFF") << std::endl;
        }
        else if (address >= 0x2000 && address <= 0x3FFF) {
            // ROM Bank Number Register (0x2000-0x3FFF)
            uint8_t bankNumber = value & 0x1F;  // Lower 5 bits
            if (bankNumber == 0) bankNumber = 1;  // Bank 0 is not allowed, use bank 1
            currentRomBank = bankNumber;
            std::cout << "MBC1 ROM Bank: " << static_cast<int>(currentRomBank) << std::endl;
        }
        else if (address >= 0x4000 && address <= 0x5FFF) {
            // RAM Bank Number / Upper ROM Bank Register (0x4000-0x5FFF)
            if (bankingMode) {
                currentRamBank = value & 0x03;  // Lower 2 bits for RAM bank
                std::cout << "MBC1 RAM Bank: " << static_cast<int>(currentRamBank) << std::endl;
            } else {
                // Upper ROM bank bits
                currentRomBank = (currentRomBank & 0x1F) | ((value & 0x03) << 5);
                std::cout << "MBC1 Upper ROM Bank: " << static_cast<int>(currentRomBank) << std::endl;
            }
        }
        else if (address >= 0x6000 && address <= 0x7FFF) {
            // Banking Mode Select Register (0x6000-0x7FFF)
            bankingMode = (value & 0x01) != 0;
            std::cout << "MBC1 Banking Mode: " << (bankingMode ? "RAM" : "ROM") << std::endl;
        }
        else if (address >= ROM_BANK_0_START && address <= ROM_BANK_N_END) {
            // Can't write to ROM
            return;
        }
        else if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
            // Writing to Echo RAM also writes to Work RAM
            memory[address - 0x2000] = value;  // Write to Work RAM
            memory[address] = value;           // Write to Echo RAM
        }
        else if (address == 0xFF40) {
            // LCDC register - enable display and set tile map
            memory[address] = value;
            std::cout << "LCDC register set to: 0x" << std::hex << static_cast<int>(value) << std::endl;
        }
        else if (address == 0xFF47) {
            // BGP register - background palette
            memory[address] = value;
            std::cout << "BGP register set to: 0x" << std::hex << static_cast<int>(value) << std::endl;
        }
        else {
            memory[address] = value;
        }
    }

    // Read a 16-bit word from memory (little-endian)
    uint16_t read_word(uint16_t address) const {
        return static_cast<uint16_t>(read(address)) | 
               (static_cast<uint16_t>(read(address + 1)) << 8);
    }

    // Write a 16-bit word to memory (little-endian)
    void write_word(uint16_t address, uint16_t value) {
        write(address, value & 0xFF);         // Low byte
        write(address + 1, value >> 8);       // High byte
    }

    void loadRom(const std::string& filename) {
        //1. Open the file
        std::ifstream rom_file(filename, std::ios::binary);
        if(!rom_file.is_open()){
            throw std::runtime_error("Could not open ROM file");
        }

        //2. Get file size
        rom_file.seekg(0, std::ios::end);
        std::streamsize file_size = rom_file.tellg();
        rom_file.seekg(0, std::ios::beg);
        
        std::cout << "ROM file size: " << file_size << " bytes" << std::endl;

        //3. Read the entire ROM into romData vector
        romData.resize(file_size);
        rom_file.read(reinterpret_cast<char*>(romData.data()), file_size);
        
        //4. Check how many bytes were read
        std::streamsize bytes_read = rom_file.gcount();
        if (bytes_read == 0) {
            throw std::runtime_error("Failed to read ROM file");
        }
        
        std::cout << "ROM loaded: " << bytes_read << " bytes" << std::endl;
        std::cout << "ROM banks available: " << (file_size / 0x4000) << std::endl;

        //5. Copy first 16KB to memory for ROM Bank 0
        if (file_size >= 0x4000) {
            std::copy(romData.begin(), romData.begin() + 0x4000, memory.begin());
            std::cout << "ROM Bank 0 (0x0000-0x3FFF) loaded" << std::endl;
        }

        //6. Close the file
        rom_file.close(); 
    }
}; // End of Memory class