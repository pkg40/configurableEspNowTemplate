# globalConstants

This library contains shared command codes and constants for your ESP-NOW communication stack, including `CommandCode` enums used in pairing, messaging, and heartbeat protocols.

## Usage

```cpp
#include <globalConstants.h>

if (static_cast<CommandCode>(pkt.command) == CommandCode::Ack) {
    // handle acknowledgment
}