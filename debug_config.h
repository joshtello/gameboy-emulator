#pragma once

// Debug System Configuration
// =========================

// Debug modes:
// 0 = No logging (fast execution, default)
// 1 = Log every instruction (full debug)
// 2 = Log only when PC is within a configurable range
// 3 = Log every Nth instruction (configurable interval)

#define DEBUG_MODE 0  // No logging for normal gameplay
#define DEBUG_PC_START 0x6F0
#define DEBUG_PC_END 0x700
#define DEBUG_INTERVAL 1000

// Enable/disable specific debug features
#define DEBUG_CPU_INSTRUCTIONS 1
#define DEBUG_MEMORY_ACCESS 0
#define DEBUG_FLAGS 0
#define DEBUG_REGISTERS 0

// Performance optimization: when DEBUG_MODE is 0, all debug code is compiled out
#if DEBUG_MODE == 0
#define DEBUG_ENABLED 0
#else
#define DEBUG_ENABLED 1
#endif
