#ifndef ACCEPT
#define ACCEPT
#if defined(_WIN32)
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0600
    #endif

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "Accept.h"
#pragma comment(lib, "Ws2_32.lib")


#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

#define PORT "8081"
#include "loadBalancer.cpp"
using namespace std;

class Accept{
    public:
    Accept();
    void poll_for_connections(int control_server, LoadBalancer& l);
    bool poll_to_backends(int port, string IP);
};

#endif