/*
 *	An Arduino library for treacle support
 *
 *	https://github.com/ncmreynolds/treacle
 *
 *	Released under LGPL-2.1 see https://github.com/ncmreynolds/treacle/LICENSE for full license
 *
 */
#ifndef treacleLoRa_cpp
#define treacleLoRa_cpp
#include "treacle.h"
/*
 *
 *	LoRa functions
 *
 */
#if defined(TREACLE_SUPPORT_LORA)
void treacleClass::setLoRaPins(int8_t cs, int8_t reset, int8_t irq)
{
	loRaCSpin = cs;						//LoRa radio chip select pin
	loRaResetPin = reset;				//LoRa radio reset pin
	loRaIrqPin = irq;					//LoRa radio interrupt pin
}
void treacleClass::setLoRaFrequency(uint32_t mhz)
{
	loRaFrequency = mhz;
}
void treacleClass::enableLoRa()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_enablingSpace);
		debugPrintln(treacleDebugString_LoRa);
	#endif
	loRaTransportId = numberOfActiveTransports++;
}
bool treacleClass::loRaEnabled()
{
	return loRaTransportId != 255;	//Not necessarily very useful, but it can be checked
}
bool treacleClass::initialiseLoRa()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_initialisingSpace);
		debugPrint(treacleDebugString_LoRa);
		debugPrint(':');
	#endif
	if(loRaIrqPin != -1)
	{
		LoRa.setPins(loRaCSpin, loRaResetPin, loRaIrqPin);		//Set CS, Reset & IRQ pin
	}
	else
	{
		LoRa.setPins(loRaCSpin, loRaResetPin);					//Set CS & Reset only
	}
	if (LoRa.begin(loRaFrequency) == true)						//This will hang of CS/IRQ pin not set appropriately
	{
		LoRa.setTxPower(loRaTxPower);							//Set TX power
		LoRa.setSpreadingFactor(loRaSpreadingFactor);			//Set spreading factor
		LoRa.setSignalBandwidth(loRaSignalBandwidth);			//Set badwidth
		LoRa.setGain(loRaRxGain);
		LoRa.setSyncWord(loRaSyncWord);							//Set sync word
		LoRa.enableCrc();										//Enable CRC check
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_OK);
		#endif
		transport[loRaTransportId].initialised = true;				//Mark as initialised
		transport[loRaTransportId].defaultTick = maximumTickTime;	//Set default tick timer
		transport[loRaTransportId].minimumTick = maximumTickTime/4;	//Set minimum tick timer
		if(loRaIrqPin != -1)									//Callbacks on IRQ pin
		{
			LoRa.onTxDone(										//Send callback function
				[]() {
					//Serial.println("LORA SENT");
					if(treacle.transport[treacle.loRaTransportId].txStartTime != 0)			//Check the initial send time was recorded
					{
						treacle.transport[treacle.loRaTransportId].txTime += micros()			//Add to the total transmit time
							- treacle.transport[treacle.loRaTransportId].txStartTime;
						treacle.transport[treacle.loRaTransportId].txStartTime = 0;			//Clear the initial send time
					}
					treacle.transport[treacle.loRaTransportId].txPackets++;					//Count the packet
				}
			);
			LoRa.onReceive(
				[](int receivedMessageLength) {
					//Serial.println("LORA RECEIVED");
					if(receivedMessageLength > 0)
					{
						if(treacle.receiveBufferSize == 0 && receivedMessageLength < treacle.maximumBufferSize)
						{
							treacle.transport[treacle.loRaTransportId].rxPackets++;				//Count the packet as received
							if(LoRa.peek() == (uint8_t)treacle.nodeId::allNodes ||
								LoRa.peek() == treacle.currentNodeId)							//Packet is meaningful to this node
							{
								LoRa.readBytes(treacle.receiveBuffer, receivedMessageLength);	//Copy the LoRa payload
								treacle.receiveBufferSize = receivedMessageLength;				//Record the amount of payload
								treacle.receiveBufferCrcChecked = false;						//Mark the payload as unchecked
								treacle.receiveTransport = treacle.loRaTransportId;				//Record that it was received by ESP-Now
								treacle.transport[treacle.loRaTransportId].rxPacketsProcessed++;//Count the packet as processed
							}
							else
							{
								treacle.transport[treacle.loRaTransportId].rxPacketsIgnored++;	//Count the ignore
							}
							return;
						}
						else
						{
							treacle.transport[treacle.loRaTransportId].rxPacketsDropped++;		//Count the drop
						}
						while(LoRa.available())													//Drop the packet
						{
							LoRa.read();
						}
					}
				}
			);
		}
		LoRa.receive();											//Start LoRa reception
	}
	else
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_failed);
		#endif
		transport[loRaTransportId].initialised = false;			//Mark as not initialised
	}
	return transport[loRaTransportId].initialised;
}
bool treacleClass::loRaInitialised()
{
	if(loRaTransportId != 255)
	{
		return transport[loRaTransportId].initialised;
	}
	return false;
}
uint32_t treacleClass::getLoRaRxPackets()
{
	if(loRaInitialised())
	{
		return transport[loRaTransportId].rxPackets;
	}
	return 0;
}
uint32_t treacleClass::getLoRaTxPackets()
{
	if(loRaInitialised())
	{
		return transport[loRaTransportId].txPackets;
	}
	return 0;
}
uint32_t treacleClass::getLoRaRxPacketsDropped()
{
	if(loRaInitialised())
	{
		return transport[loRaTransportId].rxPacketsDropped;
	}
	return 0;
}
uint32_t treacleClass::getLoRaTxPacketsDropped()
{
	if(loRaInitialised())
	{
		return transport[loRaTransportId].txPacketsDropped;
	}
	return 0;
}
float treacleClass::getLoRaDutyCycle()
{
	if(loRaInitialised())
	{
		return transport[loRaTransportId].calculatedDutyCycle;
	}
	return 0;
}
uint32_t treacleClass::getLoRaDutyCycleExceptions()
{
	if(loRaInitialised())
	{
		return transport[loRaTransportId].dutyCycleExceptions;
	}
	return 0;
}
void treacleClass::setLoRaTickInterval(uint16_t tick)
{
	transport[loRaTransportId].defaultTick = tick;
	transport[loRaTransportId].minimumTick = tick/4;
}
uint16_t treacleClass::getLoRaTickInterval()
{
	if(loRaInitialised())
	{
		return transport[loRaTransportId].nextTick;
	}
	return 0;
}
uint8_t treacleClass::getLoRaTxPower()
{
	if(loRaInitialised())
	{
		return loRaTxPower;
	}
	return 0;
}
void treacleClass::setLoRaTxPower(uint8_t value)
{
	loRaTxPower = value;
}
void treacleClass::setLoRaSpreadingFactor(uint8_t value)
{
	loRaSpreadingFactor = value;
}
void treacleClass::setLoRaSignalBandwidth(uint32_t value)
{
	loRaSignalBandwidth = value;
}
void treacleClass::setLoRaRxGain(uint8_t value)
{
	loRaRxGain = value;
}
uint8_t treacleClass::getLoRaSpreadingFactor()
{
	if(loRaInitialised())
	{
		return loRaSpreadingFactor;
	}
	return 0;
}
uint32_t treacleClass::getLoRaSignalBandwidth()
{
	if(loRaInitialised())
	{
		return loRaSignalBandwidth;
	}
	return 0;
}
bool treacleClass::sendBufferByLoRa(uint8_t* buffer, uint8_t packetSize)
{
	if(LoRa.beginPacket())
	{
		LoRa.write(buffer, packetSize);
		if(loRaIrqPin == -1)															//No LoRa IRQ, do this synchronously
		{
			transport[loRaTransportId].txStartTime = micros();
			if(LoRa.endPacket())
			{
				transport[loRaTransportId].txTime += micros()			//Add to the total transmit time
					- transport[loRaTransportId].txStartTime;
				transport[loRaTransportId].txStartTime = 0;			//Clear the initial send time
				transport[loRaTransportId].txPackets++;				//Count the packet
				return true;
			}
		}
		else
		{
			transport[loRaTransportId].txStartTime = micros();
			if(LoRa.endPacket(true))
			{
				return true;
			}
		}
	}
	transport[loRaTransportId].txStartTime = 0;
	return false;
}
bool treacleClass::receiveLoRa()
{
	uint8_t receivedMessageLength = LoRa.parsePacket();
	if(receivedMessageLength > 0)
	{
		if(receiveBufferSize == 0 && receivedMessageLength < maximumBufferSize)
		{
			transport[loRaTransportId].rxPackets++;						//Count the packet as received
			if(LoRa.peek() == (uint8_t)nodeId::allNodes ||
				LoRa.peek() == currentNodeId)							//Packet is meaningful to this node
			{
				lastLoRaRssi = LoRa.packetRssi();						//Record RSSI and SNR
				lastLoRaSNR = LoRa.packetSnr();
				LoRa.readBytes(receiveBuffer, receivedMessageLength);	//Copy the LoRa payload
				receiveBufferSize = receivedMessageLength;				//Record the amount of payload
				receiveBufferCrcChecked = false;						//Mark the payload as unchecked
				receiveTransport = loRaTransportId;						//Record that it was received by ESP-Now
				transport[loRaTransportId].rxPacketsProcessed++;		//Count the packet as processed
				return true;
			}
		}
		else
		{
			transport[loRaTransportId].rxPacketsDropped++;				//Count the drop
		}
		while(LoRa.available())											//Drop the packet
		{
			LoRa.read();
		}
	}
	return false;
}
uint16_t treacleClass::loRaRxReliability(uint8_t id)
{
	if(loRaInitialised())
	{
		uint8_t nodeIndex = nodeIndexFromId(id);
		if(nodeIndex != maximumNumberOfNodes)
		{
			return node[nodeIndex].rxReliability[loRaTransportId];
		}
	}
	return 0;
}
uint16_t treacleClass::loRaTxReliability(uint8_t id)
{
	if(loRaInitialised())
	{
		uint8_t nodeIndex = nodeIndexFromId(id);
		if(nodeIndex != maximumNumberOfNodes)
		{
			return node[nodeIndex].txReliability[loRaTransportId];
		}
	}
	return 0;
}
int16_t treacleClass::loRaRSSI(uint8_t id)
{
	if(loRaInitialised())
	{
		uint8_t nodeIndex = nodeIndexFromId(id);
		if(nodeIndex != maximumNumberOfNodes)
		{
			return rssi[nodeIndex];
		}
	}
	return 0;
}
float treacleClass::loRaSNR(uint8_t id)
{
	if(loRaInitialised())
	{
		uint8_t nodeIndex = nodeIndexFromId(id);
		if(nodeIndex != maximumNumberOfNodes)
		{
			return snr[nodeIndex];
		}
	}
	return 0;
}
#endif
#endif