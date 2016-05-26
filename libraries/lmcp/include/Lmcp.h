#ifndef _LMCP_H_
#define _LMCP_H_

#include <stdint.h>
#include <stdio.h>

const size_t TEXT_CHAR_WIDTH = 5;
const size_t TEXT_CHAR_HEIGHT = 7;

class Lmcp
{
public:
    Lmcp(size_t, size_t, uint16_t);

    bool processPacket(uint8_t*, uint16_t);
    // output the buffer
    virtual void clear();
    virtual void writeScreen();
    virtual void setPixel(uint8_t val, uint8_t x, uint8_t y);
    
    size_t width;
    size_t height;

private:

    // draw functions
    uint16_t drawStringNoLen(char*, uint8_t, uint8_t, uint8_t brightness=0xFF, bool absolute=false);
    uint16_t drawString(char*, uint16_t, size_t, size_t, uint8_t brightness=0xFF, bool absolute=false);
    uint16_t drawImage(uint8_t x, uint8_t y, uint16_t width, uint16_t height, uint8_t* data);

    uint16_t bitdepth;
};

#endif