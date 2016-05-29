// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#define LEDBOARD_WIDTH 64
#define LEDBOARD_HEIGHT 32
#define LEDBOARD_CHAIN LEDBOARD_WIDTH / LEDBOARD_HEIGHT
#define LEDBOARD_PARALLEL 1
#define LEDBOARD_BITDEPTH 0xff

#include <led-matrix.h>
#include <Lmcp.h>

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

class Interface
{

};

class NetworkInterface: public Interface
{
public:
    NetworkInterface(uint16_t);
    uint8_t *receive(int *);
    ~NetworkInterface(){};
private:
    const static uint16_t buffer_size = 65535;
    uint8_t buffer[buffer_size];
    uint16_t port;
    struct sockaddr_in addr;
    int sock;
};

NetworkInterface::NetworkInterface(uint16_t port)
{
    this->port = port;

    this->sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&(this->addr), 0, sizeof(this->addr));
    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    this->addr.sin_port = htons(this->port);
    bind(this->sock, (struct sockaddr*)&(this->addr), sizeof(this->addr));
}

uint8_t *NetworkInterface::receive(int *count)
{
    *count  = recvfrom(this->sock,
                       this->buffer,
                       this->buffer_size,
                       0, 0, 0);
    return this->buffer;
}

class LmcpServer: public Lmcp
{
public:
    LmcpServer(size_t, size_t, uint16_t);

    void clear();
    void writeScreen();
    void setPixel(uint8_t, uint8_t, uint8_t);

    void run();
    ~LmcpServer();
private:
    uint8_t test_data[2048];
    Canvas *canvas;
    GPIO io;
    NetworkInterface *network;
};

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
    this->network = new NetworkInterface(1337);
}

void LmcpServer::run()
{
    // int width = 32 * 2;
    // int height = 32;
    // this->test_data[0] = 0x11;
    // this->test_data[1] = 0x00;
    // this->test_data[2] = 0x00;
    // this->test_data[3] = width;
    // this->test_data[4] = height;
    // for(int i = 0; i < (width * height); i++)
    // {
    //     this->test_data[i + 5] = 0xFF;
    // }
    // this->processPacket(this->test_data, (width * height) + 5);
    // this->test_data[0] = 0x21;
    // this->test_data[1] = 0x00; // x = 0
    // this->test_data[2] = 0x00; // y = 0
    // this->test_data[3] = 0xFF; // brightness
    // char hello_world[] = "HelloWorld";
    // for(size_t i = 0; i < strlen(hello_world); i++)
    // {
    //     this->test_data[i+4] = hello_world[i];
    //     this->test_data[i+5] = 0;
    // }
    int count = 0;
    uint8_t *buffp = this->network->receive(&count);
    if(count)
    {
        this->processPacket(buffp, count);
    }
}

void LmcpServer::setPixel(uint8_t val, uint8_t x, uint8_t y)
{
    if(x >= this->width) return;
    if(y >= this->height) return;
    this->canvas->SetPixel(x, y, 0, 0, val);
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

// static void DrawOnCanvas(Canvas *canvas) {
//     /*
//     * Let's create a simple animation. We use the canvas to draw
//     * pixels. We wait between each step to have a slower animation.
//     */
//     canvas->Fill(0, 0, 255);

//     int center_x = canvas->width() / 2;
//     int center_y = canvas->height() / 2;
//     float radius_max = canvas->width() / 2;
//     float angle_step = 1.0 / 360;
//     for (float a = 0, r = 0; r < radius_max; a += angle_step, r += angle_step) {
//         float dot_x = cos(a * 2 * M_PI) * r;
//         float dot_y = sin(a * 2 * M_PI) * r;
//         canvas->SetPixel(center_x + dot_x, center_y + dot_y,
//         255, 0, 0);
//         usleep(1 * 1000);  // wait a little to slow down things.
//     }
// }

int main(void)
{
    printf("Entering main. \n");
    LmcpServer server = LmcpServer(LEDBOARD_WIDTH, LEDBOARD_HEIGHT, 0xff);
    while(1)
    {
        server.run();
    }
    return 0;
}
