#ifndef LOAD_BALANCER
#define LOAD_BALANCER

#include "Backend.h"
#include <vector>
#include <string>
#include <atomic>
#include <mutex>

class LoadBalancer{
    private:
        vector<Backend> backendConnections;
        std::mutex mtx;
    public:
        LoadBalancer();
        void add_connection(Backend backend);
        Backend choose_backend();
        Backend return_first();
};

#endif