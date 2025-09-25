#pragma once
#include <array>
#include <cstdint>  // for uint8_t, uint16_t
#include <fstream>  // for File handling
#include <string>   // for std::string
#include <stdexcept> // for std::runtime_error
#include <iostream>  // for error messages

class Memory {
private:
    // Game Boy has 64KB of addressable memory (0x0000-0xFFFF)
    std::array<uint8_t, 0x10000> memory;  // 64KB = 0x10000 bytes

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
    }

    // Read a byte from memory
    uint8_t read(uint16_t address) const {
        // Special handling for echo RAM
        if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
            // Echo RAM mirrors Work RAM
            return memory[address - 0x2000];  // 0x2000 is the difference between Echo and Work RAM
        }
        return memory[address];
    }

    // Write a byte to memory
    void write(uint16_t address, uint8_t value) {
        // Special handling for different memory regions
        if (address >= ROM_BANK_0_START && address <= ROM_BANK_N_END) {
            // Can't write to ROM
            return;
        }
        else if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
            // Writing to Echo RAM also writes to Work RAM
            memory[address - 0x2000] = value;  // Write to Work RAM
            memory[address] = value;           // Write to Echo RAM
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

        //2.Read the ROM data into memory starting at 0x0000
        rom_file.read(reinterpret_cast<char*>(memory.data()), 0x4000);
        
        //Check how many bytes were read
        std::streamsize bytes_read = rom_file.gcount();
        if (bytes_read == 0) {
            throw std::runtime_error("Failed to read ROM file");
        }

        //3.Close the file
        rom_file.close(); 
    }
}; // End of Memory class