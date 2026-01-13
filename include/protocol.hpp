#pragma once
#include <cstdint>

#pragma pack(push, 1)

// Client -> Server: NEW (16 bytes)
struct NewOrderRequest {
    uint16_t length;
    uint16_t messageType;   // 1
    uint32_t price;
    uint32_t size;
    int32_t  clientOrderId;
};

// Client -> Server: CANCEL (20 bytes)
struct CancelOrderRequest {
    uint16_t length;
    uint16_t messageType;   // 2
    uint32_t price;
    uint32_t size;
    uint64_t serverOrderId;
};

// Server -> Client: ACK (24 bytes)
struct OrderAck {
    uint16_t length = 24;
    uint16_t messageType;   // 1 NEW ACK, 2 CANCEL ACK
    uint32_t price;
    uint32_t size;
    int32_t  clientOrderId;
    uint64_t serverOrderId;
};

// Server -> Client: REJECT (24 bytes) ✅ FIXED
struct OrderReject {
    uint16_t length = 24;
    uint16_t messageType;   // 3 NEW REJ, 4 CANCEL REJ
    uint32_t price;
    uint32_t size;
    int32_t  clientOrderId;
    int64_t  serverOrderId;
};

#pragma pack(pop)
