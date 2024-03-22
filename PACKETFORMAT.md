# Packet format

Most packets are of the general form

| Field                  | Size                                  | Description                                                  |
| :--------------------- | ------------------------------------- | ------------------------------------------------------------ |
| **Plain text section** | 10 bytes                              |                                                              |
| Sender                 | uint8_t                               | Node ID                                                      |
| Recipient              | uint8_t                               | Node ID                                                      |
| Payload number         | uint8_t                               | Rolling counter - note it can be re-used for larger payloads |
| Payload length         | uint8_t                               | Length of payload in encrypted section, minus checksum       |
| Payload type           | uint8_t                               | Internal to treacle                                          |
| Large payload start    | uint32_t (trimmed to 24 bits)         | Position of payload in multi-packet streams (max ~16MB)      |
| Next tick              | uint16_t                              |                                                              |
| **Encrypted section**  | up to 240 bytes in blocks of 16 bytes |                                                              |
| Payload                | varies                                |                                                              |
| Checksum               | uint16_t                              | Simple CRC16                                                 |
| Padding                | varies                                | Pads encrypted section to next block size                    |

## Sender values

Valid recipient NodeIDs are 1-254. 

- Sender 0/254 should never be used

## Recipient values

Valid recipient NodeIDs are 1-254. 

- Recipient 0 is a control message
- Recipient 254 is a 'flood'

## Packet number

This is a simple increasing counter per-node, not per protocol. It is used to deduplicate packets if received more than once.

## Checksum

The checksum is a standard CRC16 using a polynome chosen to work well. The checksum is part of the encrypted section to help authenticate that the packet is genuine. It does this by checksumming the whole packet up to the end of the payload before encryption. On decryption the checsum must still match.

## Padding

If encryption is used then the encrypted section must match a block size of sixteen bytes. With a maximum packet size of 250 bytes this equates to a maximum of 240 bytes in the encrypted section, leaving 10 bytes for other data.