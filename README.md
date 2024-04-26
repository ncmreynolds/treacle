# Treacle

Treacle is a library for 'flooding' small machine-to-machine messages over multiple communications paths from a microcontroller.

The initial intention is to allow multiple microcontrollers (principally ESP32) to communicate using multiple transports interchangeably to abstract out the communication method from another of my libraries, [m2mMesh](https://github.com/ncmreynolds/m2mDirect).

Treacle is intended for infrequent, moderate latency transfer of small amounts of data like sensor readings, not low latency or large amounts of data.

It is most definitely a work in progress.

## Why treacle?

I was thinking about 'flooding' packets and the phrase 'from a trickle to a flood' popped into my head, and treacle is a sort of homophone of that.

## Scaling

The absolute maximum number of 'nodes' in a treacle network is 80, which is due to a decision to fit the routing information into a single ESP-Now or LoRa packet. This does not seem unreasonable for the expected use. Also, some platforms might struggle to do anything useful with their available SRAM even at this limit.

## Security

Treacle expects to use AES-128-CBC encryption of all packets with a fixed key and changing (but weak) initialisation vector so it is passably secure to casual eavesdropping but not secure at all for serious purposes. This may change to a different encryption scheme in future. On ESP32 it uses the hardware accelerated library, but on other platforms uses a fork of the common Arduino Crypto AES-CBC library.

The key is just set in your code so if somebody has access to that or the physical device it will be possible to recover it.

## Supported transports

Treacle uses other libraries to encapsulate and send packets so support is dependent on reasonably usable libraries for each platform. The starting point is always ESP32 support because ESP-Now support is a main driver of this project.



| Platform          | ESP-Now | UDP Multicast            | MQTT                     | LoRa    | Infrared | COBS    |
| ----------------- | ------- | ------------------------ | ------------------------ | ------- | -------- | ------- |
| ESP8266           | ✓       | ✓                        | ✓                        | ✓       | ?        | ✓       |
| ESP32             | ✓       | ✓                        | ✓                        | ✓       | Planned  | ✓       |
| AVR               |         | ✓ (Ethernet on Mega2560) | ✓ (Ethernet on Mega2560) | ✓       | ?        | ✓       |
| Raspberry Pi Pico |         | ?                        | Planned                  | Planned | ?        | Planned |

Transports are prioritised in the order they are initialised in the code.

If a node can expect to reach all its peers using the 'first' transport it will not use the 'second' or later transport at all, other than to issue keepalives. Keepalives are sent at least once every 60s to check how reachable the other nodes are. Reachability is determined using 'reliability' i.e. does a keepalive packet turn up roughly on time, so your code should not do lots of synchronous tasks or use delays for timing.

You can mix multiple transports freely and Treacle will handle which one best to send data with, but mixing ESP-Now with one that relies on a Wi-Fi connection to an access point (UDP/MQTT) isn't ideal, mostly due to issues with channel selection. The access point may dynamically manage which channel is in use and cause the connected 'node' to lose access to ESP-Now peers on a different channel.

### ESP-Now

This is a proprietary Espressif method for 'infrastructureless' connectivity using the Wi-Fi radio of their microcontrollers. It is a good choice anywhere you can't be guaranteed to have good access to local Wi-Fi. ESP-Now is considered the 'main' form of lower latency communication for Treacle nodes.

### UDP Multicast

If your devices have access to good local Wi-Fi then UDP is a great way to interconnect them. They don't need Internet access just Wi-Fi that functions locally. If one or more of your 'nodes' uses MQTT you should probably use UDP multicast as the primary transport instead of ESP-Now.

### MQTT

MQTT allows 'nodes' from anywhere with Internet connections to communicate, but relies on provision of a publicly accessible server. Some public MQTT servers exist but will most likely rate-limit your access so should only be used for testing.

### LoRa

LoRa is an excellent radio technology for long range use with microcontrollers, but is high latency and has strong limits on how often you can transmit. Making Treacle usable over LoRa along with ESP-Now is a driving factor in the design. Treacle respects the 1% duty cycle for LoRa use and will refuse to send packets that exceed this.

### Infrared

This use the RMT (remote) peripheral on ESP32 to modulate an infrared LED and read from an infrared received in a similar way to how basic IR remote controls work.

### COBS

This is a [generic method for encoding](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) packets and sending them over any Stream you supply. The general assumption is this will be a hardware serial port, but it could be something else.



## Getting started

There are numerous examples.
