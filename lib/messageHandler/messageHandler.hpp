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

#include <deviceDataPacket.h>
#include <globalConstants.h>
#include <ringBuffer.hpp>
#include <messageHandler.hpp>

/**
 * @brief Routes packets between system components without handling transport.
 */
class messageHandler {
public:
    messageHandler(ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* txQueue,
                   ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* rxQueue,
                   ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* handlerQueue);

    void loop();                                 // Routes packets from rx → handler
    bool enqueue(const deviceDataPacket& pkt);   // Push to txQueue
    bool dequeue(deviceDataPacket& pkt);         // Pull from rxQueue to user
    bool routeToHandler(const deviceDataPacket& pkt); // Push directly to handlerQueue

    ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* getTxQueue();
    ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* getRxQueue();
    ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* getHandlerQueue();

private:
    ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* txQueue;
    ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* rxQueue;
    ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE>* handlerQueue;
};