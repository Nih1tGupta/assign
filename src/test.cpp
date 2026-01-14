#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "../structure/protocol.hpp"

// Helper: receive exact number of bytes
bool recvExact(int sock, void* buffer, size_t len) {
    size_t total = 0;
    char* buf = static_cast<char*>(buffer);

    while (total < len) {
        ssize_t n = recv(sock, buf + total, len - total, 0);
        if (n <= 0) return false;
        total += n;
    }
    return true;
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return -1;
    }

    std::cout << "Connected to server\n\n";

    // ------------------ SEND NEW ------------------
    NewOrderRequest newReq{};
    newReq.length = sizeof(NewOrderRequest);
    newReq.messageType = 1;
    newReq.price = 100;
    newReq.size = 120;
    newReq.clientOrderId = 55;

    send(sock, &newReq, sizeof(newReq), 0);

    // ------------------ RECEIVE NEW RESPONSE ------------------
    uint16_t len = 0;
    if (!recvExact(sock, &len, sizeof(len))) return -1;

    char buf[1024]{};
    *reinterpret_cast<uint16_t*>(buf) = len;

    if (!recvExact(sock, buf + 2, len - 2)) return -1;

    uint16_t msgType = *reinterpret_cast<uint16_t*>(buf + 2);
    uint64_t serverOrderId = 0;

    if (msgType == 1) { // NEW ACK
        auto* ack = reinterpret_cast<OrderAck*>(buf);
        serverOrderId = ack->serverOrderId;

        std::cout << "NEW ACK received:\n";
        std::cout << "  Price         : " << ack->price << "\n";
        std::cout << "  Size          : " << ack->size << "\n";
        std::cout << "  ClientOrderID : " << ack->clientOrderId << "\n";
        std::cout << "  ServerOrderID : " << ack->serverOrderId << "\n\n";
    } else {
        auto* rej = reinterpret_cast<OrderReject*>(buf);
        std::cout << "NEW REJECT received\n";
        close(sock);
        return 0;
    }

    // ------------------ SEND CANCEL (WRONG PRICE) ------------------
    CancelOrderRequest cancelReq{};
    cancelReq.length = sizeof(CancelOrderRequest);
    cancelReq.messageType = 2;
    cancelReq.price = 1010;        // ❌ wrong price
    cancelReq.size = 120;
    cancelReq.serverOrderId = serverOrderId;

    send(sock, &cancelReq, sizeof(cancelReq), 0);

    // ------------------ RECEIVE CANCEL RESPONSE ------------------
    if (!recvExact(sock, &len, sizeof(len))) return -1;
    memset(buf, 0, sizeof(buf));
    *reinterpret_cast<uint16_t*>(buf) = len;

    if (!recvExact(sock, buf + 2, len - 2)) return -1;

    msgType = *reinterpret_cast<uint16_t*>(buf + 2);

    if (msgType == 2) {
        auto* ack = reinterpret_cast<OrderAck*>(buf);
        std::cout << "CANCEL ACK received (unexpected)\n";
    } else {
        auto* rej = reinterpret_cast<OrderReject*>(buf);
        std::cout << "CANCEL REJECT received:\n";
        std::cout << "  MsgType        : " << rej->messageType << "\n";
        std::cout << "  Price          : " << rej->price << "\n";
        std::cout << "  Size           : " << rej->size << "\n";
        std::cout << "  ClientOrderID  : " << rej->clientOrderId << "\n";
        std::cout << "  ServerOrderID  : " << rej->serverOrderId << "\n\n";
    }

    close(sock);
    std::cout << "Client finished\n";
    return 0;
}
