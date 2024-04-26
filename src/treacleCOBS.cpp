/*
 *	An Arduino library for treacle support
 *
 *	https://github.com/ncmreynolds/treacle
 *
 *	Released under LGPL-2.1 see https://github.com/ncmreynolds/treacle/LICENSE for full license
 *
 */
#ifndef treacleCOBS_cpp
#define treacleCOBS_cpp
#include "treacle.h"
/*
 *
 *	COBS functions
 *
 */
#if defined(TREACLE_SUPPORT_COBS)
void treacleClass::enableCobs()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_enablingSpace);
		debugPrint(treacleDebugString_COBS);
	#endif
	cobsTransportId = numberOfActiveTransports++;
}
bool treacleClass::cobsEnabled()
{
	return cobsTransportId != 255;	//Not necessarily very useful, but it can be checked
}
bool treacleClass::cobsInitialised()
{
	if(cobsTransportId != 255)
	{
		return transport[cobsTransportId].initialised;
	}
	return false;
}
void treacleClass::setCobsStream(Stream &stream)
{
	cobsStream_ = &stream;
}
bool treacleClass::initialiseCobs()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_initialisingSpace);
		debugPrint(treacleDebugString_COBS);
		debugPrint(':');
	#endif
	if(cobsStream_ != nullptr)
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_OK);
		#endif
		transport[cobsTransportId].initialised = true;				//Mark as initialised
		transport[cobsTransportId].defaultTick = maximumTickTime/5;	//Set default tick timer
		return true;
	}
	#if defined(TREACLE_DEBUG)
		debugPrintln(treacleDebugString_failed);
	#endif
	return false;
}
bool treacleClass::sendBufferByCobs(uint8_t* buffer, uint8_t packetSize)
{
	transport[cobsTransportId].txStartTime = micros();
	cobsStream_->write(buffer, packetSize);
	transport[cobsTransportId].txTime += micros()		//Add to the total transmit time
		- transport[cobsTransportId].txStartTime;
	transport[cobsTransportId].txStartTime = 0;			//Clear the initial send time
	transport[cobsTransportId].txPackets++;				//Count the packet
	return true;
}
bool treacleClass::receiveCobs()
{
	if(cobsStream_->available())
	{
		if(receiveBufferSize == 0)
		{
			transport[cobsTransportId].rxPackets++;						//Count the packet as received
			if(cobsStream_->peek() == (uint8_t)nodeId::allNodes ||
				cobsStream_->peek() == currentNodeId)							//Packet is meaningful to this node
			{
				uint8_t receivedMessageLength = 0;
				uint32_t lastCharacter = millis();
				while(receivedMessageLength < 254 && millis() - lastCharacter < 100)	//Time out after 100ms
				{
					if(cobsStream_->available())
					{
						lastCharacter = millis();
						receiveBuffer[receivedMessageLength++] = cobsStream_->read();	//Copy the COBS payload
					}
				}
				receiveBufferSize = receivedMessageLength;				//Record the amount of payload
				receiveBufferCrcChecked = false;						//Mark the payload as unchecked
				receiveTransport = cobsTransportId;						//Record that it was received by ESP-Now
				transport[cobsTransportId].rxPacketsProcessed++;		//Count the packet as processed
				return true;
			}
		}
		else
		{
			transport[cobsTransportId].rxPacketsDropped++;				//Count the drop
		}
		while(cobsStream_->available())									//Drop the packet
		{
			cobsStream_->read();
		}
	}
	return false;
}
#endif
#endif