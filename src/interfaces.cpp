#include "interfaces.h"

Interface::Interface()
{}

Interface::~Interface()
{}

Network::Network(uint16_t port)
{
    this->port = port;

    this->sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&(this->addr), 0, sizeof(this->addr));
    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    this->addr.sin_port = htons(this->port);
    bind(this->sock, (struct sockaddr*)&(this->addr), sizeof(this->addr));
}

Network::~Network()
{}

uint8_t *Network::receive(int *count)
{
    *count  = recvfrom(this->sock,
                       this->buffer,
                       this->buffer_size,
                       0, 0, 0);
    return this->buffer;
}