// #include <iostream>
// #include <vector>
// #include <thread>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <cstring>
// #include <atomic>
// #include <chrono>
// #include "structure/protocol.hpp"

// std::atomic<int> total_sent(0);
// std::atomic<int> total_ack(0);

// void benchmark_client(int clientId) {
//     int sock = socket(AF_INET, SOCK_STREAM, 0);
//     struct sockaddr_in addr;
//     std::memset(&addr, 0, sizeof(addr));
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8080);
//     inet_pton(AF_INET, "192.168.0.190", &addr.sin_addr);

//     if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) return;

//     char buf[1024];
//     for (int i = 0; i < 1000; ++i) {
//         // 1. Prepare NEW Order
//         NewOrderRequest newReq;
//         newReq.length = 16;
//         newReq.messageType = 1;
//         newReq.price = 500;
//         newReq.size = 10;
//         newReq.clientOrderId = (clientId * 1000) + i;

//         // 2. Send and increment counter
//         if (send(sock, &newReq, sizeof(newReq), 0) > 0) {
//             total_sent++;
//         }

//         // 3. Receive ACK (This is the bottleneck - in real HFT we'd use a separate thread for recv)
//         int n = recv(sock, buf, sizeof(buf), 0);
//         if (n > 0) {
//             total_ack++;
//         }
//     }
//     close(sock);
// }

// int main() {
//     const int NUM_CLIENTS = 8;
//     std::vector<std::thread> threads;

//     std::cout << "Starting Performance Test: 8 Clients, 1000 msgs each...\n";
    
//     auto start = std::chrono::high_resolution_clock::now();

//     for (int i = 0; i < NUM_CLIENTS; ++i) {
//         threads.emplace_back(benchmark_client, i);
//     }

//     for (auto& t : threads) t.join();

//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> elapsed = end - start;

//     double mps = total_ack / elapsed.count();

//     std::cout << "Total Messages Sent    : " << total_sent << "\n";
//     std::cout << "Total ACKs Received    : " << total_ack << "\n";
//     std::cout << "Time Elapsed           : " << elapsed.count() << " seconds\n";
//     std::cout << "Average MPS            : " << mps << " msgs/sec\n";


//     return 0;
// }

  
 



#include <iostream>
#include <vector>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <atomic>
#include <chrono>
#include "structure/protocol.hpp"

// Atomic counters for thread-safety
std::atomic<int> total_sent(0);
std::atomic<int> total_ack(0);
std::atomic<int> total_rej(0); 

void benchmark_client(int clientId) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8081);
    inet_pton(AF_INET, "192.168.0.190", &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) return;

    char buf[1024];
    for (int i = 0; i < 1000; ++i) {
        // 1. Prepare NEW Order
        NewOrderRequest newReq;
        newReq.length = 16;
        newReq.messageType = 1;
        newReq.price = 500;
        newReq.size = 10;
        newReq.clientOrderId = (clientId * 1000) + i;

        // 2. Send order
        if (send(sock, &newReq, sizeof(newReq), 0) > 0) {
            total_sent++;
        }

        int n = recv(sock, buf, sizeof(buf), 0);
        if (n > 0) {
            uint16_t msgType = *reinterpret_cast<uint16_t*>(buf + 2);

            if (msgType == 1 || msgType == 2) {
                total_ack++; // Success
            } else if (msgType == 3 || msgType == 4) {
                total_rej++; 
            }
        }
    }
    close(sock);
}

int main() {
    const int NUM_CLIENTS = 8;
    std::vector<std::thread> threads;

    std::cout << "  8 Clients, 1000 msgs each...\n";
    
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_CLIENTS; ++i) {
        threads.emplace_back(benchmark_client, i);
    }

    for (auto& t : threads) t.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    double mps = total_sent / elapsed.count();

    std::cout << "Total Orders Sent      : " << total_sent << "\n";
    std::cout << "Total ACKs Received    : " << total_ack << " (Successes)\n";
    std::cout << "Total REJs Received    : " << total_rej << " (Failures)\n";
    std::cout << "Time Elapsed           : " << elapsed.count() << " seconds\n";
    std::cout << "MPS                    : " << mps << " orders/sec\n";


    return 0;
}
