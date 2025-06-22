#include <Arduino.h>
#include <espNowCoPilot.hpp>
#include <espNowCallbacks.hpp> // contains our callback functions (see below)
#include <deviceDataPacket.h> // your struct definition
#include <ringBuffer.hpp>

ringBuffer<deviceDataPacket, 10> rxQueue;
espNowHelper<deviceDataPacket> radio(myRxCallback, myTxCallback, &rxQueue);

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("🚀 Starting CopilotExample");

    if (!radio.connectEspnow(1, WIFI_MODE_AP, true)) {
        Serial.println("❌ Failed to initialize ESP-NOW");
        return;
    }

    deviceDataPacket packetToSend{.id = 1, .value = 42.0};
    radio.sendEspNow(broadcastMAC, packetToSend);
}

void loop() {
    // Example polling for new data in rxQueue
    if (!rxQueue.isEmpty()) {
        auto pkt = rxQueue.pop();
        Serial.printf("🎯 Received: id=%d, value=%.2f\n", pkt.id, pkt.value);
    }
    delay(100);
}