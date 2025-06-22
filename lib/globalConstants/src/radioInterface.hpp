#pragma once

#include <stdint.h>

class radioInterface {
public:
    virtual bool addPeer(const uint8_t* mac, uint8_t channel, const uint8_t* lmk = nullptr) = 0;
    virtual ~radioInterface() = default;
};