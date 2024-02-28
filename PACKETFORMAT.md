# Packet format

Most packets are of the general form

| Field                  | Size                                  | Description |
| :--------------------- | ------------------------------------- | ----------- |
| **Plain text section** | 6 bytes                               |             |
| Sender                 | uint8_t                               | Node ID     |
| Recipient              | uint8_t                               | Node ID     |
| Packet number          | uint16_t                              | Counter     |
| Next tick              | uint16_t                              | Timer       |
| **Encrypted section**  | up to 240 bytes in blocks of 16 bytes |             |
| Payload                | varies                                |             |
| Checksum               | uint16_t                              |             |
| Padding                | varies                                |             |

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

The checksum is a standard CRC16 using a polynome chosen to work well. The checksum is part of the encrypted section to help authenticate that the packet is genuine.

## Padding

If encryption is used then the encrypted section must match a block size of sixteen bytes. With a maximum packet size of 255 bytes this equates to a maximum of 240 bytes in the encrypted section, leaving 15 bytes for other data.