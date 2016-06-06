#ifndef __LMCPSERVER_H__
#define __LMCPSERVER_H__

#define LEDBOARD_CHAIN LEDBOARD_WIDTH / LEDBOARD_HEIGHT
#define LEDBOARD_PARALLEL 1

#include <unistd.h>
#include <vector>

#include <led-matrix.h>
#include <lmcp.h>

#include <matrix.h>
#include <interfaces.h>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

class LmcpServer: public Lmcp
{
public:
    LmcpServer(size_t, size_t, uint16_t);

    void clear();
    void writeScreen();
    void setPixel(uint8_t, uint8_t, uint8_t);
    void setPixelRgb(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

    void run();
    ~LmcpServer();
private:
    uint32_t **back_buffer;
    GPIO io;
    Canvas *canvas;
    Network *network;
};

#endif