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

#include <messageHandler.hpp>

messageHandler::messageHandler(ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* tx,
                               ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* rx,
                               ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* handler)
    : txQueue(tx), rxQueue(rx), handlerQueue(handler) {}

void messageHandler::loop() {
    deviceDataPacket pkt;
    if (rxQueue && handlerQueue && rxQueue->pop(pkt)) {
        handlerQueue->push(pkt);
    }
}

bool messageHandler::enqueue(const deviceDataPacket& pkt) {
    return txQueue ? txQueue->push(pkt) : false;
}

bool messageHandler::dequeue(deviceDataPacket& pkt) {
    return rxQueue ? rxQueue->pop(pkt) : false;
}

bool messageHandler::routeToHandler(const deviceDataPacket& pkt) {
    return handlerQueue ? handlerQueue->push(pkt) : false;
}

ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* messageHandler::getTxQueue() {
    return txQueue;
}

ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* messageHandler::getRxQueue() {
    return rxQueue;
}

ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* messageHandler::getHandlerQueue() {
    return handlerQueue;
}