#ifndef LOAD_BALANCER
#define LOAD_BALANCER

#include "Backend.h"
#include "Accept.h"
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <algorithm>
class LoadBalancer{
    private:
        vector<Backend> backendConnections;
        string porte;
        int sockete;
        std::mutex mtx;
    public:
        LoadBalancer();
        LoadBalancer(int socket, string port);

        string getPort();
        void remove_connection(int port, string IP_address);
        void check();
        int getSocket();
        void add_connection(Backend backend);
        void accept_a_client(int control_server, LoadBalancer& l);
        bool poll_to_backends(int port, string IP);
        Backend choose_backend();
        Backend return_first();
};

#endif