#pragma once

#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
  #include <esp_now.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <espnow.h>
#else
  #error "Unsupported platform: This library only supports ESP32 and ESP8266."
#endif

#include <ringBuffer.hpp>
#include <pairingManager.hpp>
#include <radioInterface.hpp>
#include <globalConstants.h>

template <typename T>
class espNowCoPilot : public radioInterface {
    static_assert(sizeof(T) <= ESP_NOW_MAX_DATA_LEN, "Data type size exceeds ESP_NOW_MAX_DATA_LEN");

    // Callbacks
    using RxCallback = void (*)(const uint8_t* mac, const uint8_t* data, int len);
    using TxCallback = void (*)(const uint8_t* mac, esp_now_send_status_t status);

    static RxCallback _rxCallBack;
    static void* _rxCallBackArgs;
    static TxCallback _txCallBack;
    static void* _txCallBackArgs;

    static bool _txCallbackCalled;
    static bool _txCallbackSuccess;
    static bool _rxCallbackCalled;
    static bool _rxCallbackSuccess;

    static uint8_t _channel;
public:
    espNowCoPilot(pairingManager* pairing,
                  ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>* rx = nullptr,
                  ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>* tx = nullptr);
    ~espNowCoPilot();

    bool begin(uint8_t channel, wifi_mode_t mode, bool verbose = false);
    void loop();

    bool sendEspNow(const uint8_t* mac, const T& pkt);

        // 🔗 Implement interface method
    bool addPeer(const uint8_t* mac, uint8_t channel, const uint8_t* lmk = nullptr) override;

    ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>* getRXQueue();
    ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>* getTXQueue();

    static espNowCoPilot<T>* instance;

private:
    ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>* rxQueue;
    ringBuffer<T, DEVICE_MSG_BUFFER_SIZE>* txQueue;
    bool ownsQueues = false;

    pairingManager* pairingRef = nullptr;

    static void onReceive(const uint8_t* mac, const uint8_t* data, int len);
    static void onSend(const uint8_t* mac, esp_now_send_status_t status);
};

#include "espNowCoPilot.tpp"