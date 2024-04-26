/*
 *	An Arduino library for treacle support
 *
 *	https://github.com/ncmreynolds/treacle
 *
 *	Released under LGPL-2.1 see https://github.com/ncmreynolds/treacle/LICENSE for full license
 *
 */
#ifndef treacleAsyncUdp_cpp
#define treacleAsyncUdp_cpp
#include "treacle.h"
/*
 *
 *	UDP functions
 *
 */
#if defined(TREACLE_SUPPORT_UDP)
void treacleClass::setUDPMulticastAddress(IPAddress address)
{
	udpMulticastAddress = address;
}
void treacleClass::setUDPport(uint16_t port)
{
	udpPort = port;
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_UDPspace);
		debugPrint(treacleDebugString_port);
		debugPrint(':');
		debugPrintln(udpPort);
	#endif
}
void treacleClass::enableUDP()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_enablingSpace);
		debugPrintln(treacleDebugString_UDP);
	#endif
	UDPTransportId = numberOfActiveTransports++;
}
bool treacleClass::UDPEnabled()
{
	return UDPTransportId != 255;	//Not necessarily very useful, but it can be checked
}
bool treacleClass::initialiseUDP()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_initialisingSpace);
		debugPrint(treacleDebugString_UDP);
		debugPrint(':');
	#endif
	#if defined(ESP8266)
		udp = new WiFiUDP;
		if(udp->beginMulticast(WiFi.localIP(),udpMulticastAddress, udpPort))
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_OK);
			#endif
			transport[UDPTransportId].initialised = true;				//Mark as initialised
			transport[UDPTransportId].defaultTick = maximumTickTime/10;	//Set default tick timer
		}
	#elif defined(ESP32)
		udp = new AsyncUDP;
		if(udp->listenMulticast(udpMulticastAddress, udpPort))
		{
			udp->onPacket(
				[](AsyncUDPPacket receivedMessage)
				{
					#if defined(TREACLE_DEBUG)
						/*
						treacle.debugPrint("UDP Packet Type: ");
						treacle.debugPrint(receivedMessage.isBroadcast()?"Broadcast":receivedMessage.isMulticast()?"Multicast":"Unicast");
						treacle.debugPrint(", From: ");
						treacle.debugPrint(receivedMessage.remoteIP());
						treacle.debugPrint(":");
						treacle.debugPrint(receivedMessage.remotePort());
						treacle.debugPrint(", To: ");
						treacle.debugPrint(receivedMessage.localIP());
						treacle.debugPrint(":");
						treacle.debugPrint(receivedMessage.localPort());
						treacle.debugPrint(", Length: ");
						treacle.debugPrint(receivedMessage.length());
						//Serial.print(", Data: ");
						//Serial.write(receivedMessage.data(), receivedMessage.length());
						treacle.debugPrintln();
						*/
					#endif
					if(treacle.receiveBufferSize == 0 && receivedMessage.length() < treacle.maximumBufferSize)
					{
						treacle.transport[treacle.UDPTransportId].rxPackets++;						//Count the packet as received
						if(receivedMessage.data()[0] == (uint8_t)treacle.nodeId::allNodes ||
							receivedMessage.data()[0] == treacle.currentNodeId)						//Packet is meaningful to this node
						{
							memcpy(treacle.receiveBuffer, receivedMessage.data(), receivedMessage.length());	//Copy the UDP payload into the receive buffer
							treacle.receiveBufferSize = receivedMessage.length();								//Record the amount of payload
							treacle.receiveBufferCrcChecked = false;											//Mark the payload as unchecked
							treacle.receiveTransport = treacle.UDPTransportId;									//Record that it was received by UDP
							treacle.transport[treacle.UDPTransportId].rxPacketsProcessed++;						//Count the packet as processed
						}
						return;
					}
					else
					{
						treacle.transport[treacle.UDPTransportId].rxPacketsDropped++;					//Count the drop
					}
				});
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_OK);
			#endif
			transport[UDPTransportId].initialised = true;				//Mark as initialised
			transport[UDPTransportId].defaultTick = maximumTickTime/10;	//Set default tick timer
		}
	#elif defined(AVR)
		udp = new EthernetUDP;
		if(udp->beginMulticast(udpMulticastAddress, udpPort))
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_OK);
			#endif
			transport[UDPTransportId].initialised = true;				//Mark as initialised
			transport[UDPTransportId].defaultTick = maximumTickTime/10;	//Set default tick timer
		}
	#endif
	else
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_failed);
		#endif
		transport[UDPTransportId].initialised = false;				//Mark as not initialised
	}
	return transport[UDPTransportId].initialised;
}
#if defined(ESP8266) || defined(AVR)
bool treacleClass::receiveUDP()
{
	uint8_t receivedMessageLength = udp->parsePacket();
	if(receivedMessageLength > 0)
	{
		if(receiveBufferSize == 0 && receivedMessageLength < maximumBufferSize)
		{
			transport[UDPTransportId].rxPackets++;						//Count the packet as received
			if(udp->peek() == (uint8_t)nodeId::allNodes ||
				udp->peek() == currentNodeId)							//Packet is meaningful to this node
			{
				udp->read(receiveBuffer, receivedMessageLength);		//Copy the UDP payload
				receiveBufferSize = receivedMessageLength;				//Record the amount of payload
				receiveBufferCrcChecked = false;						//Mark the payload as unchecked
				receiveTransport = UDPTransportId;						//Record that it was received by ESP-Now
				transport[UDPTransportId].rxPacketsProcessed++;		//Count the packet as processed
				return true;
			}
		}
		else
		{
			transport[UDPTransportId].rxPacketsDropped++;				//Count the drop
		}
		while(udp->available())											//Drop the packet
		{
			udp->read();
		}
	}
	return false;
}
#endif
bool treacleClass::sendBufferByUDP(uint8_t* buffer, uint8_t packetSize)
{
	transport[UDPTransportId].txStartTime = micros();
	#if defined(ESP8266)
		udp->beginPacketMulticast(udpMulticastAddress, udpPort, WiFi.localIP());
		udp->write(buffer, packetSize);
		if(udp->endPacket())
		{
			transport[UDPTransportId].txTime += micros()			//Add to the total transmit time
				- transport[UDPTransportId].txStartTime;
			transport[UDPTransportId].txStartTime = 0;				//Clear the initial send time
			transport[UDPTransportId].txPackets++;					//Count the packet
			return true;
		}
	#elif defined(ESP32)
		if(udp->write(buffer, packetSize))
		{
			transport[UDPTransportId].txTime += micros()			//Add to the total transmit time
				- transport[UDPTransportId].txStartTime;
			transport[UDPTransportId].txStartTime = 0;				//Clear the initial send time
			transport[UDPTransportId].txPackets++;					//Count the packet
			return true;
		}
	#elif defined(AVR)
		udp->beginPacket(udpMulticastAddress, udpPort);
		udp->write(buffer, packetSize);
		if(udp->endPacket())
		{
			transport[UDPTransportId].txTime += micros()			//Add to the total transmit time
				- transport[UDPTransportId].txStartTime;
			transport[UDPTransportId].txStartTime = 0;				//Clear the initial send time
			transport[UDPTransportId].txPackets++;					//Count the packet
			return true;
		}
	#endif
	transport[UDPTransportId].txStartTime = 0;
	return false;
}
bool treacleClass::UDPInitialised()
{
	if(UDPTransportId != 255)
	{
		return transport[UDPTransportId].initialised;
	}
	return false;
}
uint32_t treacleClass::getUDPRxPackets()
{
	if(UDPInitialised())
	{
		return transport[UDPTransportId].rxPackets;
	}
	return 0;
}
uint32_t treacleClass::getUDPTxPackets()
{
	if(UDPInitialised())
	{
		return transport[UDPTransportId].txPackets;
	}
	return 0;
}
uint32_t treacleClass::getUDPRxPacketsDropped()
{
	if(UDPInitialised())
	{
		return transport[UDPTransportId].rxPacketsDropped;
	}
	return 0;
}
uint32_t treacleClass::getUDPTxPacketsDropped()
{
	if(UDPInitialised())
	{
		return transport[UDPTransportId].txPacketsDropped;
	}
	return 0;
}
float treacleClass::getUDPDutyCycle()
{
	if(UDPInitialised())
	{
		return transport[UDPTransportId].calculatedDutyCycle;
	}
	return 0;
}
uint32_t treacleClass::getUDPDutyCycleExceptions()
{
	if(UDPInitialised())
	{
		return transport[UDPTransportId].dutyCycleExceptions;
	}
	return 0;
}
void treacleClass::setUDPTickInterval(uint16_t tick)
{
	transport[UDPTransportId].defaultTick = tick;
}
uint16_t treacleClass::getUDPTickInterval()
{
	if(UDPInitialised())
	{
		return transport[UDPTransportId].nextTick;
	}
	return 0;
}
#endif
#endif