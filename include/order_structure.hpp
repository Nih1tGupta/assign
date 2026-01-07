#include "order_manager.hpp"

// Implementation of the NEW order logic
int64_t OrderManager::processNewOrder(const NewOrderRequest& req) {
    // Generate a unique ID starting from 1000 [cite: 19]
    int64_t id = nextServerOrderId++; 
    
    // Create a record in our map using the assigned ID [cite: 19]
    // We store the price and size to validate future cancels [cite: 18, 20]
    orderBook[id] = {req.price, req.size, false}; 
    
    return id; // This ID will be sent back in the OrderAck [cite: 19, 24]
}

// Implementation of the CANCEL validation logic
bool OrderManager::validateCancel(const CancelOrderRequest& req) {
    // Look for the ServerOrderID provided by the client [cite: 17, 20]
    auto it = orderBook.find(req.serverOrderId);
    
    // Check if:
    // 1. The ID exists in our record [cite: 25]
    // 2. It hasn't been cancelled already [cite: 25]
    // 3. The Price and Size match the original order [cite: 20]
    if (it != orderBook.end() && 
        !it->second.isCancelled && 
        it->second.price == req.price && 
        it->second.size == req.size) {
        
        // If all checks pass, mark it as cancelled so it can't be cancelled again [cite: 25]
        it->second.isCancelled = true; 
        return true; 
    }
    
    // If any check fails, the server must send a REJECT [cite: 26]
    return false; 
}