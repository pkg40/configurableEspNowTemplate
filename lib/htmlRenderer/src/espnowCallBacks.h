/* ************************************************************************************
 * espnowCallBacks.h
 ************************************************************************************
 * Copyright (c)       2016-2020 Peter K  Green            - pkg40@yahoo.com
 ************************************************************************************
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ************************************************************************************/
#pragma once
#include <Arduino.h>
#include<espnowHelper.hpp>
/*!SECTION
*/
/**-------------------------------------------
*\brief Send Callback function
*-------------------------------------------*/
void OnDataSentShort(const uint8_t mac[6], const uint8_t status) {
    myEspNow.setTXCallbackCalled(true);
    myEspNow.setTXCallbackSuccess(status == ESP_NOW_SEND_SUCCESS);
    Serial.printf("📡 ESP-NOW TX: Status=%s\n", status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILED");
}

void OnDataSent(const uint8_t mac[6], const uint8_t status, void* cbarg) {
    OnDataSentShort(mac, status);
    if (cbarg) {
        TxCallback callback = reinterpret_cast<TxCallback>(cbarg);
        callback(mac, status, cbarg);  // ✅ Invoke user-defined callback
    }
}
/**-------------------------------
*\brief Receive Callback function
*-------------------------------*/
template <typename T>
void OnDataRecvShort(const uint8_t mac[6], const uint8_t* data, size_t len) {
    if (len < sizeof(T)) {
        Serial.println("⚠️ ESP-NOW RX: Incomplete packet received!");
        espNowCoPilot<T>::_rxCallbackCalled = true;
        espNowCoPilot<T>::_rxCallbackSuccess = false;
        return;
    }

    T buffer;
    memcpy(&buffer, data, sizeof(T));  // ✅ Safe data copy into struct
    rxQueue.push(buffer);  // ✅ Push data into queue

    Serial.printf("📥 ESP-NOW RX: ID=%d, Size=%d bytes\n", buffer.id, len);

    if (espNowCoPilot<T>::_rxCallBack) {
        espNowCoPilot<T>::_rxCallBack(mac, buffer, len, espNowCoPilot<T>::_rxCallBackArgs);  // ✅ Call user-defined callback
    }

    espNowCoPilot<T>::_rxCallbackCalled = true;
    espNowCoPilot<T>::_rxCallbackSuccess = true;
}

/**-------------------------------
*\brief Receive Callback function with arguments
*-------------------------------*/
void OnDataRecv(const uint8_t mac[6], const uint8_t* data, size_t len, void* cbarg) {
    if (!cbarg) return;
    reinterpret_cast<RxCallback>(cbarg)(mac, data, len, cbarg);  // ✅ Invoke user-defined callback
}