#pragma once
#include <string>
#include <cstdint>

class HoverRenderer {
public:
    void* vtable;                 // 0x00
    uint8_t pad0[0x10 - 0x08];    // pad to 0x10
    std::string mFilteredContent; // 0x10
    uint8_t pad1[0x40 - 0x28];    // pad to 0x40
    float mCursorX;   // 0x40
    float mCursorY;   // 0x44
    float mOffsetX;   // 0x48
    float mOffsetY;   // 0x4C
    float mBoxWidth;  // 0x50
    float mBoxHeight; // 0x54
};
