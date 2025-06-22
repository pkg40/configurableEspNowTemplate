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
#include <globalConstants.h>
#include <deviceDataPacket.h>

class radioInterface;
class messageHandler;

enum class pairingState {
    idle, discovering, requestingPair, waitingAck, paired, connected, timeout
};

class pairingManager {
public:
    pairingManager(radioInterface*, messageHandler*);

    void beginPairing();
    void loop();
    void handlePacket(const deviceDataPacket& pkt);

    bool isPaired() const;
    const uint8_t* getPeerMac() const;

    static const char* toString(pairingState state);
    void setRadio(radioInterface* radioIn) { radio = radioIn; }

private:
    pairingState state = pairingState::idle;
    radioInterface* radio = nullptr;
    messageHandler* messenger = nullptr;

    unsigned long lastAction = 0;
    uint8_t retryCount = 0;
    uint8_t pairSequence = 0;
    uint8_t peerLastSequence = 0;
    uint8_t peerMac[6] = {0};

    void transition(pairingState nextState);
    void sendPairRequest();
    void sendHeartbeat();
    deviceDataPacket makePacket(CommandCode cmd);
};