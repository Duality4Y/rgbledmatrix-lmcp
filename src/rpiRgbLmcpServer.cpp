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

    // create a 2D back buffer.
    this->back_buffer = new uint32_t*[width];
    for(size_t i = 0; i < width; i++)
    {
        this->back_buffer[i] = new uint32_t[height]; 
    }
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
    usleep(2e3);
}

void LmcpServer::setPixel(uint8_t val, uint8_t x, uint8_t y)
{
    static uint8_t cr, cg, cb, r, g, b;
    if(x >= this->width) return;
    if(y >= this->height) return;
    cr = this->set_color[0];
    cg = this->set_color[1];
    cb = this->set_color[2];
    r = ((uint64_t)(val * cr)) / 0xff;
    g = ((uint64_t)(val * cg)) / 0xff;
    b = ((uint64_t)(val * cb)) / 0xff;
    this->back_buffer[x][y] = (r << (2 * 8)) | (g << 8) | b;
    // this->canvas->SetPixel(x, y, r, g, b);
}

void LmcpServer::setPixelRgb(uint8_t r, uint8_t g, uint8_t b, uint8_t x, uint8_t y)
{
    if(x >= this->width) return;
    if(y >= this->height) return;
    this->back_buffer[x][y] = (r << (2 * 8)) | (g << 8) | b;
    // this->canvas->SetPixel(x, y, r, g, b);
}

void LmcpServer::clear()
{
    this->canvas->Fill(0x00, 0x00, 0x00);
}

void LmcpServer::writeScreen()
{
    for(size_t x = 0; x < this->width; x++)
    {
        for(size_t y = 0; y < this->height; y++)
        {
            uint8_t r = (this->back_buffer[x][y]) >> (2 * 8);
            uint8_t g = (this->back_buffer[x][y]) >> 8;
            uint8_t b = (this->back_buffer[x][y]);
            this->canvas->SetPixel(x, y, r, g, b);
        }
    }
}

LmcpServer::~LmcpServer()
{
    this->canvas->Clear();
    delete this->canvas;
    for(size_t i = 0; i < this->width; i++)
        delete [] this->back_buffer[i];
    delete [] this->back_buffer;
}