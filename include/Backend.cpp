#include "Backend.h"
#include <iostream>
#include <string>
#include <atomic>

using namespace std;

Backend::Backend(string ip_address, int port, bool isHealthy, int active_connections)
    : ip_address(std::move(ip_address)), port(port), isHealthy(isHealthy), active_connections(active_connections)
{
}

Backend::Backend(const Backend &c)
    : ip_address(c.ip_address), port(c.port), isHealthy(c.isHealthy),
      active_connections(c.active_connections.load(memory_order_relaxed))
{
}

Backend &Backend::operator=(const Backend& c){
    if(this != &c){
        this->ip_address = c.ip_address;
        this->port = c.port;
        this->isHealthy = c.isHealthy;
        this->active_connections.store(
            c.active_connections.load(memory_order_relaxed), memory_order_relaxed
        );
    }
    return *this;
}