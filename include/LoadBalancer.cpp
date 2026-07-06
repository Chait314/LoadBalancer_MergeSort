
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


#include <vector>
#include <string>
#include <atomic>
#include <limits>
#include <stdexcept>
#include "LoadBalancer.h"
#include <algorithm>
#include <sstream>
#include <cstring>

#define PORT3 "8082"

LoadBalancer::LoadBalancer(){

}

void merge(vector<Backend>&backendC, int l, int mid,int r){
    vector<Backend>lefts;
    vector<Backend>rights;

    for(int ll = l; ll <= mid; ll++){
        lefts.push_back(backendC[ll]);
    }
    for(int rr = mid+1; rr <= r; rr++){
        rights.push_back(backendC[rr]);
    }
    
    int i = 0;
    int j = 0;
    int k = l;

    while(k <= r && i < lefts.size() && j < rights.size()){
        if(lefts[i].active_connections < rights[j].active_connections){
            backendC[k] = lefts[i++];
        }
        else{
            backendC[k] = rights[j++];
        }
        k++;
    }
    while(i < lefts.size()){
        backendC[k++] = lefts[i++];
    }
    while(j < rights.size()){
        backendC[k++] = rights[j++];
    }

    return;
}


void mergeSort(vector<Backend>&backendC, int l, int r){
    if(l >= r) return;

    int mid = l + (r-l)/2;
    mergeSort(backendC, l, mid);
    mergeSort(backendC, mid+1, r);
    merge(backendC, l, mid, r);
    return;
}



void LoadBalancer::remove_connection(int port, string IP_address){
    std::lock_guard<std::mutex>lock(mtx);

    backendConnections.erase(
        std::remove_if(backendConnections.begin(), backendConnections.end(),
            [&](const Backend& b) {
                return (b.ip_address == IP_address && b.port == port);
            }),
        backendConnections.end());

    std::cout << "Server: " << IP_address << ":" << port << " removed\n";
}

struct addrinfo *res_3 = NULL, *ptr_3 = NULL,hints_3;

void LoadBalancer::accept_a_client(int control_server, LoadBalancer& l){
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(control_server, (struct sockaddr*)&client_addr, &client_len);

    if (client_fd < 0) {
        throw std::runtime_error("Accept failed on control socket. Error: ");
    }

    l.check();

    Backend chosenBackend = l.choose_backend();

    stringstream ss;
    ss << "REDIRECT " << chosenBackend.ip_address << ":" << chosenBackend.port << "\n";
    string message = ss.str();

    if(send(client_fd, message.c_str(), sizeof(message), 0) <= 0){
        std::cerr << "Error in sending backend details to client" << "\n";
    }
    cout << "Backend: " << chosenBackend.ip_address << ":" << chosenBackend.port << "\n";
    return;
}

bool LoadBalancer::poll_to_backends(int port, string IP){
    string httpreq = "GET / HTTP/1.1\r\nHost: " + IP + "\r\nConnection: close\r\n\r\n";

    memset(&hints_3, 0, sizeof(hints_3));
    hints_3.ai_family = AF_INET;
    hints_3.ai_flags = AI_PASSIVE;
    hints_3.ai_socktype = SOCK_STREAM;
    hints_3.ai_protocol = IPPROTO_TCP;



    int healthSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    int ires = getaddrinfo(IP.c_str(), to_string(port).c_str(), &hints_3, &res_3);
    if (ires != 0) {
        std::cerr << "[HealthCheck] Address resolution failed for " << IP << ":" << port << "\n";
        return false;
    }

    if(healthSocket < 0){
        std::cerr << "invalid health socket\n";
        return false;
    }

    if(ires < 0){
        std::cerr << "invalid address resolved\n";
        return false;
    }

    int msgsnd_Socket = socket(res_3->ai_family, res_3->ai_socktype, res_3->ai_protocol);

    if(msgsnd_Socket < 0){
        freeaddrinfo((addrinfo*)ires);
        return false;
    }

    if (connect(msgsnd_Socket, res_3->ai_addr, (int)res_3->ai_addrlen) < 0) {
        std::cerr << "[HealthCheck] Outbound msgsnd connection failed.\n";
        freeaddrinfo(res_3);
        return false;
    }

    int msgrcv_socket = socket(res_3->ai_family, res_3->ai_socktype, res_3->ai_protocol);
    if (msgrcv_socket < 0) {
        freeaddrinfo(res_3);
        return false;
    }

    //std::cout << "[HealthCheck] Connecting outbound msgrcv socket...\n";
    if (connect(msgrcv_socket, res_3->ai_addr, (int)res_3->ai_addrlen) < 0) {
        std::cerr << "[HealthCheck] Outbound msgrcv connection failed.\n";
      //  closesocket(msgsnd_socket);
        //closesocket(msgrcv_socket);
        freeaddrinfo(res_3);
        return false;
    }

    freeaddrinfo((addrinfo*)ires);

    int bytes_sent = send(msgsnd_Socket, httpreq.c_str(), httpreq.length(), 0);
    
    // Shut down sending capability on this socket since we are finished talking 

    if (bytes_sent <= 0) {
      //  closesocket(msgsnd_socket);
       // closesocket(msgrcv_socket);
        cout << "No Bytes sent : " << IP << ":" << port <<'\n';
        return false;
    }

    char response_buffer[128] = {0};
    int bytes_received = recv(msgrcv_socket, response_buffer, sizeof(response_buffer) - 1, 0);


    if (bytes_received > 0 && std::string(response_buffer).find("200 OK") != std::string::npos) {
        std::cout << "[HealthCheck] Dual-sockets successfully verified backend status.\n";
        return true;
    }

    return false;
}

void LoadBalancer::check(){
    for(auto& backend: backendConnections){
        bool b = poll_to_backends(backend.port, backend.ip_address);
        if(b == false){
            remove_connection(backend.port, backend.ip_address);
        }
    }
    return;
}

Backend LoadBalancer:: return_first(){
    return backendConnections[0];
}

void LoadBalancer::add_connection(Backend backend){
    backendConnections.push_back(backend);
    int l = 0;
    int r = backendConnections.size();
    mergeSort(backendConnections, l, r-1);
    return;
}

Backend LoadBalancer::choose_backend(){
    std::lock_guard<std::mutex> lock(mtx);

    if (backendConnections.empty()) {
        throw std::runtime_error("No backend servers registered!");
    }
    Backend* best_backend = nullptr;
    int min_connections = std::numeric_limits<int>::max();

    for (auto& backend : backendConnections) {
        if (backend.isHealthy) {
            min_connections = backend.active_connections;
            best_backend = &backend;
            break;
        }
    }

    best_backend->active_connections++;
    int r = backendConnections.size();
    mergeSort(backendConnections, 0, r-1);

    return *best_backend;
}
