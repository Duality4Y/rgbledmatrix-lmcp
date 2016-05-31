// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

/* system libraries. */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
/* local libraries. */
/* source includes */
#include <rpiRgbLmcpServer.h>
#include <matrix.h>
#include <interfaces.h>

void ki_func(int sig)
{
    printf("\nCaught keyboard interrupt! signal: %d\n", sig);
    exit(1);
}

int main(void)
{
    printf("Entering main. \n");
    signal(SIGINT, ki_func);
    LmcpServer server = LmcpServer(LEDBOARD_WIDTH, LEDBOARD_HEIGHT, 0xff);
    while(1)
    {
        server.run();
    }
    return 0;
}
