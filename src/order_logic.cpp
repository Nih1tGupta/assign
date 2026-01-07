#pragma once
#include <unordered_map>
#include <cstdint>
#include "protocol.hpp"

/**
 * OrderDetails: Internal server state for a single order.
 * We store this to validate future CANCEL requests.
 */
struct OrderDetails {
    int32_t price;          // Original price from the NEW order [cite: 14, 18]
    int32_t size;           // Original size from the NEW order [cite: 14, 18]
    bool isCancelled;       // Track if this order has already been closed [cite: 25]
};