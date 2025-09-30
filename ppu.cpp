#include "ppu.h"
#include <iostream>

// Constructor
PPU::PPU(Memory& mem) : memory(mem) {
    window = nullptr;
    renderer = nullptr;
    framebuffer = nullptr;
}

// Destructor
PPU::~PPU() {
    cleanup();
}

// Initialize SDL2 and create window
void PPU::init() {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Create window (scaled up for visibility)
    window = SDL_CreateWindow("Game Boy Emulator",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               160 * SCALE_FACTOR, 144 * SCALE_FACTOR,  // Scaled Game Boy screen
                               SDL_WINDOW_SHOWN);
    
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Allocate framebuffer (160x144 pixels)
    framebuffer = new uint32_t[160 * 144];
    
    std::cout << "PPU initialized successfully!" << std::endl;
    std::cout << "SDL Window created: " << (window ? "YES" : "NO") << std::endl;
    std::cout << "SDL Renderer created: " << (renderer ? "YES" : "NO") << std::endl;
}

// Render the framebuffer to screen
void PPU::render() {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Get LCD Control Register
    uint8_t lcdc = memory.read(0xFF40);
    
    // DEBUG: Print render call info
    static int render_count = 0;
    render_count++;
    if (render_count <= 20) {
        std::cout << "PPU::render() #" << render_count << " LCDC=0x" << std::hex << (int)lcdc 
                  << " LCD_ENABLED=" << ((lcdc & 0x80) ? "YES" : "NO")
                  << " BG_ENABLED=" << ((lcdc & 0x01) ? "YES" : "NO") << std::endl;
    }
    
    // Check if LCD is enabled (bit 7 of LCDC)
    if (!(lcdc & 0x80)) {
        // LCD is disabled, show blank screen
        if (render_count <= 20) {
            std::cout << "LCD DISABLED - showing blank screen" << std::endl;
        }
        SDL_RenderPresent(renderer);
        return;
    }
    
    // Check if background is enabled (bit 0 of LCDC)
    if (!(lcdc & 0x01)) {
        if (render_count <= 20) {
            std::cout << "BACKGROUND DISABLED - showing blank screen" << std::endl;
        }
        SDL_RenderPresent(renderer);
        return;
    }
    
    // Get background palette
    uint8_t bgp = memory.read(0xFF47);
    
    // Choose tile map base: 0x9800 or 0x9C00 (LCDC bit 3)
    uint16_t tileMapBase = (lcdc & 0x08) ? 0x9C00 : 0x9800;
    
    // Choose tile data region based on LCDC bit 4
    uint16_t tileDataBase;
    bool unsignedTiles;
    if (lcdc & 0x10) {
        // LCDC bit 4 = 1: unsigned tile IDs, base 0x8000
        tileDataBase = 0x8000;
        unsignedTiles = true;
    } else {
        // LCDC bit 4 = 0: signed tile IDs, base 0x8800 (tile 0 at 0x9000)
        tileDataBase = 0x8800;
        unsignedTiles = false;
    }
    
    // Get scroll registers
    uint8_t scrollX = memory.read(0xFF43);
    uint8_t scrollY = memory.read(0xFF42);
    
    // DEBUG: Check if we have any tile data
    if (render_count <= 5) {
        std::cout << "PPU: tileMapBase=0x" << std::hex << tileMapBase 
                  << " tileDataBase=0x" << tileDataBase 
                  << " scrollX=" << (int)scrollX << " scrollY=" << (int)scrollY << std::endl;
        
        // Check first few tiles
        for (int i = 0; i < 4; i++) {
            uint8_t tileNum = memory.read(tileMapBase + i);
            std::cout << "Tile " << i << " = 0x" << std::hex << (int)tileNum << std::endl;
        }
    }
    
    // Test pattern removed - now rendering actual game graphics
    
    // Render each pixel on the screen (160x144)
    for (int screenY = 0; screenY < 144; screenY++) {
        for (int screenX = 0; screenX < 160; screenX++) {
            // Calculate background coordinates with scroll
            int bgY = (screenY + scrollY) & 0xFF;  // Wrap at 256 pixels
            int bgX = (screenX + scrollX) & 0xFF;  // Wrap at 256 pixels
            
            // Calculate tile coordinates (32x32 tile map)
            int tileY = bgY / 8;
            int tileX = bgX / 8;
            
            // Wrap tile coordinates for 32x32 tile map
            tileY &= 31;
            tileX &= 31;
            
            // Get tile number from tile map
            uint16_t mapAddr = tileMapBase + (tileY * 32) + tileX;
            uint8_t tileNum = memory.read(mapAddr);
            
            // Calculate tile data address
            uint16_t tileAddr;
            if (unsignedTiles) {
                // Unsigned tile numbers (0-255)
                tileAddr = tileDataBase + (tileNum * 16);
            } else {
                // Signed tile numbers (-128 to 127)
                // Tile 0 is at 0x8800, tile 128 is at 0x9000
                int8_t signedTileNum = static_cast<int8_t>(tileNum);
                tileAddr = tileDataBase + ((signedTileNum + 128) * 16);
            }
            
            // Calculate pixel position within tile
            int pixelY = bgY % 8;
            int pixelX = bgX % 8;
            
            // Read tile data (2 bytes per row)
            uint8_t lowByte = memory.read(tileAddr + (pixelY * 2));
            uint8_t highByte = memory.read(tileAddr + (pixelY * 2) + 1);
            
            // Extract color index from bitplanes
            uint8_t colorIndex = 0;
            if (lowByte & (0x80 >> pixelX)) colorIndex |= 1;   // Bit 0
            if (highByte & (0x80 >> pixelX)) colorIndex |= 2;  // Bit 1
            
            
            // Get color from BGP register
            SDL_Color color = getColorFromBGP(colorIndex, bgp);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            
            // Draw pixel (scaled up)
            SDL_Rect pixelRect = {screenX * SCALE_FACTOR, screenY * SCALE_FACTOR, 
                                  SCALE_FACTOR, SCALE_FACTOR};
            SDL_RenderFillRect(renderer, &pixelRect);
        }
    }
    
    // TODO: Add sprite rendering here when implementing OAM
    
    // Present the rendered frame
    SDL_RenderPresent(renderer);
    
    // Debug: Print some VRAM info occasionally
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) {  // Every 60 frames (1 second at 60fps)
        std::cout << "Frame " << frameCount << " - LCDC: 0x" << std::hex << (int)lcdc 
                  << " BGP: 0x" << (int)bgp << " Scroll: (" << (int)scrollX << "," << (int)scrollY << ")" << std::endl;
        std::cout << "TileMap: 0x" << std::hex << tileMapBase 
                  << " TileData: 0x" << tileDataBase 
                  << " " << (unsignedTiles ? "Unsigned" : "Signed") << " tiles" << std::endl;
        
        // Debug VRAM content
        std::cout << "VRAM 0x8000-0x800F: ";
        for (int i = 0; i < 16; i++) {
            std::cout << std::hex << (int)memory.read(0x8000 + i) << " ";
        }
        std::cout << " | TileMap 0x9800-0x980F: ";
        for (int i = 0; i < 16; i++) {
            std::cout << std::hex << (int)memory.read(0x9800 + i) << " ";
        }
        std::cout << std::endl;
    }
}

// Clean up resources
void PPU::cleanup() {
    if (framebuffer) {
        delete[] framebuffer;
        framebuffer = nullptr;
    }
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    SDL_Quit();
}

// Convert CPU memory to pixels (placeholder for now)
void PPU::updateFramebuffer() {
    // This will be implemented later when we connect to CPU memory
    // For now, just a placeholder
}

// Render a single 8x8 tile
void PPU::renderTile(uint8_t* tileData, int screenX, int screenY) {
    // Get background palette for proper color mapping
    uint8_t bgp = memory.read(0xFF47);
    
    // Process each row of the tile
    for (int row = 0; row < 8; row++) {
        uint8_t lowByte = tileData[row * 2];      // Low bits
        uint8_t highByte = tileData[row * 2 + 1]; // High bits
        
        // Process each pixel in the row
        for (int pixel = 0; pixel < 8; pixel++) {
            // Extract 2-bit color value
            uint8_t colorIndex = 0;
            if (lowByte & (0x80 >> pixel)) {
                colorIndex |= 1;  // Set bit 0
            }
            if (highByte & (0x80 >> pixel)) {
                colorIndex |= 2;  // Set bit 1
            }
            
            // Draw the pixel (scaled up)
            int x = (screenX + pixel) * SCALE_FACTOR;
            int y = (screenY + row) * SCALE_FACTOR;
            
            // Only draw if within screen bounds
            if (x >= 0 && x < 160 * SCALE_FACTOR && y >= 0 && y < 144 * SCALE_FACTOR) {
                SDL_Color color = getColorFromBGP(colorIndex, bgp);
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                
                // Draw a scaled block for each pixel
                SDL_Rect pixelRect = {x, y, SCALE_FACTOR, SCALE_FACTOR};
                SDL_RenderFillRect(renderer, &pixelRect);
            }
        }
    }
}

// Decode Game Boy VRAM tile into pixel array
std::array<std::array<uint8_t, 8>, 8> PPU::decodeTile(const uint8_t* tileData) {
    std::array<std::array<uint8_t, 8>, 8> pixels;
    
    // Each tile is 16 bytes: 8 rows × 2 bytes per row
    for (int row = 0; row < 8; row++) {
        uint8_t lowByte = tileData[row * 2];      // Low bitplane
        uint8_t highByte = tileData[row * 2 + 1]; // High bitplane
        
        // Each row has 8 pixels
        for (int pixel = 0; pixel < 8; pixel++) {
            // Extract 2-bit color index by combining bits from both bitplanes
            uint8_t colorIndex = 0;
            
            // Check bit in low byte (bit 0)
            if (lowByte & (0x80 >> pixel)) {
                colorIndex |= 1;
            }
            
            // Check bit in high byte (bit 1)
            if (highByte & (0x80 >> pixel)) {
                colorIndex |= 2;
            }
            
            // Store the pixel value (0-3)
            pixels[row][pixel] = colorIndex;
        }
    }
    
    return pixels;
}

// Map 2-bit color index to SDL color using BGP register
SDL_Color PPU::getColorFromBGP(uint8_t colorIndex, uint8_t bgpRegister) {
    // Extract the 2-bit color value for the given color index
    uint8_t colorValue = (bgpRegister >> (colorIndex * 2)) & 0x03;
    
    // Game Boy color mapping (4 shades of gray)
    // 0 = White, 1 = Light gray, 2 = Dark gray, 3 = Black
    SDL_Color color;
    switch (colorValue) {
        case 0: // White
            color = {255, 255, 255, 255};
            break;
        case 1: // Light gray
            color = {192, 192, 192, 255};
            break;
        case 2: // Dark gray
            color = {96, 96, 96, 255};
            break;
        case 3: // Black
            color = {0, 0, 0, 255};
            break;
        default:
            color = {0, 0, 0, 255}; // Fallback to black
            break;
    }
    
    return color;
}

// PPU step function - handles LCD timing
void PPU::step(int cycles) {
    // Get LCD Control Register
    uint8_t lcdc = memory.read(0xFF40);
    
    // Debug logging: Print when LY changes
    uint8_t ly = memory.read(0xFF44);
    static uint8_t lastLY = 255; // impossible value
    if (ly != lastLY) {
        std::cout << "LY=" << (int)ly << std::endl;
        lastLY = ly;
    }
    
    // If LCD is disabled, keep LY=0, STAT=mode 0, and do not render
    if (!(lcdc & 0x80)) {
        static bool lcdWasDisabled = false;
        if (!lcdWasDisabled) {
            cycleCounter = 0;
            memory.write(0xFF44, 0);  // LY = 0
            memory.write(0xFF41, memory.read(0xFF41) & 0xFC);  // Mode = 0
            lcdWasDisabled = true;
            std::cout << "PPU: LCD disabled (LCDC=0x" << std::hex << (int)lcdc << ") - resetting timing" << std::endl;
        }
        return;
    } else {
        static bool lcdWasEnabled = false;
        if (!lcdWasEnabled) {
            std::cout << "PPU: LCD enabled (LCDC=0x" << std::hex << (int)lcdc << ") - starting timing!" << std::endl;
            lcdWasEnabled = true;
        }
    }
    
    // LCD is enabled - run PPU timing
    cycleCounter += cycles;
    
    // Get current STAT and LYC (ly already declared above)
    uint8_t stat = memory.read(0xFF41);
    uint8_t lyc = memory.read(0xFF45);
    
    // Process cycles and advance LY counter
    while (cycleCounter >= CYCLES_PER_LINE) {
        cycleCounter -= CYCLES_PER_LINE;
        
        // Advance to next line
        ly++;
        
        // Handle VBlank start (LY = 144)
        if (ly == VISIBLE_LINES) {
            std::cout << "PPU: LY=" << (int)ly << " - VBlank started!" << std::endl;
            
            // Set VBlank interrupt flag (IF bit 0)
            uint8_t interruptFlag = memory.read(0xFF0F);
            interruptFlag |= 0x01;  // VBlank interrupt
            memory.write(0xFF0F, interruptFlag);
            std::cout << "PPU: VBlank interrupt requested!" << std::endl;
            
            vblankFlag = true;
        }
        
        // Handle frame end (LY = 154, wrap to 0)
        if (ly >= TOTAL_LINES) {
            std::cout << "PPU: LY=" << (int)ly << " - Frame complete, resetting to LY=0" << std::endl;
            render();
            ly = 0;
            vblankFlag = false;
        }
        
        // Update LY register
        memory.write(0xFF44, ly);
        
        // Update STAT coincidence flag (bit 2)
        if (ly == lyc) {
            stat |= 0x04;  // Set coincidence flag
        } else {
            stat &= ~0x04;  // Clear coincidence flag
        }
        
        // Set initial mode for new line
        uint8_t mode = 0;
        if (ly >= VISIBLE_LINES) {
            mode = 0x01;  // Mode 1: VBlank (LY ≥ 144)
        } else {
            mode = 0x02;  // Mode 2: OAM Search (start of visible line)
        }
        
        // Update STAT mode bits (bits 0-1)
        stat = (stat & 0xFC) | mode;
        memory.write(0xFF41, stat);
    }
    
    // Update mode based on current cycle position within the line
    if (ly < VISIBLE_LINES) {
        uint8_t mode = 0;
        if (cycleCounter < CYCLES_OAM) {
            mode = 0x02;  // Mode 2: OAM Search (0-79 cycles)
        } else if (cycleCounter < CYCLES_OAM + CYCLES_TRANSFER) {
            mode = 0x03;  // Mode 3: VRAM Scan (80-251 cycles)
        } else {
            mode = 0x00;  // Mode 0: HBlank (252-455 cycles)
        }
        
        // Update STAT mode bits (bits 0-1) only if mode changed
        uint8_t currentStat = memory.read(0xFF41);
        uint8_t newStat = (currentStat & 0xFC) | mode;
        if (newStat != currentStat) {
            memory.write(0xFF41, newStat);
        }
        
        // Check STAT interrupt conditions for mode changes
        uint8_t interruptFlag = memory.read(0xFF0F);
        bool statInterrupt = false;
        
        // Bit 3 = HBlank interrupt (mode 0)
        if ((currentStat & 0x08) && mode == 0x00) {
            statInterrupt = true;
        }
        // Bit 5 = OAM interrupt (mode 2)
        if ((currentStat & 0x20) && mode == 0x02) {
            statInterrupt = true;
        }
        // Bit 6 = LYC=LY interrupt (coincidence flag)
        if ((currentStat & 0x40) && (currentStat & 0x04)) {
            statInterrupt = true;
        }
        
        if (statInterrupt) {
            interruptFlag |= 0x02;  // Set STAT interrupt flag
            memory.write(0xFF0F, interruptFlag);
        }
    } else {
        // VBlank line (LY ≥ 144) - Mode 1
        uint8_t currentStat = memory.read(0xFF41);
        uint8_t newStat = (currentStat & 0xFC) | 0x01;  // Mode 1: VBlank
        if (newStat != currentStat) {
            memory.write(0xFF41, newStat);
        }
        
        // Check VBlank STAT interrupt
        uint8_t interruptFlag = memory.read(0xFF0F);
        if ((currentStat & 0x10)) {  // Bit 4 = VBlank interrupt
            interruptFlag |= 0x02;  // Set STAT interrupt flag
            memory.write(0xFF0F, interruptFlag);
        }
    }
}

// Check if we should begin a new frame (at start of VBlank)
bool PPU::beginFrame() {
    uint8_t ly = memory.read(0xFF44);
    uint8_t stat = memory.read(0xFF41);
    
    // TEMPORARY: Trigger frame render very frequently for testing
    // Frame begins at start of VBlank (LY = 144, Mode = 1)
    // For testing, trigger every 5 scanlines
    bool shouldBegin = (ly == VISIBLE_LINES && (stat & 0x03) == 0x01) || (ly % 5 == 0);
    
    // DEBUG: Print PPU state
    static int frame_check_count = 0;
    frame_check_count++;
    if (frame_check_count <= 20) {
        std::cout << "PPU beginFrame() #" << frame_check_count << " LY=" << (int)ly 
                  << " STAT=0x" << std::hex << (int)stat << " mode=" << (stat & 0x03)
                  << " shouldBegin=" << (shouldBegin ? "YES" : "NO") << std::endl;
    }
    
    return shouldBegin;
}

// Check if we should end the current frame
bool PPU::endFrame() {
    uint8_t ly = memory.read(0xFF44);
    // Frame ends when we reach line 153 (after VBlank)
    return (ly == TOTAL_LINES - 1);
}

