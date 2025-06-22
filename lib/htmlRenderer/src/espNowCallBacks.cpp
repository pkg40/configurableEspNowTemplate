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
#include <espNowCallBacks.hpp>

// example promiscuous cllback function
//  This function will be called for every received packet in promiscuous mode
//  You can filter packets based on type, channel, etc.
void IRAM_ATTR onPromiscuousRx(void *buf, wifi_promiscuous_pkt_type_t type)
{
    if (type != WIFI_PKT_MGMT)
        return; // Only process management frames

    const wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    const uint8_t *payload = pkt->payload;
    size_t len = pkt->rx_ctrl.sig_len;

    Serial.printf("📡 Packet received on channel %d, RSSI: %d dBm, length: %d bytes\n",
                  pkt->rx_ctrl.channel,
                  pkt->rx_ctrl.rssi,
                  len);
}

/*!SECTION
void IRAM_ATTR onPromiscuousRx2(void *buf, wifi_promiscuous_pkt_type_t type)
{
    if (type != WIFI_PKT_MGMT)
        return; // Focus on management frames

    const wifi_promiscuous_pkt_t *pkt = (const wifi_promiscuous_pkt_t *)buf;
    const uint8_t *payload = pkt->payload;
    size_t len = pkt->rx_ctrl.sig_len;

    if (len < sizeof(beaconPacket))
        return;

    beaconPacket beacon;
    memcpy(&beacon, payload, sizeof(BeaconPacket));

    if (beacon.version != 1 || beacon.deviceType != 1)
        return;

    String sharedSecret = config.getValue("espnow", "sharedsecret");
    if (sharedSecret.isEmpty())
    {
        Serial.println("❌ Shared secret not configured.");
        return;
    }
    if (myEspNow.verifyBeacon(beacon, reinterpret_cast<const uint8_t*>(sharedSecret.c_str()), sharedSecret.length()))
    {
        Serial.printf("✅ Verified beacon from MAC %02X:%02X:%02X:%02X:%02X:%02X\n",
                      beacon.mac[0], beacon.mac[1], beacon.mac[2],
                      beacon.mac[3], beacon.mac[4], beacon.mac[5]);

        uint8_t lmkBytes[16];
        String lmkHex = config.getValue("espnow", "lmk");

        if (!config.parseHexStringToBytes(lmkHex, lmkBytes, sizeof(lmkBytes)))
        {
            Serial.println("❌ Failed to parse LMK");
            return;
        }
        myEspNow.addPeer(beacon.mac, pkt->rx_ctrl.channel, lmkBytes);

        // Optional: save to config here
    }
    else
    {
        Serial.println("❌ Invalid HMAC on beacon");
    }
}
*/

// Example receive callback (Rx)
void myRxCallback(const uint8_t *mac, const uint8_t *data, size_t len, void *cbarg)
{
    if (len != sizeof(MyData))
    {
        Serial.println("⚠️ Invalid payload size.");
        return;
    }

    MyData received;
    memcpy(&received, data, sizeof(MyData));

    Serial.printf("📥 Received data from %02X:%02X... | ID: %d, Value: %.2f\n",
                  mac[0], mac[1], received.id, received.value);

    // Optionally cast cbarg if you're passing extra info
}

// Example transmit callback (Tx)
void myTxCallback(const uint8_t *mac, const uint8_t status, void *cbarg)
{
    Serial.printf("📡 Send status for %02X:%02X...: %s\n", mac[0], mac[1],
                  (status == ESP_NOW_SEND_SUCCESS) ? "Success" : "Failed");

    // Optionally notify or log more data
}