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
	uint8_t lastZeroOffset = 0;
	transport[cobsTransportId].txStartTime = micros();
	/*
	debugPrintln();
	debugPrint("Starting COBS packet:");
	for(uint8_t chunkIndex = 0; chunkIndex < packetSize; chunkIndex++)
	{
		Serial.printf("%02x ", buffer[chunkIndex]);
	}
	debugPrintln();
	*/
	for(uint8_t index = 0; index < packetSize; index++)
	{
		lastZeroOffset++;
		if(buffer[index] == 0)	//It's a zero in the buffer, so record the offset from the last one
		{
			/*
			debugPrint("Chunk:");
			for(uint8_t chunkIndex = index-(lastZeroOffset-1); chunkIndex <= index; chunkIndex++)
			{
				Serial.printf("%02x ", buffer[chunkIndex]);
			}
			debugPrintln();
			debugPrint("Becomes:");
			debugPrint(lastZeroOffset);
			debugPrint(' ');
			*/
			if(lastZeroOffset > 1)	//Send the data between the previous zero (or the start) and this
			{
				/*
				for(uint8_t chunkIndex = index-(lastZeroOffset-1); chunkIndex < index; chunkIndex++)
				{
					Serial.printf("%02x ", buffer[chunkIndex]);
				}
				*/
				cobsStream_->write(&buffer[index-(lastZeroOffset-1)], lastZeroOffset - 1);
			}
			cobsStream_->write(lastZeroOffset);	//Send the relative position of this zero
			lastZeroOffset = 0;
			//debugPrintln();
		}
		else if(lastZeroOffset == packetSize && index == packetSize - 1) //Special case of no zeroes at all
		{
			/*
			debugPrint("No zeroes COBS packet: ");
			for(uint8_t chunkIndex = 0; chunkIndex < packetSize; chunkIndex++)
			{
				Serial.printf("%02x ", buffer[chunkIndex]);
			}
			debugPrintln();
			*/
			cobsStream_->write((uint8_t)0xff);							//Send the 'no zeroes left in this chunk' marker
			cobsStream_->write(buffer, packetSize);						//Send the whole buffer
		}
		else if(index == packetSize - 1)								//Have reached the end of the packet
		{
			/*
			debugPrint("Chunk:");
			for(uint8_t chunkIndex = index-(lastZeroOffset-1); chunkIndex <= index; chunkIndex++)
			{
				Serial.printf("%02x ", buffer[chunkIndex]);
			}
			debugPrintln();
			debugPrint("Becomes:");
			debugPrint((uint8_t)0xff);
			debugPrint(' ');
			*/
			if(lastZeroOffset > 1)	//Send the data between the previous zero (or the start) and this
			{
				/*
				for(uint8_t chunkIndex = index-(lastZeroOffset-1); chunkIndex <= index; chunkIndex++)
				{
					Serial.printf("%02x ", buffer[chunkIndex]);
				}
				*/
				cobsStream_->write(&buffer[index-(lastZeroOffset-1)], lastZeroOffset - 1);
			}
			cobsStream_->write(lastZeroOffset);	//Send the relative position of this zero
			lastZeroOffset = 0;
			//debugPrintln();
		}
	}
	//debugPrintln("Ending COBS packet");
	cobsStream_->write((uint8_t)0x00);					//Send the zero to mark the end of the packet
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
			transport[cobsTransportId].rxPackets++;								//Count the packet as received
			uint8_t nextZero = cobsStream_->read();								//Mark the next zero, which SHOULD be the first transmitted value
			if(cobsStream_->peek() == (uint8_t)nodeId::allNodes ||
				cobsStream_->peek() == currentNodeId)							//Packet is meaningful to this node
			{
				uint32_t lastCharacter = millis();
				while(receiveBufferSize < maximumBufferSize && cobsStream_->peek() != 0 && millis() - lastCharacter < 100)	//Look for tailing zero or time out after 100ms in the event there isn't one
				{
					if(cobsStream_->available())
					{
						lastCharacter = millis();
						if(--nextZero == 0)													//At a zero in the sent buffer, before COMS encoding
						{
							nextZero = cobsStream_->read();									//Mark the next zero, which is inserted instead of the zero itself
							receiveBuffer[receiveBufferSize++] = 0;							//Rebuild the original payload, filling in the zero
						}
						else
						{
							receiveBuffer[receiveBufferSize++] = cobsStream_->read();		//Copy the COBS payload
						}
					}
				}
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