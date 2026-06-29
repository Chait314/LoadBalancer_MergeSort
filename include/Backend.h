#ifndef BACKEND
#define BACKEND

#include <atomic>
#include <string>
#include <iostream>

using namespace std;

class Backend{
    public:
        string ip_address;
        int port;
        bool isHealthy;
        atomic<int> active_connections;

        Backend(string ip_address, int port, bool isHealthy, int active_connections);
        Backend(const Backend &c);
        Backend &operator=(const Backend &c);
};

#endif