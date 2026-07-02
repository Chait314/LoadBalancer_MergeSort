#include <vector>
#include <string>
#include <atomic>
#include <limits>
#include <stdexcept>
#include "LoadBalancer.h"
#include <algorithm>

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

void LoadBalancer::check(){
    Accept a;
    for(auto& backend: backendConnections){
        bool b = a.poll_to_backends(backend.port, backend.ip_address);
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
