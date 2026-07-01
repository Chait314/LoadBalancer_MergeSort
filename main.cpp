#if defined(_WIN32)
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0600
    #endif

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")


#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
#endif


#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include "include/Accept.cpp"
#include "include/Backend.cpp"
#include <cstring>


#pragma comment(lib, "Ws2_32.lib")
using namespace std;

struct addrinfo *results=NULL,*ptr = NULL, hints;

int main(){

    memset(&hints, 0, sizeof(hints));

    //hints specify the type of network, IPV4 or IPV6
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int res = getaddrinfo(NULL, PORT, &hints, &results);


    if(res != 0){
        printf("error ires %d\n", res);
        return 1;
    }

    int Listener = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

    if(Listener < 0){
        perror("error in socket");
        return 1;
    }

    auto bindSocket = bind(Listener, results->ai_addr, (int)results->ai_addrlen);

    if(bindSocket < 0){
        perror("error in bind");
        return 1;
    }

    if(listen(Listener, SOMAXCONN) < 0){
        perror("Listen failed w error");
        return 1;
    }

    LoadBalancer loadBalancer = LoadBalancer();
    Accept accept = Accept();

    while(1){
        try{
            cout <<"hh";
            Backend back = accept.poll_for_connections(Listener, loadBalancer);
            cout << "[Main] Successfully registered backend: " << back.ip_address << ":" << back.port << endl;
        }catch(const std::exception& e){
            cerr << "[Main] Error handling registration: " << e.what() << endl;
        }
        
    }
    return 0;
}