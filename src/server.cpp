#include <iostream>
#include <vector>
#include "protocol.hpp"
#include "order_manager.hpp"

void handleIncomingData(char* buffer, OrderManager& om) {
    // Every message has MessageType at Offset 2 [cite: 14, 17]
    uint16_t type = *reinterpret_cast<uint16_t*>(buffer + 2);

    if (type == 1) { // It's a NEW Order
        auto* req = reinterpret_cast<NewOrderRequest*>(buffer);
        std::cout << "Received NEW: ClientID " << req->clientOrderId << "\n";
        
        int64_t serverId = om.processNewOrder(*req);
        // Next step: Send OrderAck back to client with serverId [cite: 19, 24]
    } 
    else if (type == 2) { // It's a CANCEL Order
        auto* req = reinterpret_cast<CancelOrderRequest*>(buffer);
        
        if (om.validateCancel(*req)) {
            std::cout << "CANCEL Accepted for ServerID " << req->serverOrderId << "\n";
            // Next step: Send CancelAck [cite: 24]
        } else {
            std::cout << "CANCEL Rejected!\n";
            // Next step: Send OrderReject [cite: 26, 28]
        }
    }
}

int main() {
    OrderManager myExch;
    // Simulate a buffer receiving 16 bytes from a socket
    char fakeBuffer[16]; 
    
    // In a real app, you would do: read(socket, fakeBuffer, 16);
    // Then call handleIncomingData(fakeBuffer, myExch);
    
    return 0;
}