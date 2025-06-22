#pragma once

template <typename T>
espNowCoPilot<T>* espNowCoPilot<T>::instance = nullptr;

template <typename T>
espNowCoPilot<T>::espNowCoPilot(pairingManager* pairing,
                                ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>* rx,
                                ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>* tx)
    : rxQueue(rx), txQueue(tx), pairingRef(pairing) {
    instance = this;

    if (!rxQueue || !txQueue) {
        rxQueue = new ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>();
        txQueue = new ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>();
        ownsQueues = true;
    }

    esp_now_register_recv_cb(onReceive);
    esp_now_register_send_cb(onSend);
}

template <typename T>
espNowCoPilot<T>::~espNowCoPilot() {
    if (ownsQueues) {
        delete rxQueue;
        delete txQueue;
    }
    instance = nullptr;
}

template <typename T>
bool espNowCoPilot<T>::begin(uint8_t channel, wifi_mode_t mode, bool verbose) {
    WiFi.mode(mode);
    WiFi.disconnect(true);

    if (esp_now_init() != ESP_OK) {
        if (verbose) Serial.println("❌ esp_now_init failed");
        return false;
    }
    if (verbose) Serial.printf("✅ ESP-NOW initialized on channel %d\n", channel);
    return true;
}

template <typename T>
void espNowCoPilot<T>::onReceive(const uint8_t* mac, const uint8_t* data, int len) {
    if (!instance || !instance->rxQueue || len != sizeof(T)) return;

    T pkt;
    memcpy(&pkt, data, sizeof(T));
    // memcpy(pkt.senderMac, mac, 6);  // Optional if T has sender MAC
    instance->rxQueue->push(pkt);
}

template <typename T>
void espNowCoPilot<T>::onSend(const uint8_t* mac, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) {
        Serial.println("⚠️ ESP-NOW send failed");
    }
}

template <typename T>
void espNowCoPilot<T>::loop() {
    if (!pairingRef || !pairingRef->isPaired()) return;

    T pkt;
    if (txQueue && txQueue->pop(pkt)) {
        const uint8_t* mac = pairingRef->getPeerMac();
        sendEspNow(mac, pkt);
    }
}

template <typename T>
bool espNowCoPilot<T>::sendEspNow(const uint8_t* mac, const T& pkt) {
    return esp_now_send(mac, reinterpret_cast<const uint8_t*>(&pkt), sizeof(T)) == ESP_OK;
}

template <typename T>
bool espNowCoPilot<T>::addPeer(const uint8_t* mac, uint8_t channel, const uint8_t* lmk) {
#if defined(ESP32)
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = channel;
    peerInfo.encrypt = (lmk != nullptr);
    if (lmk) memcpy(peerInfo.lmk, lmk, 16);
    peerInfo.ifidx = WIFI_IF_AP;
    return esp_now_add_peer(&peerInfo) == ESP_OK;
#else
    return false;
#endif
}

template <typename T>
ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>* espNowCoPilot<T>::getRXQueue() { return rxQueue; }

template <typename T>
ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>* espNowCoPilot<T>::getTXQueue() { return txQueue; }