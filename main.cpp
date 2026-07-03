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
#include <thread>
#include <vector>
#include <string>
#include <atomic>
#include "include/Accept.cpp"
#include "include/Backend.cpp"
#include <cstring>

#define PORT2 "8080"


#pragma comment(lib, "Ws2_32.lib")

using namespace std;

struct addrinfo *results=NULL,*ptr = NULL, hints;

struct addrinfo *res_client=NULL,*pt_client=NULL, h_client;

void runBackendReg(Accept& accept, int listener, LoadBalancer& l){
    while (true) {
        try {
            accept.poll_for_connections(listener, l);
            std::cout << "[Registration Thread] Successfully registered a backend!\n";
        } catch (const std::exception& e) {
            std::cerr << "[Registration Thread] Error: " << e.what() << std::endl;
        }
    }
}

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

    sockaddr_in bound_addr;
    int bound_addr_len = sizeof(bound_addr);

    if(getsockname(Listener, (struct sockaddr*)&bound_addr, (socklen_t *)&bound_addr_len) == 0){
        char local_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &bound_addr.sin_addr, local_ip, sizeof(local_ip));

        int local_port = ntohs(bound_addr.sin_port);
        cout << "[Main] SUCCESS: Load Balancer is actively bound to local address: " 
        << local_ip << ":" << local_port << endl;
    }
    else{
        cerr << "[Main] Failed to retrieve bound address details. Error: ";
    }

    LoadBalancer loadBalancer = LoadBalancer();
    Accept accept = Accept();

    thread bg_run(runBackendReg, ref(accept), Listener, ref(loadBalancer));

    bg_run.detach();


    memset(&h_client,0,sizeof(h_client));
    h_client.ai_family = AF_INET;
    h_client.ai_protocol = IPPROTO_TCP;
    h_client.ai_flags = AI_PASSIVE;
    h_client.ai_socktype = SOCK_STREAM;

    int ires_client = getaddrinfo(NULL, PORT2, &h_client, &res_client);

    if(ires_client != 0){
        printf("error ires %d\n", res);
        return 1;
    }

    int client_listener = socket(res_client->ai_family, res_client->ai_socktype, res_client->ai_protocol);

    if(client_listener < 0){
        perror("error in socket");
        return 1;
    }

    auto client_bind = bind(client_listener, res_client->ai_addr, (int)res_client->ai_addrlen);

    if(client_bind < 0){
        perror("error in bind2");
        return 1;
    }

    if(listen(client_listener, SOMAXCONN) < 0){
        perror("Listen failed w error");
        return 1;
    }

    while(true){
        try{
            loadBalancer.accept_a_client(client_listener, loadBalancer);
        }catch(const std::exception& e){
            cerr << "Could not accept clients\n";
        }
    }
    return 0;
}