#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "include/Accept.cpp"
#include "include/Backend.cpp"

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

int main(){

    WSADATA wsaData;
    int wsResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsResult != 0) {
        cerr << "[Main] WSAStartup failed with error: " << wsResult << endl;
        return 1;
    }
    int control_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (control_listen_socket < 0) {
        cerr << "[Main] Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    LoadBalancer loadBalancer = LoadBalancer();
    Accept accept = Accept();

    while(1){
        Backend back = accept.poll_for_connections(8081, loadBalancer);
    }
    return 0;
}