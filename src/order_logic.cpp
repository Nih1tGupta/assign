#include "order_manager.hpp"

uint64_t OrderManager::addOrder(const NewOrderRequest& req) {
    uint64_t id = nextId++;
    orderBook[id] = {req.price, req.size, req.clientOrderId, false};
    return id;
}

bool OrderManager::tryCancel(const CancelOrderRequest& req, int32_t& outClientId) {
    auto it = orderBook.find(req.serverOrderId);

    if (it == orderBook.end()) {
        outClientId = -1;
        return false;
    }

    if (it->second.isCancelled ||
        it->second.price != req.price ||
        it->second.size  != req.size) {

        outClientId = it->second.clientOrderId;
        return false;
    }

    it->second.isCancelled = true;
    outClientId = it->second.clientOrderId;
    return true;
}
