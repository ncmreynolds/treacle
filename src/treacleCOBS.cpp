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
	if(cobsTransportId != 255 && cobsStream_ != nullptr)
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_OK);
		#endif
		transport[cobsTransportId].initialised = true;					//Mark as initialised
		transport[cobsTransportId].defaultTick = maximumTickTime/5;		//Set default tick timer
		transport[cobsTransportId].minimumTick = maximumTickTime/20;	//Set minimum tick timer
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
	#if defined(TREACLE_DEBUG_COBS)
	Serial.print("\r\nSending COBS:-- ");
	for(uint8_t chunkIndex = 0; chunkIndex < packetSize; chunkIndex++)
	{
		Serial.printf("%02x ", buffer[chunkIndex]);
	}
	Serial.print("\r\nEncoded COBS:");
	#endif
	for(uint8_t index = 0; index < packetSize; index++)
	{
		lastZeroOffset++;
		if(buffer[index] == 0)	//It's a zero in the buffer, so record the offset from the last one
		{
			#if defined(TREACLE_DEBUG_COBS)
			Serial.printf("%02x ", lastZeroOffset);
			#endif
			cobsStream_->write(lastZeroOffset);	//Send the relative position of this zero
			if(lastZeroOffset > 1)	//Send the data between the previous zero (or the start) and this
			{
				#if defined(TREACLE_DEBUG_COBS)
				for(uint8_t chunkIndex = index-(lastZeroOffset-1); chunkIndex < index; chunkIndex++)
				{
					Serial.printf("%02x ", buffer[chunkIndex]);
				}
				#endif
				cobsStream_->write(&buffer[index-(lastZeroOffset-1)], lastZeroOffset - 1);
			}
			lastZeroOffset = 0;
		}
		else if(lastZeroOffset == packetSize && index == packetSize - 1) //Special case of no zeroes at all
		{
			#if defined(TREACLE_DEBUG_COBS)
			Serial.printf("%02x ", 0xff);
			for(uint8_t chunkIndex = 0; chunkIndex < packetSize; chunkIndex++)
			{
				Serial.printf("%02x ", buffer[chunkIndex]);
			}
			#endif
			cobsStream_->write((uint8_t)0xff);							//Send the 'no zeroes left in this chunk' marker
			cobsStream_->write(buffer, packetSize);						//Send the whole buffer
		}
		else if(index == packetSize - 1)								//Have reached the end of the packet
		{
			#if defined(TREACLE_DEBUG_COBS)
			Serial.printf("%02x ", 0xff);
			#endif
			cobsStream_->write((uint8_t)0xff);							//Send the 'no zeroes left in this chunk' marker
			if(lastZeroOffset > 1)	//Send the data between the previous zero (or the start) and this
			{
				#if defined(TREACLE_DEBUG_COBS)
				for(uint8_t chunkIndex = index-(lastZeroOffset-1); chunkIndex <= index; chunkIndex++)
				{
					Serial.printf("%02x ", buffer[chunkIndex]);
				}
				#endif
				cobsStream_->write(&buffer[index-(lastZeroOffset-1)], lastZeroOffset);
			}
			lastZeroOffset = 0;
		}
	}
	cobsStream_->write((uint8_t)0x00);					//Send the zero to mark the end of the packet
	#if defined(TREACLE_DEBUG_COBS)
	Serial.printf("%02x\r\n", 0x00);
	#endif
	transport[cobsTransportId].txTime += micros()		//Add to the total transmit time
		- transport[cobsTransportId].txStartTime;
	transport[cobsTransportId].txStartTime = 0;			//Clear the initial send time
	transport[cobsTransportId].txPackets++;				//Count the packet
	//debugPrintln("\r\nTX COBS packet");
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
			uint32_t lastCharacter = millis();									//Use to check for timeouts
			#if defined(TREACLE_DEBUG_COBS)
			Serial.print("\r\nReceivd COBS:");
			Serial.printf("%02x ", nextZero);
			#endif
			while(receiveBufferSize < maximumBufferSize && cobsStream_->peek() != 0 && millis() - lastCharacter < 100)	//Look for tailing zero or time out after 100ms in the event there isn't one
			{
				if(cobsStream_->available())
				{
					lastCharacter = millis();
					//receiveBuffer[receiveBufferSize++] = cobsStream_->read();		//Copy the COBS payload
					nextZero--;															//Decrement the zero position indicator
					if(nextZero == 0)													//At a zero in the sent buffer, before COBS encoding
					{
						nextZero = cobsStream_->read();									//Mark the next zero, which is inserted instead of the zero itself
						receiveBuffer[receiveBufferSize++] = 0;							//Rebuild the original payload, filling in the zero
						#if defined(TREACLE_DEBUG_COBS)
						Serial.printf("%02x ", nextZero);
						#endif
					}
					else
					{
						receiveBuffer[receiveBufferSize++] = cobsStream_->read();		//Copy the COBS payload
						#if defined(TREACLE_DEBUG_COBS)
						Serial.printf("%02x ", receiveBuffer[receiveBufferSize-1]);
						#endif
					}
				}
			}
			#if defined(TREACLE_DEBUG_COBS)
			if(cobsStream_->peek() == 0)
			{
				Serial.printf_P(PSTR("%02x"), cobsStream_->read());
			}
			else
			{
				Serial.print(F("timeout"));
			}
			Serial.print("\r\nDecoded COBS:-- ");
			for(uint8_t index = 0; index < receiveBufferSize; index++)
			{
				Serial.printf_P(PSTR("%02x "), receiveBuffer[index]);
			}
			#endif
			if(receiveBuffer[(uint8_t)headerPosition::recipient] == (uint8_t)nodeId::allNodes ||
				receiveBuffer[(uint8_t)headerPosition::recipient] == currentNodeId)		//Packet is meaningful to this node
			{
				receiveBufferCrcChecked = false;										//Mark the payload as unchecked
				receiveTransport = cobsTransportId;										//Record that it was received by COBS
				transport[cobsTransportId].rxPacketsProcessed++;						//Count the packet as processed
				#if defined(TREACLE_DEBUG_COBS)
				Serial.println();
				#endif
				return true;
			}
			else
			{
				#if defined(TREACLE_DEBUG_COBS)
				Serial.println(" IGNORED");
				#endif
				receiveBufferSize = 0;									//Mark the buffer as clear
				transport[cobsTransportId].rxPacketsIgnored++;			//Count the ignore
			}
		}
		else
		{
			transport[cobsTransportId].rxPacketsDropped++;				//Count the drop
			#if defined(TREACLE_DEBUG_COBS)
			Serial.println(" DROPPED");
			#endif
		}
		while(cobsStream_->available())									//Drop any remaining characters in the Stream
		{
			cobsStream_->read();
		}
	}
	return false;
}
void treacleClass::setCobsTickInterval(uint16_t tick)
{
	transport[cobsTransportId].defaultTick = tick;
	transport[cobsTransportId].minimumTick = tick/4;
}

#endif
#endif