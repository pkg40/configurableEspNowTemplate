
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


#include "pairingManager.hpp"
#include <messageHandler.hpp>
#include <radioInterface.hpp>
#include <globalConstants.h>
#include <string.h>

constexpr uint8_t maxRetries = 5;
constexpr unsigned long retryIntervalMs = 1000;
constexpr unsigned long heartbeatIntervalMs = 5000;

pairingManager::pairingManager(radioInterface* radioIn, messageHandler* messengerIn)
    : radio(radioIn), messenger(messengerIn) {}

void pairingManager::beginPairing() {
    pairSequence = random(1, 250);
    retryCount = 0;
    lastAction = millis();
    transition(pairingState::requestingPair);
}

void pairingManager::transition(pairingState nextState) {
    state = nextState;
    lastAction = millis();
    retryCount = 0;
}

bool pairingManager::isPaired() const {
    return state == pairingState::connected;
}

const uint8_t* pairingManager::getPeerMac() const {
    return peerMac;
}

deviceDataPacket pairingManager::makePacket(CommandCode cmd) {
    deviceDataPacket pkt = {};

    pkt.version = 1;
    pkt.command = static_cast<uint8_t>(cmd);
    pkt.flags = 0;  // Set flags as needed (e.g., ackRequired)
    pkt.seqId = ++peerLastSequence;

//    memcpy(pkt.senderMac, radio->getMacAddress(), 6);
    memcpy(pkt.nonce, &pkt.seqId, sizeof(pkt.nonce));  // Simple nonce example
    memset(pkt.tag, 0, sizeof(pkt.tag));  // Optional: fill with HMAC or checksum

    return pkt;
}

void pairingManager::sendPairRequest() {
    deviceDataPacket pkt = makePacket(CommandCode::PairRequest);
    pkt.seqId = pairSequence;
    pkt.flags |= 0x01;  // Example: mark as ackRequired
    messenger->enqueue(pkt);
    lastAction = millis();
    ++retryCount;
}

void pairingManager::sendHeartbeat() {
    deviceDataPacket pkt = makePacket(CommandCode::Heartbeat);
    messenger->enqueue(pkt);
    lastAction = millis();
}

void pairingManager::loop() {
    unsigned long now = millis();

    switch (state) {
    case pairingState::requestingPair:
    case pairingState::waitingAck:
        if (now - lastAction >= retryIntervalMs) {
            if (retryCount >= maxRetries) {
                transition(pairingState::timeout);
            } else {
                sendPairRequest();
                if (state == pairingState::requestingPair)
                    transition(pairingState::waitingAck);
            }
        }
        break;

    case pairingState::connected:
        if (now - lastAction >= heartbeatIntervalMs) {
            sendHeartbeat();
        }
        break;

    default:
        break;
    }
}

void pairingManager::handlePacket(const deviceDataPacket& pkt) {
    CommandCode cmd = static_cast<CommandCode>(pkt.command);

    if (cmd == CommandCode::PairAccept || cmd == CommandCode::Ack) {
        memcpy(peerMac, pkt.senderMac, 6);
        peerLastSequence = pkt.seqId;

        if (radio) {
            radio->addPeer(peerMac, pkt.flags, nullptr);  // Assuming flags carry channel or config
        }

        transition(pairingState::connected);
    } else if (cmd == CommandCode::Heartbeat) {
        lastAction = millis();
    }
}

const char* pairingManager::toString(pairingState state) {
    switch (state) {
    case pairingState::idle: return "idle";
    case pairingState::discovering: return "discovering";
    case pairingState::requestingPair: return "requestingPair";
    case pairingState::waitingAck: return "waitingAck";
    case pairingState::paired: return "paired";
    case pairingState::connected: return "connected";
    case pairingState::timeout: return "timeout";
    default: return "unknown";
    }
}