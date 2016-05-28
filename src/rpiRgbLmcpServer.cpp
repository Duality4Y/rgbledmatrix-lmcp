#include "rpiRgbLmcpServer.h"

LmcpServer::LmcpServer(size_t width, size_t height, uint16_t bitdepth):
Lmcp(width, height, bitdepth)
{
    // init gpio pins.
    if (!this->io.Init())
        fprintf(stderr, "failed to initialize GPIO's. \n");

    /*
    * Set up the RGBMatrix. It implements a 'Canvas' interface.
    */
    int rows = height;    // Ledmatrix is 32 led High
    int chain = LEDBOARD_CHAIN;    // 64 / 32 long.
    int parallel = LEDBOARD_PARALLEL; // single chain
    this->canvas = new RGBMatrix(&(this->io), rows, chain, parallel);
    this->clear();
    this->network = new Network(1337);
}

void LmcpServer::run()
{
    static int count;
    static uint8_t *buffp;
    buffp = this->network->receive(&count);
    if(count)
    {
        this->processPacket(buffp, count);
    }
}

void LmcpServer::setPixel(uint8_t val, uint8_t x, uint8_t y)
{
    if(x >= this->width) return;
    if(y >= this->height) return;
    this->canvas->SetPixel(x, y, val, 0, 0);
}

void LmcpServer::clear()
{
    this->canvas->Fill(0x00, 0x00, 0x00);
}

void LmcpServer::writeScreen()
{

}

LmcpServer::~LmcpServer()
{
    this->canvas->Clear();
    delete this->canvas;
}