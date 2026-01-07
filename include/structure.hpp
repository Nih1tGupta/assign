#pragma once
#include <cstdint>

#pragma pack(push, 1) // Forces exact byte alignment matching the PDF offsets

// --- Client to Server Messages ---

struct NewOrderRequest {
    uint16_t length;        // Offset 0 [cite: 14]
    uint16_t messageType;   // Offset 2 (1 for NEW) [cite: 14]
    int32_t price;          // Offset 4 [cite: 14]
    int32_t size;           // Offset 8 [cite: 14]
    int32_t clientOrderId;  // Offset 12 [cite: 14]
};

struct CancelOrderRequest {
    uint16_t length;        // Offset 0 [cite: 17]
    uint16_t messageType;   // Offset 2 (2 for CANCEL) [cite: 17]
    int32_t price;          // Offset 4 [cite: 17]
    int32_t size;           // Offset 8 [cite: 17]
    int32_t serverOrderId;  // Offset 12 (Assigned by Server) [cite: 17]
};

// --- Server to Client Responses ---

struct OrderAck {
    uint16_t length = 24;   // Total size [cite: 24]
    uint16_t messageType;   // 1 for NEW ACK, 2 for CANCEL ACK [cite: 24]
    int32_t price;          // Offset 4 [cite: 24]
    int32_t size;           // Offset 8 [cite: 24]
    int32_t clientOrderId;  // Offset 12 [cite: 24]
    int64_t exchOrderId;    // Offset 16 (8 bytes for ServerOrderID) [cite: 24]
};

struct OrderReject {
    uint16_t length = 16;   // Total size [cite: 28]
    uint16_t messageType;   // 3 for NEW REJECT, 4 for CANCEL REJECT [cite: 28]
    int32_t price;          // Offset 4 [cite: 28]
    int32_t size;           // Offset 8 [cite: 28]
    int32_t serverOrderId;  // Offset 12 (-1 for NEW REJECT) [cite: 28]
};

#pragma pack(pop)