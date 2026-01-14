#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h> // htons(conversion wale)
#include <unistd.h>  // close
#include <cstring>
#include "../structure/protocol.hpp"


int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    //server ki iden. store karna hai 
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return -1;
    }

    std::cout << "Connected to server\n\n";

    NewOrderRequest newReq;
    newReq.length = 16;
    newReq.messageType = 1;
    newReq.price = 100;
    newReq.size = 120;
    newReq.clientOrderId = 55;

    std::cout << "Sending NEW order:\n";
    std::cout << "  Price          : " << newReq.price << "\n";
    std::cout << "  Size           : " << newReq.size << "\n";
    std::cout << "  ClientOrderID  : " << newReq.clientOrderId << "\n\n";

    send(sock, &newReq, sizeof(newReq), 0);

    // 1 buffer for (serv-> client) 
    char buf[1024];
    memset(buf,0,sizeof(buf));
    int n = recv(sock, buf, sizeof(buf), 0);
    if(n<=0){std::cerr<<"DISCONNECTED"<<std::endl;return -1;}
    uint16_t msgType = *reinterpret_cast<uint16_t*>(buf + 2);

    int64_t serverOrderId = -1;

    if (msgType == 1) { // NEW ACK
        auto* ack = reinterpret_cast<OrderAck*>(buf);
        serverOrderId = ack->serverOrderId;

        std::cout << "NEW ACK received:\n";
        std::cout<<"Msg Type"<<ack->messageType<<"\n";
        std::cout << "  Price          : " << ack->price << "\n";
        std::cout << "  Size           : " << ack->size << "\n";
        std::cout << "  ClientOrderID  : " << ack->clientOrderId << "\n";
        std::cout << "  ServerOrderID  : " << ack->serverOrderId << "\n\n";
    } else {
        auto* rej = reinterpret_cast<OrderReject*>(buf);
        std::cout << "NEW REJECT received:\n";
        std::cout<<"Msg Type"<<rej->messageType<<"\n";
        std::cout << "  Price          : " << rej->price << "\n";
        std::cout << "  Size           : " << rej->size << "\n";
        std::cout << "  ServerOrderID  : " << rej->serverOrderId << "\n\n";
        close(sock);
        return 0;
    }

    CancelOrderRequest cancelReq;
    cancelReq.length = 20;
    cancelReq.messageType = 2;
    cancelReq.price = 1010;
    cancelReq.size = 120;
    cancelReq.serverOrderId = serverOrderId;

    std::cout << "Sending CANCEL order:\n";
    std::cout<<"Msg Type           : " <<cancelReq.messageType<<"\n";
    std::cout << "  Price          : " << cancelReq.price << "\n";
    std::cout << "  Size           : " << cancelReq.size << "\n";
    std::cout << "  ServerOrderID  : " << cancelReq.serverOrderId << "\n\n";

    send(sock, &cancelReq, sizeof(cancelReq), 0);

    //memset(buf,0,sizeof(buf));
    n = recv(sock, buf, sizeof(buf), 0);
    msgType = *reinterpret_cast<uint16_t*>(buf + 2);

    if (msgType == 2) { // CANCEL ACK
        auto* ack = reinterpret_cast<OrderAck*>(buf);
        std::cout << "CANCEL ACK received:\n";
        std::cout << "  Price          : " << ack->price << "\n";
        std::cout << "  Size           : " << ack->size << "\n";
        std::cout << "  ClientOrderID  : " << ack->clientOrderId << "\n";
        std::cout << "  ServerOrderID  : " << ack->serverOrderId << "\n\n";
    } else {
        auto* rej = reinterpret_cast<OrderReject*>(buf);
        std::cout << "CANCEL REJECT received:\n";
        std::cout << "  MsgType          : " << rej->messageType << "\n";
        std::cout << "  Price          : " << rej->price << "\n";
        std::cout << "  Size           : " << rej->size << "\n";
        std::cout << "  ClientOrderId   :"<< rej->clientOrderId<< "\n";
        std::cout << "  ServerOrderID  : " << rej->serverOrderId << "\n\n";
    }

    close(sock);
    std::cout << "Client finished\n";
    return 0;
}
