/*
 * MIT License
 *
 * Copyright (c) 2025 Peter K Green (pkg40)
 * Email: pkg40@yahoo.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once
#include <Arduino.h>
#pragma once
#include <stdint.h>

#pragma pack(push, 1)

struct deviceDataPacket {
    uint8_t version;     // Protocol version
    uint8_t command;     // Command type or operation
    uint8_t flags;       // Bitmask: ackRequired, encrypted, broadcast, etc.
    uint8_t seqId;       // Sequence or message ID

    union {
        uint8_t values[4];      // Generic data
        struct {
            int16_t param1;
            int16_t param2;
        };
    };

    uint8_t nonce[4];     // Random nonce (optional; for replay prevention)
    uint8_t tag[4];       // Optional integrity or HMAC tag (truncated)
    uint8_t senderMac[6]; // MAC address of the packet's originator
};

#pragma pack(pop)
