// You can tweak this value anytime to change all queue sizes
#pragma once
constexpr size_t DEVICE_MSG_BUFFER_SIZE = 8;

enum class CommandCode : uint8_t {
    PairRequest  = 0x01,
    PairAccept   = 0x02,
    Heartbeat    = 0x03,
    Ack          = 0x04,
    // Add more as needed...
};
