
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

#include "Accept.h"
#include <sstream>
#include <iostream>

#define ZeroMemory RtlZeroMemory


using namespace std;

Accept::Accept(){

}

Backend Accept::accept_a_client(int control_server, LoadBalancer& l){
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(control_server, (struct sockaddr*)&client_addr, &client_len);

    if (client_fd < 0) {
        throw std::runtime_error("Accept failed on control socket. Error: ");
    }

    l.check();
}

bool Accept::poll_to_backends(int port, string IP){
    string httpreq = "GET / HTTP/1.1\r\nHost: " + IP + "\r\nConnection: close\r\n\r\n";

    int healthSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(healthSocket < 0){
        std::cerr << "invalid health socket\n";
        return false;
    }
    struct sockaddr_in target_addr;

    target_addr.sin_port = htons(port);
    target_addr.sin_family = AF_INET;
    inet_pton(AF_INET, IP.c_str(), &target_addr.sin_addr);

    int connection_result = connect(healthSocket, (sockaddr*)&target_addr, sizeof(target_addr));

    if (connection_result == 0 || connection_result < 0) {
        int y = closesocket(healthSocket);
        std::cerr << "[HealthCheck] Port " << port << " is DOWN or unreachable. Winsock Error: "<< "\n";
        return true; 
    }

    int bytes_sent = send(healthSocket, httpreq.c_str(), sizeof(httpreq), 0);

    if(bytes_sent == 0 || bytes_sent < 0){
        std::cerr << "[HealthCheck] Port connected, but failed to transmit data.\n";
        int y = closesocket(healthSocket);
        return false;
    }

    char response_buffer[128] = {0};
    int bytes_received = recv(healthSocket, response_buffer, sizeof(response_buffer) - 1, 0);

    int m = closesocket(healthSocket);

    if (bytes_received > 0 && std::string(response_buffer).find("200 OK") != std::string::npos) {
      //  std::cout << "[HealthCheck] Port " << port << " is ACTIVE and fully healthy.\n";
        return true;
    }

    std::cout << "Port :" << port << " IP: " << IP << "is inactive\n";
    return false;
}

void Accept::poll_for_connections(int control_server, LoadBalancer& l){

    std::cout << "[Accept] (Windows) Waiting for Node.js backends to check in...\n";
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(control_server, (struct sockaddr*)&client_addr, &client_len);
    std::cout << client_fd;

    if (client_fd < 0) {
        throw std::runtime_error("Accept failed on control socket. Error: ");
    }

    char buffer[1024] = {0};
    int bytes_read = recv(client_fd, buffer, sizeof(buffer)-1, 0);

    if(bytes_read > 0){
        std::string message(buffer); // Expecting: "REGISTER 127.0.0.1:3001\n"
        std::cout << "[Accept] Received message: " << message;

        if(message.rfind("REGISTER",0)==0){
            std::string cmd, ip_port_str;
            std::stringstream ss(message);
            ss >> cmd >> ip_port_str;

            size_t colon_pos = ip_port_str.find(':');

            if(colon_pos !=std::string::npos){
                std::string ip = ip_port_str.substr(0, colon_pos);
                int port = std::stoi(ip_port_str.substr(colon_pos + 1));

                Backend new_backend{ip, port, true, 0};
                l.add_connection(new_backend); 
            }
        }
    }
}
