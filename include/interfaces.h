#ifndef __INTERFACES_H__
#define __INTERFACES_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

class Interface
{
public:
    Interface();
    ~Interface();
    virtual uint8_t *receive(int *){ return NULL;};
};

class Network: public Interface
{
public:
    Network(uint16_t);
    ~Network();
    uint8_t *receive(int *);
private:
    const static uint16_t buffer_size = 65535;
    uint8_t buffer[buffer_size];
    uint16_t port;
    struct sockaddr_in addr;
    int sock;
};

#endif