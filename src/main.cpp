// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

/* system libraries. */
#include <stdio.h>
/* local libraries. */
#include <graphics/graphics.h>
#include <eventhandler.h>
#include <matrixsim.h>
#include <lmcp.h>
/* source includes */
#include <matrix.h>
#include <interfaces.h>

EventHandler global_event_handler = EventHandler();

class Simulator: public Lmcp
{
public:
    Simulator(int, int, uint8_t);
    ~Simulator();

    void clear();
    void writeScreen();
    void setPixel(uint8_t, uint8_t, uint8_t);
    void run();

    Graphics *surface;
    Network *network;
    MatrixSimulator *sim;

};

Simulator::Simulator(int width, int height, uint8_t bitdepth):
Lmcp(width, height, bitdepth)
{
    rect_t screendims = {0, 0, width, height};
    this->surface = new Graphics(screendims);
    this->sim = new MatrixSimulator(screendims, 2);
    this->network = new Network(1337);
}

Simulator::~Simulator()
{}

void Simulator::clear()
{
    static RGBColor_t alloff = BLACK;
    this->surface->fill(alloff);
}

void Simulator::writeScreen()
{
    this->sim->process(this->surface);
}

void Simulator::setPixel(uint8_t color_val, uint8_t x, uint8_t y)
{
    static RGBColor_t color = BLACK;
    color.red = color_val * 3;
    this->surface->draw_pixel(x, y, color);
}

void Simulator::run()
{
    static int count;
    static uint8_t *buffp;

    // this is required for even handling of the simulator.
    global_event_handler.process();

    // if there is data (above zero bytes) we pass it to be processed by lmcp.
    // the protocol.
    buffp = this->network->receive(&count);
    if(count)
    {
        this->processPacket(buffp, count);
    }
}

void ki_func(int sig)
{
    printf("\nCaught keyboard interrupt! signal: %d\n", sig);
    exit(1);
}

int main(void)
{
    printf("Entering main. \n");
    signal(SIGINT, ki_func);
    // LmcpServer server = LmcpServer(LEDBOARD_WIDTH, LEDBOARD_HEIGHT, 0xff);
    Simulator sim = Simulator(LEDBOARD_WIDTH, LEDBOARD_HEIGHT, 0xff);
    while(1)
    {
        sim.run();
    }
    return 0;
}
