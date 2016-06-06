#include "lmcp.h"
#include "font7x5.h"
#include "defines.h"
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>


/*

Protocol is called LMCP (LedMatrixControlProtocol)

Works via UDP port 1337

Command format:
    uint8_t command:    the command
    [uint8_t data ...]: the data (can be non-existant)
    
Multiple commands can be sent in 1 datagram (optional).

Commands:
    0x01: write buffer, writes the current framebuffer to the screem
        no arguments
    0x02: clear, clears the matrix and writes the current framebuffer to the screen
        no arguments
    0x10: draw rows
        * uint y:
            y position of the row to draw (0-5)
        * uint8_t data[96*8]:
            send the data for 8 rows at a time, position is y * 8
    0x11: draw image rectangle
        * uint8_t x:
            top left x position of pixel data
        * uint8_t y:
            top left y position of pixel data
        * uint8_t width:
            width of pixel data
        * uint8_t height:
            height of pixel data
        * uint8_t data[width * height]:
            pixel data
    0x20: write text line based
        chars are 5x7 -> 6x8 including space and line separation
        * uint8_t x:
            top left x position in chars (0-15)
        * uint8_t y:
            top left y position in chars (0-5)
        * uint8_t brightness:
            brightness of text (0x00-0xFF)
        * [uint8_t text[...]]:
            text (ascii)
        * 0x00:
            terminator
    0x21: write text absolute
        * uint8_t x:
            top left x position in pixels (0-95)
        * uint8_t y:
            top left y position in pixels (0-47)
        * uint8_t brightness:
            brightness of text (0x00-0xFF)
        * [uint8_t text[...]]:
            text (ascii)
        * 0x00:
            terminator
    
*/

Lmcp::Lmcp(size_t width, size_t height, uint16_t bitdepth)
{
    this->width = width;
    this->height = height;
    this->bitdepth = bitdepth;
    this->set_color[0] = bitdepth/2;
    this->set_color[1] = bitdepth/2;
    this->set_color[2] = bitdepth/2;
}

// processes the incoming packets
bool Lmcp::processPacket(uint8_t* data, uint16_t packet_len)
{
    uint16_t packet_position = 0;
    // as long as there is data still...
    while(packet_position < packet_len)
    {
        // first byte is command
        uint8_t cmd = data[packet_position++];
        #ifdef DEBUG
        printf("Packet command:0x%x len:%d\n", cmd, packet_len);
        #endif
        switch(cmd)
        {
            // write buffer
            case 0x01:
                this->writeScreen();
                break;

            // clear
            case 0x02:
                this->clear();
                break;

            // draw rows
            case 0x10:
            {
                if(packet_len < 2)
                {
                    return false;
                }
                uint8_t y = data[packet_position++];

                packet_position += drawImage(0, y * 8, this->width, this->height, (uint8_t*)data + packet_position);

                break;
            }
            // draw image rectangle
            case 0x11:
            {
                // 4 bytes for header
                if(packet_len - packet_position < 4)
                    return false;
                uint8_t x = data[packet_position++];
                uint8_t y = data[packet_position++];
                uint8_t width = data[packet_position++];
                uint8_t height = data[packet_position++];

                // need enough bytes 
                if(packet_len - packet_position < width * height)
                    return false;

                packet_position += drawImage(x, y, width, height, (uint8_t*)(data + packet_position));
                
                break;
            }

            // write text line based
            case 0x20:
            // write text absolute
            case 0x21:
            {
                bool absolute = cmd == 0x21;
                uint8_t x = data[packet_position++];
                uint8_t y = data[packet_position++];
                uint8_t brightness = data[packet_position++];
                int16_t str_size = strnlen((char*)(data + packet_position), packet_len - packet_position);
                // string error
                if(str_size < 0)
                    return false;
                packet_position += drawString(
                    (char*)(data + packet_position), 
                    str_size, 
                    x, y, 
                    brightness,
                    absolute
                );
                break;
            }
            // draw rgb rows.
            case 0x30:
            {
                uint8_t y = data[packet_position++];
                packet_position += drawImageRgb(0, y*8, this->width, 1022 / this->width, (uint8_t *)data + packet_position);
                break;
            }
            // draw rgb image.
            case 0x31:
            {
                uint8_t x = data[packet_position++];
                uint8_t y = data[packet_position++];
                uint8_t width = data[packet_position++];
                uint8_t height = data[packet_position++];
                packet_position += drawImageRgb(x, y, width, height, (uint8_t *)(data + packet_position));
                break;
            }
            // set legacy color.
            case 0x32:
            {
                this->set_color[0] = data[packet_position++];
                this->set_color[1] = data[packet_position++];
                this->set_color[2] = data[packet_position++];
                break;
            }
            // unknown command -> ignore this packet
            default:
                return false;
        }
    }
    return true;
}


uint16_t Lmcp::drawStringNoLen(char* text, uint8_t x_pos, uint8_t y_pos, uint8_t brightness, bool absolute)
{
    return drawString(text, strlen(text), x_pos, y_pos, brightness, absolute);
}


// draw a string at x_pos, y_pos, optionally absolute position
uint16_t Lmcp::drawString(char* text, uint16_t len, size_t x_pos, size_t y_pos, uint8_t brightness, bool absolute)
{
    if(!absolute)
    {
        x_pos *= TEXT_CHAR_WIDTH + 1;
        y_pos *= TEXT_CHAR_HEIGHT + 1;
    }
    // if(x_pos + TEXT_CHAR_WIDTH >= width) goto writeText_exit;
    // if(y_pos + TEXT_CHAR_HEIGHT >= height) goto writeText_exit;
    if((x_pos + TEXT_CHAR_WIDTH >= this->width) || y_pos + TEXT_CHAR_HEIGHT >= this->height)
    {
        return len + 1;
    }

    for(int i = 0; i < len; i++)
    {
        int char_pos = (text[i] - 0x20);
        if(char_pos < 0 || char_pos > (0x7f - 0x20)) char_pos = 0;
        char_pos *= TEXT_CHAR_WIDTH; // 5 byte per char

        // draw 1 extra for the space, it's ok if it overflows there because setPixel will catch that
        for(size_t char_x = 0; char_x < TEXT_CHAR_WIDTH + 1; char_x++)
        {
            // this draws a 1 pixel empty column after each character
            uint8_t c = char_x == TEXT_CHAR_WIDTH ? 0 : Font5x7[char_pos + char_x];

            // draw 1 extra to make sure the pixels under each char are cleared too
            for(size_t k = 0; k < TEXT_CHAR_HEIGHT + 1; k++)
            {
                bool on = (c & (1 << k)) != 0;
                setPixel(
                    on ? brightness : 0x00,
                    x_pos + char_x + ((TEXT_CHAR_WIDTH + 1) * i), 
                    y_pos + k
                );
            }
        }
    }
    return len + 1;
}


// draws an image in the specified region
uint16_t Lmcp::drawImage(uint8_t x, uint8_t y, uint16_t width, uint16_t height, uint8_t* data)
{
    for(int y_pos = 0; y_pos < height; y_pos++)
    {
        for(int x_pos = 0; x_pos < width; x_pos++)
        {
            this->setPixel(*data++, x + x_pos, y + y_pos);
        }
    }
    return width * height;
}

uint16_t Lmcp::drawImageRgb(uint8_t x, uint8_t y, uint16_t width, uint16_t height, uint8_t *data)
{
    for(int y_pos = 0; y_pos < height; y_pos++)
    {
        for(int x_pos = 0; x_pos < width; x_pos++)
        {
            uint8_t r = *data++;
            uint8_t g = *data++;
            uint8_t b = *data++;
            this->setPixelRgb(r, g, b, x + x_pos , y + y_pos);
        }
    }
    return width * height;
}

// draw the curent framebuffer on the screen
void Lmcp::writeScreen()
{
}

// set pixel by x/y position
void Lmcp::setPixel(uint8_t val, uint8_t x, uint8_t y)
{
}

void Lmcp::setPixelRgb(uint8_t r, uint8_t g, uint8_t b, uint8_t x, uint8_t y)
{    
}

// clear the whole board
void Lmcp::clear()
{
}
