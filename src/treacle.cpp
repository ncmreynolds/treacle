/*
 *	An Arduino library for treacle support
 *
 *	https://github.com/ncmreynolds/treacle
 *
 *	Released under LGPL-2.1 see https://github.com/ncmreynolds/treacle/LICENSE for full license
 *
 */
#ifndef treacle_cpp
#define treacle_cpp
#include "treacle.h"


treacleClass::treacleClass()	//Constructor function
{
}

treacleClass::~treacleClass()	//Destructor function
{
}

void treacleClass::setNodeName(char* name)
{
	if(name != nullptr)
	{
		if(currentNodeName != nullptr)
		{
			delete currentNodeName;
		}
		currentNodeName = new char[strlen(name) + 1];
		strlcpy(currentNodeName, name, strlen(name) + 1);
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_node_name);
			debugPrint(':');
			debugPrintStringln(currentNodeName);
		#endif
	}
}
void treacleClass::setNodeId(uint8_t id)
{
	if(id >= minimumNodeId && id <= maximumNodeId)
	{
		currentNodeId = id;
	}
}
uint8_t treacleClass::getNodeId()
{
	return currentNodeId;
}
bool treacleClass::begin(uint8_t maxNodes)
{
	//The maximum number of nodes is used in creating a load of data structures
	maximumNumberOfNodes = maxNodes;
	node = new nodeInfo[maximumNumberOfNodes];	//Assign at start
	//The name is important so assign one if it is not set. This is based off MAC address on ESP8266/ESP32
	if(currentNodeName == nullptr)
	{
		#if defined(TREACLE_SUPPORT_ESPNOW) && defined(TREACLE_SUPPORT_LORA) && defined(TREACLE_SUPPORT_COBS)
			if(currentNodeName == nullptr && espNowEnabled() && loRaEnabled() && cobsEnabled())
			{
				currentNodeName = new char[34];
				sprintf_P(currentNodeName, PSTR("EspNow_LoRa_COBS_%02X%02X%02X%02X%02X%02X%02X%02X"),UniqueID8[0] ,UniqueID8[1] ,UniqueID8[2] ,UniqueID8[3] ,UniqueID8[4] ,UniqueID8[5] ,UniqueID8[6],UniqueID8[7]);  //Add the UniqueID to a standard name
			}
		#endif
		#if defined(TREACLE_SUPPORT_ESPNOW) && defined(TREACLE_SUPPORT_LORA)
			if(currentNodeName == nullptr && espNowEnabled() && loRaEnabled())
			{
				currentNodeName = new char[29];
				sprintf_P(currentNodeName, PSTR("EspNow_LoRa_%02X%02X%02X%02X%02X%02X%02X%02X"),UniqueID8[0] ,UniqueID8[1] ,UniqueID8[2] ,UniqueID8[3] ,UniqueID8[4] ,UniqueID8[5] ,UniqueID8[6],UniqueID8[7]);  //Add the UniqueID to a standard name
			}
		#endif
		#if defined(TREACLE_SUPPORT_ESPNOW) && defined(TREACLE_SUPPORT_COBS)
			if(currentNodeName == nullptr && espNowEnabled()&& cobsEnabled())
			{
				currentNodeName = new char[29];
				sprintf_P(currentNodeName, PSTR("EspNow_COBS_%02X%02X%02X%02X%02X%02X%02X%02X"),UniqueID8[0] ,UniqueID8[1] ,UniqueID8[2] ,UniqueID8[3] ,UniqueID8[4] ,UniqueID8[5] ,UniqueID8[6],UniqueID8[7]);  //Add the UniqueID to a standard name
			}
		#endif
		#if defined(TREACLE_SUPPORT_LORA) && defined(TREACLE_SUPPORT_COBS)
			if(currentNodeName == nullptr && loRaEnabled() && cobsEnabled())
			{
				currentNodeName = new char[27];
				sprintf_P(currentNodeName, PSTR("LoRa_COBS_%02X%02X%02X%02X%02X%02X%02X%02X"),UniqueID8[0] ,UniqueID8[1] ,UniqueID8[2] ,UniqueID8[3] ,UniqueID8[4] ,UniqueID8[5] ,UniqueID8[6],UniqueID8[7]);  //Add the UniqueID to a standard name
			}
		#endif
		#if defined(TREACLE_SUPPORT_ESPNOW)
			if(currentNodeName == nullptr && espNowEnabled())
			{
				currentNodeName = new char[24];
				sprintf_P(currentNodeName, PSTR("EspNow_%02X%02X%02X%02X%02X%02X%02X%02X"),UniqueID8[0] ,UniqueID8[1] ,UniqueID8[2] ,UniqueID8[3] ,UniqueID8[4] ,UniqueID8[5] ,UniqueID8[6],UniqueID8[7]);  //Add the UniqueID to a standard name
			}
		#endif
		#if defined(TREACLE_SUPPORT_LORA)
			if(currentNodeName == nullptr && loRaEnabled())
			{
				currentNodeName = new char[22];
				sprintf_P(currentNodeName, PSTR("LoRa_%02X%02X%02X%02X%02X%02X%02X%02X"),UniqueID8[0] ,UniqueID8[1] ,UniqueID8[2] ,UniqueID8[3] ,UniqueID8[4] ,UniqueID8[5] ,UniqueID8[6],UniqueID8[7]);  //Add the UniqueID to a standard name
			}
		#endif
		#if defined(TREACLE_SUPPORT_COBS)
			if(currentNodeName == nullptr && cobsEnabled())
			{
				currentNodeName = new char[22];
				sprintf_P(currentNodeName, PSTR("COBS_%02X%02X%02X%02X%02X%02X%02X%02X"),UniqueID8[0] ,UniqueID8[1] ,UniqueID8[2] ,UniqueID8[3] ,UniqueID8[4] ,UniqueID8[5] ,UniqueID8[6],UniqueID8[7]);  //Add the UniqueID to a standard name
			}
		#endif
		if(currentNodeName == nullptr)
		{
			currentNodeName = new char[22];
			sprintf_P(currentNodeName, PSTR("node_%02X%02X%02X%02X%02X%02X%02X%02X"),UniqueID8[0] ,UniqueID8[1] ,UniqueID8[2] ,UniqueID8[3] ,UniqueID8[4] ,UniqueID8[5] ,UniqueID8[6],UniqueID8[7]);  //Add the UniqueID to a standard name
		}
	}
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_node_name);
		debugPrint(':');
		debugPrintln(currentNodeName);
		debugPrint(treacleDebugString_treacleSpace);
		debugPrintln(treacleDebugString_starting);
	#endif
	changeCurrentState(state::starting);
	if(numberOfActiveTransports > 0)
	{
		transport = new transportData[numberOfActiveTransports];
		//Initialise all the transports
		uint8_t numberOfInitialisedTransports = 0;
		for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)	//Initialise every transport that is enabled
		{
			#if defined(TREACLE_SUPPORT_ESPNOW)
				if(transportIndex == espNowTransportId)
				{
					transport[transportIndex].initialised = initialiseEspNow();
				}
			#endif
			#if defined(TREACLE_SUPPORT_LORA)
				if(transportIndex == loRaTransportId)
				{
					transport[transportIndex].initialised = initialiseLoRa();
					if(transport[transportIndex].initialised)
					{
						rssi = new int16_t[maximumNumberOfNodes];	//Storage for RSSI values
						snr = new float[maximumNumberOfNodes];		//Storage for RSSI values
						for(uint8_t index = 0; index < maximumNumberOfNodes; index++)
						{
							rssi[index] = 0;
							snr[index] = 0;
						}
					}
				}
			#endif
			#if defined(TREACLE_SUPPORT_MQTT)
				if(transportIndex == MQTTTransportId)
				{
					transport[transportIndex].initialised = initialiseMQTT();
				}
			#endif
			#if defined(TREACLE_SUPPORT_UDP)
				if(transportIndex == UDPTransportId)
				{
					transport[transportIndex].initialised = initialiseUDP();
				}
			#endif
			#if defined(TREACLE_SUPPORT_COBS)
				if(transportIndex == cobsTransportId)
				{
					transport[transportIndex].initialised = initialiseCobs();
				}
			#endif
			if(transport[transportIndex].initialised == true)
			{
				numberOfInitialisedTransports++;
			}
			if(encryptionKey != nullptr)
			{
				transport[transportIndex].encrypted = true;	//Default to encrypted if a key is set
			}
		}
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_start);
			debugPrint(':');
		#endif
		if(numberOfInitialisedTransports == numberOfActiveTransports)
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_OK);
				debugPrint(treacleDebugString_treacleSpace);
				debugPrint(treacleDebugString_ActiveSpaceTransports);
				debugPrint(':');
				debugPrintln(numberOfInitialisedTransports);
				for(uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
				{
					debugPrint(treacleDebugString_treacleSpace);
					debugPrintTransportName(transportId);
					debugPrint(treacleDebugString_SpaceTransportID);
					debugPrint(':');
					debugPrintln(transportId);
				}
			#endif
			if(currentNodeId == 0)
			{
				changeCurrentState(state::selectingId);
			}
			else
			{
				changeCurrentState(state::selectedId);
			}
			setNextTickTime();	//Set initial tick times
			return true;
		}
	}
	#if defined(TREACLE_DEBUG)
		debugPrintln(treacleDebugString_failed);
	#endif
	changeCurrentState(state::stopped);
	return false;
}
void treacleClass::end()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrintln(treacleDebugString_ended);
	#endif
}
void treacleClass::enableDebug(Stream &debugStream)
{
	#if defined(TREACLE_DEBUG)
		debug_uart_ = &debugStream;				//Set the stream used for debug
		#if defined(ESP8266)
			if(&debugStream == &Serial)
			{
				  debug_uart_->write(17);			//Send an XON to stop the hung terminal after reset on ESP8266, which often emits an XOFF
			}
		#endif
	#endif
}
void treacleClass::disableDebug()
{
	#if defined(TREACLE_DEBUG)
		debug_uart_ = nullptr;
	#endif
}
/*
 *
 *	Transport abstraction/genericisation helpers
 *
 */
bool treacleClass::packetInQueue()
{
	for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)
	{
		if(transport[transportIndex].bufferSent == false)			//At least one transport has not sent the buffer
		{
			return true;
		}
	}
	return false;
}
bool treacleClass::packetInQueue(uint8_t transportId)
{
	if(transport[transportId].bufferSent == false)		//Transport has not sent the buffer
	{
		return true;
	}
	return false;
}
bool treacleClass::sendBuffer(uint8_t transportId, uint8_t* buffer, uint8_t packetSize)
{
	#if defined(TREACLE_SUPPORT_ESPNOW)
		if(transportId == espNowTransportId)
		{
			return sendBufferByEspNow(buffer, packetSize);
		}
	#endif
	#if defined(TREACLE_SUPPORT_LORA)
		if(transportId == loRaTransportId)
		{
			return sendBufferByLoRa(buffer, packetSize);
		}
	#endif
	#if defined(TREACLE_SUPPORT_MQTT)
		if(transportId == MQTTTransportId)
		{
			return sendBufferByMQTT(buffer, packetSize);
		}
	#endif
	#if defined(TREACLE_SUPPORT_UDP)
		if(transportId == UDPTransportId)
		{
			return sendBufferByUDP(buffer, packetSize);
		}
	#endif
	#if defined(TREACLE_SUPPORT_COBS)
		if(transportId == cobsTransportId)
		{
			return sendBufferByCobs(buffer, packetSize);
		}
	#endif
	return false;
}
/*
 *
 *	Generic transport functions
 *
 */
void treacleClass::enableEncryption(uint8_t transportId)				//Enable encryption for a specific transport
{
	if(transport != nullptr && transportId != 255)
	{
		transport[transportId].encrypted = true;
	}
}
void treacleClass::disableEncryption(uint8_t transportId)				//Disable encryption for a specific transport
{
	if(transport != nullptr && transportId != 255)
	{
		transport[transportId].encrypted = false;
	}
}
/*
 *
 * ID management
 *
 */
bool treacleClass::selectNodeId()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_selectingSpace);
		debugPrint(treacleDebugString_nodeId);
		debugPrint(':');
	#endif
	bool idIsUnique = false;
	while(idIsUnique == false)
	{
		currentNodeId = random(minimumNodeId,maximumNodeId);
		idIsUnique = (nodeExists(currentNodeId) == false);
	}
	if(idIsUnique == true)
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(currentNodeId);
		#endif
		return true;
	}
	#if defined(TREACLE_DEBUG)
		debugPrintln(treacleDebugString_failed);
	#endif
	return false;
}
/*
 *
 *	General packet handling
 *
 */
bool treacleClass::processPacketBeforeTransmission(uint8_t transportId)
{
	if(appendChecksumToPacket(transport[transportId].transmitBuffer, transport[transportId].transmitPacketSize))		//Append checksum after making the packet, but do not increment the packetLength field
	{
		if(transport[transportId].encrypted == true)
		{
			return encryptPayload(transport[transportId].transmitBuffer, transport[transportId].transmitPacketSize);	//Encrypt the payload but not the header
		}
		return true;
	}
	return false;
}
/*
 *
 *	Encryption
 *
 */
void treacleClass::setEncryptionKey(uint8_t* key)	//Set the encryption key
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_addingSpace);
		debugPrintln(treacleDebugString_encryption_key);
	#endif
	encryptionKey = key;
	#if defined(TREACLE_ENCRYPT_WITH_CBC)
		#if defined(ESP32)
			esp_aes_init(&context);							//Initialise the AES context
			esp_aes_setkey(&context, encryptionKey, 128);	//Set the key
		#endif
	#endif
}
bool treacleClass::encryptPayload(uint8_t* buffer, uint8_t& packetSize)	//Pad the buffer if necessary and encrypt the payload
{
	uint8_t padding = (encryptionBlockSize - (packetSize - (uint8_t)headerPosition::blockIndex)%encryptionBlockSize)%encryptionBlockSize;
	while((packetSize - (uint8_t)headerPosition::blockIndex)%encryptionBlockSize !=0 && packetSize < maximumBufferSize)
	{
		buffer[packetSize++] = padding;	//Pad the packet PKC5 style
	}
	if(padding > 0)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_padded_by);
			debugPrint(' ');
			debugPrint(padding);
			debugPrint(' ');
			debugPrint(treacleDebugString_toSpace);
			debugPrint(packetSize);
			debugPrint(' ');
			debugPrint(treacleDebugString_bytes);
			debugPrint(' ');
		#endif
	}
	#if defined(TREACLE_ENCRYPT_WITH_CBC) || defined(TREACLE_ENCRYPT_WITH_EAX)
		uint8_t encryptedData[packetSize - (uint8_t)headerPosition::blockIndex];		//Temporary storage for the encryted data
		uint8_t initialisationVector[16];												//Allocate an initialisation vector
		memcpy(&initialisationVector[0],  buffer, 4);									//Use the first four bytes of the packet, repeated for the initialisation vector
		memcpy(&initialisationVector[4],  buffer, 4);
		memcpy(&initialisationVector[8],  buffer, 4);
		memcpy(&initialisationVector[12], buffer, 4);
		#if defined(TREACLE_ENCRYPT_WITH_CBC)
			#if defined(ESP32)
				esp_aes_crypt_cbc(&context, ESP_AES_ENCRYPT,									//Do the encryption
					packetSize - (uint8_t)headerPosition::blockIndex,							//Length of the data to encrypt
					initialisationVector,														//Initialisation vector
					&buffer[(uint8_t)headerPosition::blockIndex],								//The point to start encrypting from
					encryptedData);																//Destination for the encrypted version
			#else
				CBC<AES128> cbc;
				cbc.setKey(encryptionKey, 16);
				cbc.setIV(initialisationVector, 16);											//Initialisation vector
				cbc.encrypt(																	//Do the encryption
					encryptedData,																//Destination for the encrypted version
					&buffer[(uint8_t)headerPosition::blockIndex],								//The point to start encrypting from
					packetSize - (uint8_t)headerPosition::blockIndex);							//Length of the data to encrypt
			#endif
		#endif
		#if defined(TREACLE_ENCRYPT_WITH_EAX)
			EAX<AES128> eax;
			eax.setKey(encryptionKey, 16);														//Set the encryption key
			eax.setIV(initialisationVector, 16);												//Set the initialisation vector
			eax.addAuthData(adata, sizeof(adata));												//Set the authentication data
			eax.encrypt(																		//Do the encryption
				encryptedData,																	//Destination for the encrypted version
				&buffer[(uint8_t)headerPosition::blockIndex],									//The point to start encrypting from
				packetSize - (uint8_t)headerPosition::blockIndex);								//Length of the data to encrypt
			eax.computeTag(tag, sizeof(tag));
		#endif
		memcpy(&buffer[(uint8_t)headerPosition::blockIndex], encryptedData, packetSize - (uint8_t)headerPosition::blockIndex); //Copy the encrypted version back over the buffer
	#else
		for(uint8_t bufferIndex = (uint8_t)headerPosition::blockIndex; bufferIndex < packetSize; bufferIndex++)
		{
			buffer[bufferIndex] = buffer[bufferIndex] ^
				encryptionKey[bufferIndex%encryptionBlockSize];							//This is obfuscation only, for testing.
		}
	#endif
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_encrypted);
		debugPrint(' ');
	#endif
	buffer[(uint8_t)headerPosition::payloadType] = buffer[(uint8_t)headerPosition::payloadType] | (uint8_t)payloadType::encrypted;	//Mark as encrypted
	return true;
}
	
bool treacleClass::decryptPayload(uint8_t* buffer, uint8_t& packetSize)	//Decrypt the payload and remove the padding, if necessary
{
	if(packetSize != buffer[(uint8_t)headerPosition::packetLength] + 2)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_padded_by);
			debugPrint(' ');
			debugPrint(packetSize - (buffer[(uint8_t)headerPosition::packetLength] + 2));
			debugPrint(' ');
			debugPrint(treacleDebugString_bytes);
			debugPrint(' ');
		#endif
	}
	//if(packetSize >= (uint8_t)headerPosition::blockIndex && packetSize <= maximumBufferSize - 2)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_decrypted);
			debugPrint(' ');
		#endif
		buffer[(uint8_t)headerPosition::payloadType] = buffer[(uint8_t)headerPosition::payloadType] & (0xff ^ (uint8_t)payloadType::encrypted);	//Mark as not encrypted, otherwise the IV and CRC is invalid
		#if defined(TREACLE_ENCRYPT_WITH_CBC)
			uint8_t initialisationVector[16];												//Allocate an initialisation vector
			memcpy(&initialisationVector[0],  buffer, 4);									//Use the first four bytes of the packet, repeated for the initialisation vector
			memcpy(&initialisationVector[4],  buffer, 4);
			memcpy(&initialisationVector[8],  buffer, 4);
			memcpy(&initialisationVector[12], buffer, 4);
			uint8_t decryptedData[packetSize - (uint8_t)headerPosition::blockIndex];		//Temporary storage for the decryted data
			#if defined(TREACLE_ENCRYPT_WITH_CBC)
				#if defined(ESP32)
					esp_aes_crypt_cbc(&context, ESP_AES_DECRYPT,								//Do the decryption
						packetSize - (uint8_t)headerPosition::blockIndex,						//Length of the data to encrypt
						initialisationVector,													//Initialisation vector
						&buffer[(uint8_t)headerPosition::blockIndex],							//The point to start encrypting from
						decryptedData);															//Destination for the encrypted version
				#else
					CBC<AES128> cbc;
					cbc.setKey(encryptionKey, 16);												//Encryption key
					cbc.setIV(initialisationVector, 16);										//Initialisation vector
					cbc.decrypt(																//Do the encryption
						decryptedData,															//Destination for the encrypted version
						&buffer[(uint8_t)headerPosition::blockIndex],							//The point to start encrypting from
						packetSize - (uint8_t)headerPosition::blockIndex);						//Length of the data to encrypt
				#endif
			#endif
			memcpy(&buffer[(uint8_t)headerPosition::blockIndex], decryptedData, packetSize - (uint8_t)headerPosition::blockIndex); //Copy the decrypted version back over the buffer
		#else
			for(uint8_t bufferIndex = (uint8_t)headerPosition::blockIndex; bufferIndex < packetSize; bufferIndex++)
			{
				buffer[bufferIndex] = buffer[bufferIndex] ^
					encryptionKey[bufferIndex%encryptionBlockSize];							//This is obfuscation only, for testing.
			}
		#endif
		return true;
	}
	return false;
}
	
/*
 *
 *	Checksums
 *
 */
bool treacleClass::appendChecksumToPacket(uint8_t* buffer, uint8_t& packetSize)
{
	if(packetSize < maximumBufferSize - 2)	//The CRC will expand the packet by two bytes
	{
		CRC16 crc(treaclePolynome);
		crc.add((uint8_t *)buffer, packetSize);
		uint16_t packetChecksum = crc.calc();
		buffer[packetSize++] = (packetChecksum & 0xff00) >> 8;	//Append this to the packet
		buffer[packetSize++] = packetChecksum & 0xff;			//Packet size is also increased by 2 for the CRC, but the length in the header DOES NOT include the CRC!
		return true;
	}
	return false;
}
bool treacleClass::validatePacketChecksum(uint8_t* buffer, uint8_t& packetSize)
{
	CRC16 crc(treaclePolynome);
	crc.add((uint8_t *)buffer, buffer[(uint8_t)headerPosition::packetLength]);
	uint16_t expectedChecksum = crc.calc();
	uint8_t checksumPosition = buffer[(uint8_t)headerPosition::packetLength];	//The CRC is added at the end of the packet and is not included in the size in the header field
	uint16_t packetChecksum = (buffer[checksumPosition] << 8) + buffer[checksumPosition + 1];
	if(expectedChecksum == packetChecksum)
	{
		packetSize -= 2;	//Checksum can now be ignored so buffer size is reduced by 2
		return true;
	}
	return false;
}
/*
 *
 *	State tracking
 *
 */
void treacleClass::changeCurrentState(state newState)
{
	if(currentState != newState)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_newSpaceState);
			debugPrint(':');
			debugPrintState(newState);
			debugPrint(' ');
			debugPrint(treacleDebugString_after);
			debugPrint(' ');
			debugPrint((millis()-lastStateChange)/60E3);
			debugPrint(' ');
			debugPrintln(treacleDebugString_minutes);
		#endif
		currentState = newState;
		lastStateChange = millis();
	}
}
/*
 *
 *	Tick timers
 *
 */
void treacleClass::setNextTickTime()
{
	for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)
	{
		setNextTickTime(transportIndex);
	}
}
void treacleClass::setNextTickTime(uint8_t transportId)
{
	transport[transportId].nextTick = transport[transportId].defaultTick - tickRandomisation(transportId);
}
uint16_t treacleClass::tickRandomisation(uint8_t transportId)
{
	return random(transport[transportId].minimumTick, transport[transportId].minimumTick*2);
}
void treacleClass::bringForwardNextTick()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_expediting_);
		debugPrintln(treacleDebugString_response);
	#endif
	for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)
	{
		if(transport[transportIndex].nextTick != 0)
		{
			transport[transportIndex].lastTick = millis() - (transport[transportIndex].nextTick + tickRandomisation(transportIndex));
		}
	}
}
bool treacleClass::sendPacketOnTick()
{
	for(uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
	{
		if(transport[transportId].nextTick != 0 && millis() - transport[transportId].lastTick > transport[transportId].nextTick)	//nextTick = 0 implies never
		{
			transport[transportId].lastTick = millis();									//Update the last tick time
			calculateDutyCycle(transportId);
			#if defined(TREACLE_DEBUG)
				debugPrint(treacleDebugString_treacleSpace);
				debugPrintTransportName(transportId);
				debugPrint(' ');
			#endif
			if(transport[transportId].calculatedDutyCycle < transport[transportId].maximumDutyCycle)
			{
				if(packetInQueue(transportId) == false)									//Nothing ready to send from the application for _this_ transport
				{
					if(currentState == state::selectingId)								//Speed up ID selection by asking existing node IDs
					{
						buildIdResolutionRequestPacket(transportId, currentNodeName);	//Ask about this node with a name->Id request
						#if defined(TREACLE_DEBUG)
							debugPrint(treacleDebugString_idResolutionRequest);
						#endif
					}
					else
					{
						//if(packetInQueue() == false && transport[transportId].payloadNumber%4 == 0)	//If nothing else is queued to transmit over any transport, 25% of packets can be name requests to backfill names
						if(packetInQueue() == false)	//If nothing else is queued to transmit over any transport backfill names
						{
							for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
							{
								if(node[nodeIndex].name == nullptr && online(node[nodeIndex].id) && packetInQueue() == false)		//This node has no name yet
								{
									buildNameResolutionRequestPacket(transportId, node[nodeIndex].id);	//Ask for the node's name with an id->name request
									#if defined(TREACLE_DEBUG)
										debugPrint(treacleDebugString_nameResolutionRequest);
									#endif
								}
							}
						}
						if(packetInQueue(transportId) == false)				//Nothing else is queued to transmit over _this_ transport, keepalives can go over a higher priority transport with something in a lower one queued
						{
							buildKeepalivePacket(transportId);				//Create an empty tick as a keepalive and announcement of all known nodeIds
							#if defined(TREACLE_DEBUG)
								debugPrint(treacleDebugString_keepalive);
							#endif
						}
					}
				}
				else	//There is already data which may or many not already have been sent by other transports
				{
					#if defined(TREACLE_DEBUG)
						debugPrintPayloadTypeDescription(transport[transportId].transmitBuffer[(uint8_t)headerPosition::payloadType]);
					#endif
				}
				#if defined(TREACLE_DEBUG)
					debugPrint(':');
				#endif
				if(sendBuffer(transportId, transport[transportId].transmitBuffer, transport[transportId].transmitPacketSize))
				{
					#if defined(TREACLE_DEBUG)
						debugPrint(transport[transportId].transmitPacketSize);
						debugPrint(' ');
						debugPrint(treacleDebugString_bytes);
						debugPrint(' ');
						debugPrint(treacleDebugString_sent);
						debugPrint(' ');
						debugPrint(treacleDebugString_toSpace);
						debugPrint(treacleDebugString_nodeId);
						debugPrint(':');
						if(transport[transportId].transmitBuffer[0] == (uint8_t)nodeId::allNodes)
						{
							debugPrintln(treacleDebugString_all);
						}
						else if(transport[transportId].transmitBuffer[0] == (uint8_t)nodeId::unknownNode)
						{
							debugPrintln(treacleDebugString_unknown);
						}
						else
						{
							debugPrintln(transport[transportId].transmitBuffer[0]);
						}
					#endif
					transport[transportId].bufferSent = true;
					return true;
				}
				else
				{
					#if defined(TREACLE_DEBUG)
						debugPrintln(treacleDebugString_failed);
					#endif
					return false;
				}
			}
			else
			{
				transport[transportId].dutyCycleExceptions++;
				#if defined(TREACLE_DEBUG)
					debugPrint(treacleDebugString_duty_cycle_exceeded);
					debugPrint(':');
					debugPrint(transport[transportId].calculatedDutyCycle);
					debugPrintln('%');
				#endif
			}
		}
	}
	return false;
}
void treacleClass::timeOutTicks()
{
	uint16_t totalTxReliability = 0x0000;
	uint16_t totalRxReliability = 0x0000;
	for(uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
	{
		for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
		{
			if(millis() - node[nodeIndex].lastTick[transportId] > node[nodeIndex].nextTick[transportId] + transport[transportId].minimumTick	//Missed the next window
				&& node[nodeIndex].rxReliability[transportId] > 0																				//Actually has some reliability to begin with
				)
			{
				node[nodeIndex].rxReliability[transportId] = node[nodeIndex].rxReliability[transportId] >> 1;	//Reduce rxReliability
				node[nodeIndex].txReliability[transportId] = node[nodeIndex].txReliability[transportId] >> 1;	//As we've not heard anything to the contrary also reduce txReliability
				node[nodeIndex].lastTick[transportId] = millis();										//Update last tick timer, even though one was missed
				#if defined(TREACLE_DEBUG)
					debugPrint(treacleDebugString_treacleSpace);
					debugPrintTransportName(transportId);
					debugPrint(' ');
					debugPrint(treacleDebugString_nodeId);
					debugPrint(':');
					debugPrint(node[nodeIndex].id);
					debugPrint(' ');
					debugPrint(treacleDebugString_dropped);
					debugPrint(' ');
					debugPrint(treacleDebugString_rxReliability);
					debugPrint(':');
					debugPrint(reliabilityPercentage(node[nodeIndex].rxReliability[transportId]));
					debugPrintln('%');
				#endif
			}
			totalTxReliability = totalTxReliability | node[nodeIndex].txReliability[transportId];		//OR all the bits of transmit reliability we have
			totalRxReliability = totalRxReliability | node[nodeIndex].rxReliability[transportId];		//OR all the bits of receive reliability we have
		}
	}
	if((totalRxReliability == 0x0000 || totalTxReliability == 0x0000) && currentState == state::online)
	{
		changeCurrentState(state::offline);
	}
	else if((countBits(totalRxReliability) > 2 && countBits(totalTxReliability) > 2) && (currentState == state::selectedId || currentState == state::offline))
	{
		changeCurrentState(state::online);
	}
}

/*
 *
 *	Packet packing
 *
 */
void treacleClass::buildPacketHeader(uint8_t transportId, uint8_t recipient, payloadType type)
{
	setNextTickTime(transportId);																								//Set the next tick time for this packet
	if(recipient == (uint8_t)nodeId::unknownNode)
	{
		bringForwardNextTick();																									//Bring forward the next tick ASAP for any starting nodes
	}
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::recipient] = recipient;										//Add the recipient Id
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::sender] = currentNodeId;										//Add the current nodeId
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::payloadType] = (uint8_t)type;								//Payload type
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::payloadNumber] = transport[transportId].payloadNumber++;		//Payload number, which post-increments
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::packetLength] = 0;											//Payload length - starts at 0 and gets updated later
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::blockIndex] = random(0,256);									//Large payload start bits 16-23, by default just randomised
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::blockIndex+1] = random(0,256);								//Large payload start bits 8-15
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::blockIndex+2] = random(0,256);								//Large payload start bits 0-7
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::nextTick] = (transport[transportId].nextTick & 0xff00) >> 8;	//nextTick bits 8-15
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::nextTick+1] = (transport[transportId].nextTick & 0x00ff);	//nextTick bits 0-7
	//
	transport[transportId].transmitPacketSize = (uint8_t)headerPosition::payload;												//Set the size to just the header
	transport[transportId].bufferSent = false;																					//Mark as unsent for this transport
}
void treacleClass::buildKeepalivePacket(uint8_t transportId)
{
	buildPacketHeader(transportId, (uint8_t)nodeId::allNodes, payloadType::keepalive);						//Set payloadType
	for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)									//Add all nodes with a known name
	{
		if(node[nodeIndex].name != nullptr && node[nodeIndex].rxReliability > 0)					//Include nodes with names that have non-zero receive history
		{
			transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize++] = node[nodeIndex].id;							//Include node ID
			transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize++] =											//Include node RX reliability MSB
				(uint8_t)((node[nodeIndex].rxReliability[transportId]&0xff00)>>8);
			transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize++] =											//Include node RX reliability LSB
				(uint8_t)(node[nodeIndex].rxReliability[transportId]&0x00ff);
			
		}
	}
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::packetLength] = transport[transportId].transmitPacketSize;				//Update packetLength field
	processPacketBeforeTransmission(transportId);																							//Do CRC and encryption if needed
}
void treacleClass::buildIdResolutionRequestPacket(uint8_t transportId, char* name)				//Ask for a ID from a name
{
	buildPacketHeader(transportId, (uint8_t)nodeId::allNodes, payloadType::idResolutionRequest);			//Set payloadType
	transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize++] = strlen(name);
	memcpy(&transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize], name, strlen(name));
	transport[transportId].transmitPacketSize += strlen(name);
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::packetLength] = transport[transportId].transmitPacketSize;						//Update packetLength field
	processPacketBeforeTransmission(transportId);																								//Do CRC and encryption if needed
}
void treacleClass::buildNameResolutionRequestPacket(uint8_t transportId, uint8_t id)			//Ask for a name from an ID
{
	buildPacketHeader(transportId, (uint8_t)nodeId::allNodes, payloadType::nameResolutionRequest);			//Set payloadType
	transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize++] = id;
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::packetLength] = transport[transportId].transmitPacketSize;						//Update packetLength field
	processPacketBeforeTransmission(transportId);																								//Do CRC and encryption if needed
}
void treacleClass::buildIdAndNameResolutionResponsePacket(uint8_t transportId, uint8_t id, uint8_t senderId)		//Supply ID and name
{
	buildPacketHeader(transportId, senderId, payloadType::idAndNameResolutionResponse);															//Set recipient and payloadType
	transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize++] = id;														//Add the nodeId
	if(id == currentNodeId && currentNodeName != nullptr)
	{
		transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize++] = strlen(currentNodeName);								//Add the name length
		memcpy(&transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize], currentNodeName, strlen(currentNodeName));		//Add the name
		transport[transportId].transmitPacketSize += strlen(currentNodeName);
		transport[transportId].transmitBuffer[(uint8_t)headerPosition::packetLength] = transport[transportId].transmitPacketSize;					//Update packetLength field
	}
	else
	{
		uint8_t nodeIndex = nodeIndexFromId(id);
		if(nodeIndex != maximumNumberOfNodes)
		{
			transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize++] = strlen(node[nodeIndex].name);							//Add the name length
			memcpy(&transport[transportId].transmitBuffer[transport[transportId].transmitPacketSize], node[nodeIndex].name, strlen(node[nodeIndex].name));	//Add the name
			transport[transportId].transmitPacketSize += strlen(node[nodeIndex].name);
			transport[transportId].transmitBuffer[(uint8_t)headerPosition::packetLength] = transport[transportId].transmitPacketSize;				//Update packetLength field
		}
	}
	processPacketBeforeTransmission(transportId);																								//Do CRC and encryption if needed
}
/*
 *
 *	Packet unpacking
 *
 */
void treacleClass::unpackPacket()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrintTransportName(receiveTransport);
		debugPrint(' ');
		debugPrint(treacleDebugString_received);
		debugPrint(' ');
		debugPrint(receiveBufferSize);
		debugPrint(' ');
		debugPrint(treacleDebugString_bytes);
		debugPrint(' ');
	#endif
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] >= (uint8_t)headerPosition::payload)	//Must reach a minimum size of just the header and CRC
	{
		if(receiveBufferSize >= receiveBuffer[(uint8_t)headerPosition::packetLength] + 2)			//Must also be a consistent length for a packet with a CRC at the end, but allow for padding
		{
			#if defined(TREACLE_DEBUG)
				debugPrintPayloadTypeDescription((uint8_t)receiveBuffer[(uint8_t)headerPosition::payloadType]);
				debugPrint(' ');
			#endif
			if(receiveBuffer[(uint8_t)headerPosition::payloadType] & (uint8_t)payloadType::encrypted)	//Check for encrypted packet
			{
				decryptPayload(receiveBuffer, receiveBufferSize);			//Decrypt payload after the header, note this is not shown as valid until the CRC is checked
			}
			if(validatePacketChecksum(receiveBuffer, receiveBufferSize))	//Checksum must be valid. This also strips the checksum from the end of the packet!
			{
				receiveBufferCrcChecked = true;
				uint8_t senderId = receiveBuffer[(uint8_t)headerPosition::sender];
				#if defined(TREACLE_DEBUG)
					debugPrint(treacleDebugString_fromSpace);
					debugPrint(treacleDebugString_nodeId);
					debugPrint(':');
					debugPrint(senderId);
				#endif
				if(senderId != (uint8_t)nodeId::unknownNode && receiveBuffer[(uint8_t)headerPosition::sender] != (uint8_t)nodeId::allNodes)	//Only handle valid node IDs
				{
					if(nodeExists(receiveBuffer[(uint8_t)headerPosition::sender]) == false)		//Check if it doesn't exist
					{
						if(addNode(receiveBuffer[(uint8_t)headerPosition::sender]))				//Add node if possible
						{
							#if defined(TREACLE_DEBUG)
								debugPrint(treacleDebugString_SpacenewCommaadded);
							#endif
						}
						else	//Abandon process, there are already the maximum number of nodes
						{
							#if defined(TREACLE_DEBUG)
								debugPrint(treacleDebugString__too_many_nodes);
							#endif
							clearReceiveBuffer();
							return;
						}
					}
					#if defined(TREACLE_DEBUG)
						debugPrint(' ');
					#endif
					uint8_t nodeIndex = nodeIndexFromId(receiveBuffer[(uint8_t)headerPosition::sender]);								//Turn node ID into nodeIndex
					#if defined(TREACLE_SUPPORT_LORA)
						if(receiveTransport == loRaTransportId)
						{
							rssi[nodeIndex] = lastLoRaRssi;																					//Record RSSI if it's a LoRa packet
							snr[nodeIndex] = lastLoRaSNR;																					//Record SNR if it's a LoRa packet
						}
					#endif
					#if defined(TREACLE_DEBUG)
						debugPrintString(node[nodeIndex].name);
					#endif
					if(receiveBuffer[(uint8_t)headerPosition::payloadNumber] == node[nodeIndex].lastPayloadNumber[receiveTransport])	//Check for duplicate packets
					{
						#if defined(TREACLE_DEBUG)
							debugPrintln(treacleDebugString_duplicate);
						#endif
						clearReceiveBuffer();
						return;
					}
					node[nodeIndex].lastPayloadNumber[receiveTransport] = receiveBuffer[(uint8_t)headerPosition::payloadNumber];
					#if defined(TREACLE_DEBUG)
						debugPrint(treacleDebugString_payload_numberColon);
						debugPrint(node[nodeIndex].lastPayloadNumber[receiveTransport]);
					#endif
					if(node[nodeIndex].rxReliability[receiveTransport] != 0xffff)
					{
						#if defined(TREACLE_DEBUG)
							debugPrint(' ');
							debugPrint(treacleDebugString_rxReliability);
							debugPrint(':');
							debugPrint(reliabilityPercentage(node[nodeIndex].rxReliability[receiveTransport]));
							debugPrint('%');
						#endif
					}
					node[nodeIndex].lastSeen = millis();	//Overall last seen
					node[nodeIndex].rxReliability[receiveTransport] = (node[nodeIndex].rxReliability[receiveTransport] >> 1) | 0x8000;	//Potentially improve rxReliability
					node[nodeIndex].lastTick[receiveTransport] = millis();															//Update last tick time
					node[nodeIndex].nextTick[receiveTransport] = ((uint16_t)receiveBuffer[(uint8_t)headerPosition::nextTick])<<8;	//Update next tick time MSB
					node[nodeIndex].nextTick[receiveTransport] += ((uint16_t)receiveBuffer[1+(uint8_t)headerPosition::nextTick]);	//Update next tick time LSB
					#if defined(TREACLE_DEBUG)
						debugPrint(' ');
					#endif
					if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::keepalive)
					{
						if(currentState != state::selectingId)
						{
							unpackKeepalivePacket(receiveTransport, senderId);	//This might include this node's last used ID after a restart but it is impossible to tell in a keepalive
						}
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::idResolutionRequest)
					{
						unpackIdResolutionRequestPacket(receiveTransport, senderId);
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::nameResolutionRequest)
					{
						unpackNameResolutionRequestPacket(receiveTransport, senderId);
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::idAndNameResolutionResponse)
					{
						unpackIdAndNameResolutionResponsePacket(receiveTransport, senderId);
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::shortApplicationData)
					{
						#if defined(TREACLE_DEBUG)
							debugPrintln();
						#endif
						//Nothing needed, the application will pick this up
					}
					else
					{
						#if defined(TREACLE_DEBUG)
							debugPrint(treacleDebugString_unknown);
							debugPrint(':');
							debugPrintln(receiveBuffer[(uint8_t)headerPosition::payloadType]);
						#endif
						clearReceiveBuffer();
					}
				}
				else if(senderId == 0)	//The only likely case is that a node is starting and hasn't picked an ID
				{
					if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::idResolutionRequest)
					{
						#if defined(TREACLE_DEBUG)
							debugPrint(' ');
							debugPrintln(treacleDebugString_idResolutionRequest);
						#endif
						unpackIdResolutionRequestPacket(receiveTransport, senderId);		//Answer if possible to give back the same ID to a restarted node
					}
					clearReceiveBuffer();
				}
			}
			else
			{
				#if defined(TREACLE_DEBUG)
					debugPrintln(treacleDebugString_checksum_invalid);
				#endif
				clearReceiveBuffer();
				transport[receiveTransport].rxPacketsInvalid++;				//Note the invalid packet
			}
		}
		else
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_inconsistent);
			#endif
			clearReceiveBuffer();
			transport[receiveTransport].rxPacketsInvalid++;					//Note the invalid packet
		}
	}
	else
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_tooShort);
		#endif
		clearReceiveBuffer();
		transport[receiveTransport].rxPacketsInvalid++;						//Note the invalid packet
	}
}
void treacleClass::unpackKeepalivePacket(uint8_t transportId, uint8_t senderId)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_includes);
		#endif
		for(uint8_t bufferIndex = (uint8_t)headerPosition::payload; bufferIndex < receiveBuffer[(uint8_t)headerPosition::packetLength]; bufferIndex++)
		{
			#if defined(TREACLE_DEBUG)
				debugPrint(treacleDebugString_treacleSpace);
				debugPrint("\t");
				debugPrint(treacleDebugString_nodeId);
				debugPrint(':');
				debugPrint(receiveBuffer[bufferIndex]);
			#endif
			uint16_t receivedTxReliabilityMetric = (receiveBuffer[bufferIndex+1]<<8) + (receiveBuffer[bufferIndex+2]); 	//Retrieve txReliability MSB + MSB
			
			if(receiveBuffer[bufferIndex] == currentNodeId)
			{
				uint8_t senderIndex = nodeIndexFromId(receiveBuffer[(uint8_t)headerPosition::sender]);
				node[senderIndex].txReliability[transportId] = receivedTxReliabilityMetric;
				#if defined(TREACLE_DEBUG)
					debugPrint(' ');
					debugPrint(treacleDebugString_this_node);
					debugPrint(' ');
					debugPrint(treacleDebugString_txReliability);
					debugPrint(':');
					debugPrintln(node[senderIndex].txReliability[transportId]);
				#endif
			}
			else
			{
				if(nodeExists(receiveBuffer[bufferIndex]) == false)
				{
					if(receiveBuffer[bufferIndex] != (uint8_t)nodeId::unknownNode && receiveBuffer[bufferIndex] != (uint8_t)nodeId::allNodes)
					{
						if(addNode(receiveBuffer[bufferIndex], receivedTxReliabilityMetric))	//Use txReliability from the other node as a best guess starting point
						{
							#if defined(TREACLE_DEBUG)
								debugPrintln(treacleDebugString_SpacenewCommaadded);
							#endif
						}
						else	//Abandon process, there are already the maximum number of nodes
						{
							#if defined(TREACLE_DEBUG)
								debugPrintln(treacleDebugString__too_many_nodes);
							#endif
						}
					}
				}
				else
				{
					#if defined(TREACLE_DEBUG)
						debugPrint(' ');
						debugPrint(treacleDebugString_txReliability);
						debugPrint(':');
						debugPrintln(receivedTxReliabilityMetric);
					#endif
				}
			}
			bufferIndex+=2;	//Skip to next node ID in keepalive (the loop already does bufferIndex++)
		}
	}
	else
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln();
		#endif
	}
}
void treacleClass::unpackIdResolutionRequestPacket(uint8_t transportId, uint8_t senderId)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		uint8_t nameLength = receiveBuffer[(uint8_t)headerPosition::payload];	
		char nameToLookUp[nameLength + 1];
		strlcpy(nameToLookUp,(const char*)&receiveBuffer[1+(uint8_t)headerPosition::payload],nameLength + 1);	//Copy the name from the packet
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_looking_up);
			debugPrint(':');
			debugPrint(nameToLookUp);
			debugPrint(' ');
		#endif
		uint8_t nodeIndex = nodeIndexFromName(nameToLookUp);
		if(nodeIndex != maximumNumberOfNodes)
		{
			#if defined(TREACLE_DEBUG)
				debugPrint(treacleDebugString_nodeId);
				debugPrint(':');
			#endif
			if(packetInQueue() == false)
			{
				#if defined(TREACLE_DEBUG)
					debugPrint(node[nodeIndex].id);
					debugPrint(' ');
					debugPrintln(treacleDebugString_responding);
				#endif
				buildIdAndNameResolutionResponsePacket(transportId, node[nodeIndex].id, senderId);	//This is _probably_ from an unknown node, so send it direct
			}
			else
			{
				#if defined(TREACLE_DEBUG)
					debugPrintln(node[nodeIndex].id);
				#endif
			}
		}
		else
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_unknown);
			#endif
		}
	}
}
void treacleClass::unpackNameResolutionRequestPacket(uint8_t transportId, uint8_t senderId)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		uint8_t id = receiveBuffer[(uint8_t)headerPosition::payload];
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_looking_up);
			debugPrint(' ');
			debugPrint(treacleDebugString_nodeId);
			debugPrint(':');
			debugPrint(id);
			debugPrint(' ');
		#endif
		if(id == currentNodeId)	//It's this node, which MUST have a name
		{
			#if defined(TREACLE_DEBUG)
				debugPrint(treacleDebugString_node_name);
				debugPrint(':');
				debugPrint(currentNodeName);
				debugPrint(' ');
				debugPrintln(treacleDebugString_this_node);
			#endif
			if(packetInQueue() == false)										//Nothing currently in the queue
			{
				buildIdAndNameResolutionResponsePacket(transportId, id, (uint8_t)nodeId::allNodes);	//Send a response
			}
		}
		else
		{
			uint8_t nodeIndex = nodeIndexFromId(id);
			if(nodeIndex != maximumNumberOfNodes && node[nodeIndex].name != nullptr)	//It's known about and there's a name
			{
				#if defined(TREACLE_DEBUG)
					debugPrint(treacleDebugString_node_name);
					debugPrint(':');
				#endif
				if(packetInQueue() == false)							//Nothing currently in the queue
				{
					#if defined(TREACLE_DEBUG)
						debugPrint(node[nodeIndex].name);
						debugPrint(' ');
						debugPrintln(treacleDebugString_responding);
					#endif
					buildIdAndNameResolutionResponsePacket(transportId, node[nodeIndex].id, (uint8_t)nodeId::allNodes);	//Send a response
				}
				else
				{
					#if defined(TREACLE_DEBUG)
						debugPrintln(node[nodeIndex].name);	//Will have to respond later
					#endif
				}
			}
			else
			{
				#if defined(TREACLE_DEBUG)
					debugPrintln(treacleDebugString_unknown);
				#endif
			}
		}
	}
}
void treacleClass::unpackIdAndNameResolutionResponsePacket(uint8_t transportId, uint8_t senderId)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		uint8_t nameLength = receiveBuffer[1 + (uint8_t)headerPosition::payload];
		char nameReceived[nameLength + 1];
		strlcpy(nameReceived,(const char*)&receiveBuffer[2+(uint8_t)headerPosition::payload],nameLength + 1);
		if(currentState == state::selectingId)	//Will have asked for this node's ID
		{
			if(currentNodeId == 0 && strcmp(nameReceived, currentNodeName) == 0)	//It's this node's name and nodeId is unset
			{
				#if defined(TREACLE_DEBUG)
					debugPrint(receiveBuffer[(uint8_t)headerPosition::payload]);
					debugPrint(' ');
					debugPrintln(treacleDebugString_this_node);
				#endif
				currentNodeId = receiveBuffer[(uint8_t)headerPosition::payload];	//Use the ID
				changeCurrentState(state::selectedId);
			}
		}
		else
		{
			uint8_t nodeIndex = nodeIndexFromId(receiveBuffer[(uint8_t)headerPosition::payload]);	//Find the node nodeIndex this is for
			if(nodeIndex == maximumNumberOfNodes)													//Node does not exist
			{
				addNode(receiveBuffer[(uint8_t)headerPosition::payload]);							//Add the node
				nodeIndex = nodeIndexFromId(receiveBuffer[(uint8_t)headerPosition::payload]);		//Find the node nodeIndex for this new node
			}
			if(nodeIndex != maximumNumberOfNodes)
			{
				#if defined(TREACLE_DEBUG)
					debugPrintln(treacleDebugString_received);
					debugPrint(treacleDebugString_treacleSpace);
					debugPrint("\t");
					debugPrint(treacleDebugString_nodeId);
					debugPrint(':');
					debugPrint(receiveBuffer[(uint8_t)headerPosition::payload]);
					debugPrint(" <-> \"");
					debugPrint(nameReceived);
					debugPrint('"');
				#endif
				bool copyName = false;
				if(node[nodeIndex].name != nullptr && strcmp(node[nodeIndex].name, nameReceived) != 0)	//Name has changed
				{
					delete node[nodeIndex].name;
					copyName = true;
				}
				else if(node[nodeIndex].name == nullptr)	//No name set
				{
					copyName = true;
				}
				if(copyName == true)	//Copy the name
				{
					node[nodeIndex].name = new char[nameLength];
					strlcpy(node[nodeIndex].name,(const char*)&receiveBuffer[2+(uint8_t)headerPosition::payload],nameLength + 1);
					#if defined(TREACLE_DEBUG)
						debugPrintln(treacleDebugString_SpacenewCommaadded);
					#endif
				}
				else
				{
					#if defined(TREACLE_DEBUG)
						debugPrintln();
					#endif
				}
			}
		}
	}
}
/*
 *
 *	Node management
 *
 */
bool treacleClass::nodeExists(uint8_t id)
{
	/*
	for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)	//Naive iteration, it's not like the data is large
	{
		if(node[nodeIndex].id == id)
		{
			return true;
		}
	}
	return false;
	*/
	return nodeIndexFromId(id) != maximumNumberOfNodes;
}
uint8_t treacleClass::nodeIndexFromId(uint8_t id)
{
	for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)	//Naive iteration, it's not like the data is large
	{
		if(node[nodeIndex].id == id)
		{
			return nodeIndex;
		}
	}
	return maximumNumberOfNodes;
}
uint8_t treacleClass::nodeIndexFromName(char* name)
{
	for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)	//Naive iteration, it's not like the data is large
	{
		if(node[nodeIndex].name != nullptr && strcmp(node[nodeIndex].name, name) == 0)
		{
			return nodeIndex;
		}
	}
	return maximumNumberOfNodes;
}
bool treacleClass::addNode(uint8_t id, uint16_t reliability)
{
	if(numberOfNodes < maximumNumberOfNodes)
	{
		node[numberOfNodes].id = id;													//Simple storage of ID
		node[numberOfNodes].lastTick = new uint32_t[numberOfActiveTransports];			//This is per transport
		node[numberOfNodes].nextTick = new uint16_t[numberOfActiveTransports];			//This is per transport
		node[numberOfNodes].rxReliability = new uint16_t[numberOfActiveTransports];		//This is per transport
		node[numberOfNodes].txReliability = new uint16_t[numberOfActiveTransports];		//This is per transport
		node[numberOfNodes].lastPayloadNumber = new uint8_t[numberOfActiveTransports];	//This is per transport
		for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)
		{
			node[numberOfNodes].lastTick[transportIndex] = millis();						//Count the addition of the node as a 'tick'
			node[numberOfNodes].nextTick[transportIndex] = maximumTickTime;					//Don't time it out until it's genuinely missed a 'tick'
			node[numberOfNodes].rxReliability[transportIndex] = reliability;
			node[numberOfNodes].txReliability[transportIndex] = reliability;
			node[numberOfNodes].lastPayloadNumber[transportIndex] = 0;						//Cannot make any assumptions about payload number
		}
		numberOfNodes++;
		return true;
	}
	return false;
}
/*
 *
 *	Duty cycle management
 *
 */
void treacleClass::calculateDutyCycle(uint8_t transportId)
{
	transport[transportId].calculatedDutyCycle = ((float)transport[transportId].txTime/(float)millis())/10.0;	//txTime is in micros so divided by 1000 to get percentage
}
/*
void treacleClass::calculateDutyCycle()
{
	for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)	//This does not cope with counter rollover!
	{
		
		//transport[transportIndex].calculatedDutyCycle = ((float)transport[transportIndex].txTime/(float)millis())/10.0;	//txTime is in micros so divided by 1000 to get percentage
	}
}
*/
/*
 *
 *	Node status functions
 *
 */
bool treacleClass::online(uint8_t id)
{
	uint8_t nodeIndex = nodeIndexFromId(id);
	if(nodeIndex != maximumNumberOfNodes)
	{
		for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)
		{
			if(online(nodeIndex, transportIndex))
			{
				return true;
			}
		}
	}
	return false;
}
bool treacleClass::online(uint8_t index, uint8_t transport)
{
	return (node[index].txReliability[transport] >= 0x8000 || node[index].rxReliability[transport] >= 0x8000 || countBits(node[index].txReliability[transport]) > 8 || countBits(node[index].rxReliability[transport]) > 8);
}
uint32_t treacleClass::rxAge(uint8_t id)
{
	uint8_t nodeIndex = nodeIndexFromId(id);
	if(nodeIndex != maximumNumberOfNodes)
	{
		return millis() - node[nodeIndex].lastSeen;
	}
	return 0;
}
uint16_t treacleClass::rxReliability(uint8_t id)
{
	uint8_t nodeIndex = nodeIndexFromId(id);
	uint16_t reliability = 0;
	if(nodeIndex != maximumNumberOfNodes)
	{
		for(uint8_t index = 0; index < numberOfActiveTransports; index++)
		{
			if(node[nodeIndex].rxReliability[index] > reliability)
			{
				reliability = node[nodeIndex].rxReliability[index];
			}
		}
	}
	/*
	if(nodeIndex != maximumNumberOfNodes)
	{
		if(espNowInitialised() == true)
		{
			if(loRaInitialised() == true)
			{
				return max(node[nodeIndex].rxReliability[espNowTransportId], node[nodeIndex].rxReliability[loRaTransportId]);
			}
			return node[nodeIndex].rxReliability[espNowTransportId];
		}
		else if(loRaInitialised() == true)
		{
			return node[nodeIndex].rxReliability[loRaTransportId];
		}
	}
	*/
	return reliability;
}
uint16_t treacleClass::txReliability(uint8_t id)
{
	uint8_t nodeIndex = nodeIndexFromId(id);
	uint16_t reliability = 0;
	if(nodeIndex != maximumNumberOfNodes)
	{
		for(uint8_t index = 0; index < numberOfActiveTransports; index++)
		{
			if(node[nodeIndex].txReliability[index] > reliability)
			{
				reliability = node[nodeIndex].txReliability[index];
			}
		}
		/*
		if(espNowInitialised() == true)
		{
			if(loRaInitialised() == true)
			{
				return max(node[nodeIndex].txReliability[espNowTransportId], node[nodeIndex].txReliability[loRaTransportId]);
			}
			return node[nodeIndex].txReliability[espNowTransportId];
		}
		else if(loRaInitialised() == true)
		{
			return node[nodeIndex].txReliability[loRaTransportId];
		}
		*/
	}
	return reliability;
}
/*
 *
 *	Messaging functions
 *
 */ 
uint8_t treacleClass::nodes()
{
	return numberOfNodes;
}
bool treacleClass::online()
{
	return currentState == state::online;
}
void goOffline()
{
}
void goOnline()
{
}
bool treacleClass::packetReceived()
{
	return receiveBufferSize != 0;	//Check the buffer payload
}
bool treacleClass::applicationDataPacketReceived()
{
	return receiveBufferSize != 0 && receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::shortApplicationData;	//Check the buffer payload type
}
void treacleClass::clearReceiveBuffer()
{
	receiveBufferSize = 0;	//Clear the buffer payload
}
uint32_t treacleClass::messageWaiting()
{
	#if defined(TREACLE_DEBUG)
		if(currentState == state::online && debug_uart_ != nullptr && millis() - lastStatusMessage > 60E3)
		{
			lastStatusMessage = millis();
			showStatus();
		}
	#endif
	#if defined(TREACLE_SUPPORT_MQTT)
		if(MQTTTransportId != 255 && transport[MQTTTransportId].initialised == true)	//Polling method for MQTT, must be enabled and initialised
		{
			if(!mqtt->connected())
			{
				connectToMQTTserver();
			}
			else
			{
				mqtt->loop();
			}
		}
	#endif
	#if defined(TREACLE_SUPPORT_LORA)
		if(loRaTransportId != 255 && transport[loRaTransportId].initialised == true && loRaIrqPin == -1)	//Polling method for loRa packets, must be enabled and initialised
		{
			receiveLoRa();
		}
	#endif
	#if defined(TREACLE_SUPPORT_UDP) && (defined(ESP8266) || defined(AVR))
		if(UDPTransportId != 255 && transport[UDPTransportId].initialised == true)	//Polling method for UDP packets, must be enabled and initialised
		{
			receiveUDP();
		}
	#endif
	#if defined(TREACLE_SUPPORT_COBS)
		if(cobsTransportId != 255 && transport[cobsTransportId].initialised == true)	//Polling method for COBS packets, must be enabled and initialised
		{
			receiveCobs();
		}
	#endif
	if(currentState == state::uninitialised || currentState == state::starting || currentState == state::stopped)
	{
		return 0;						//Nothing can be sent or received in these states
	}
	else if(packetReceived() && receiveBufferCrcChecked == false)	//Only unpack a packet once
	{
		unpackPacket();					//Handle unpacking of an incoming packet
	}
	else if(sendPacketOnTick() == true)	//Send a packet if it needs to happen now
	{
		return 0;						//A tick has been sent, so the application can wait until next time for any data
	}
	timeOutTicks();					//Potentially time out ticks from other nodes if they are not responding or the application is slow calling this
	if(currentState == state::selectingId)
	{
		if(applicationDataPacketReceived())
		{
			clearReceiveBuffer();		//Any incoming application data is binned in this state
		}
		else if(millis() - lastStateChange > maximumTickTime)
		{
			if(selectNodeId())
			{
				changeCurrentState(state::selectedId);	//Need to inform other nodes of this node's presence
			}
			else
			{
				changeCurrentState(state::stopped);		//Nothing can be sent or received
			}
		}
		return 0;	//Nothing can be sent or received, only IDs collected
	}
	else if(currentState == state::selectedId || currentState == state::online || currentState == state::offline)	//Normal active states
	{
		if(applicationDataPacketReceived() && receiveBufferCrcChecked == true)
		{
			return receiveBuffer[(uint8_t)headerPosition::packetLength] - (uint8_t)headerPosition::payload;
		}
	}
	return 0;
}
void treacleClass::clearWaitingMessage()
{
	clearReceiveBuffer();
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_message);
		debugPrint(' ');
		debugPrintln(treacleDebugString_cleared);
	#endif
}
uint32_t treacleClass::suggestedQueueInterval()
{
	bool nodeReached[numberOfNodes] = {};	//Used to track which nodes _should_ have been reached, in transport priority order and avoid sending using lower priority transports, if possible
	uint8_t numberOfNodesReached = 0;
	if(numberOfActiveTransports > 1)
	{
		for (uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
		{
			if(transport[transportId].initialised == true)	//It's initialised
			{
				for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
				{
					if(nodeReached[nodeIndex] == false)
					{
						if(online(nodeIndex, transportId))
						//if(node[nodeIndex].txReliability[transportId] >= 0x8000 || countBits(node[nodeIndex].txReliability[transportId]) > 8)	//This node is PROBABLY reachable on this transport!
						{
							nodeReached[nodeIndex] = true;
							numberOfNodesReached++;
						}
					}
				}
			}
			if(numberOfNodesReached == numberOfNodes)
			{
				#if defined(TREACLE_DEBUG)
					debugPrint(treacleDebugString_treacleSpace);
					debugPrint(treacleDebugString_suggested_message_interval);
					debugPrint(' ');
					debugPrint((transport[transportId].nextTick * 2)/1000);
					debugPrintln('s');
				#endif
				return transport[transportId].nextTick * 2;
			}
		}
	}
	else if(numberOfActiveTransports == 1)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_suggested_message_interval);
			debugPrint(' ');
			debugPrint((transport[0].nextTick * 2)/1000);
			debugPrintln('s');
		#endif
		return transport[0].nextTick * 2;
	}
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_suggested_message_interval);
		debugPrint(' ');
		debugPrint((maximumTickTime * 2)/1000);
		debugPrintln('s');
	#endif
	return maximumTickTime * 2;
}
uint8_t treacleClass::messageSender()
{
	return receiveBuffer[(uint8_t)headerPosition::sender];
}
bool treacleClass::queueMessage(char* data)
{
	return queueMessage((uint8_t*)data, strlen(data)+1);
}
bool treacleClass::queueMessage(const unsigned char* data, uint8_t length)
{
	return queueMessage((uint8_t*)data, (uint8_t)length);
}
bool treacleClass::queueMessage(uint8_t* data, uint8_t length)
{
	bool nodeReached[numberOfNodes] = {};	//Used to track which nodes _should_ have been reached, in transport priority order and avoid sending using lower priority transports, if possible
	uint8_t numberOfNodesReached = 0;
	if(length < maximumPayloadSize)
	{
		for (uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
		{
			if(transport[transportId].initialised == true &&	//It's initialised
				packetInQueue(transportId) == false) 			//It's got nothing waiting to go
			{
				buildPacketHeader(transportId, (uint8_t)nodeId::allNodes, payloadType::shortApplicationData);			//Make an application data packet
				memcpy(&transport[transportId].transmitBuffer[(uint8_t)headerPosition::payload], data, length);			//Copy the data starting at headerPosition::payload
				transport[transportId].transmitBuffer[(uint8_t)headerPosition::packetLength] = 							//Update packetLength field
				(uint8_t)headerPosition::payload + length;
				transport[transportId].transmitPacketSize += length;													//Update the length of the transmit buffer
				processPacketBeforeTransmission(transportId);															//Do CRC and encryption if needed
				for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
				{
					if(nodeReached[nodeIndex] == false)
					{
						if(online(nodeIndex, transportId))
						//if(node[nodeIndex].txReliability[transportId] >= 0x8000 || countBits(node[nodeIndex].txReliability[transportId]) > 8)	//This node is PROBABLY reachable on this transport!
						{
							nodeReached[nodeIndex] = true;
							numberOfNodesReached++;
						}
					}
				}
			}
			if(numberOfNodesReached == numberOfNodes)
			{
				return true;	//We have almost certainly reached all the nodes with this transport, do not queue the message for lower priority (or higher cost) transports
			}
		}
		return true;
	}
	return false;
}
bool treacleClass::sendMessage(char* data)
{
	bringForwardNextTick();
	return queueMessage((uint8_t*)data, strlen(data)+1);
}
bool treacleClass::sendMessage(const unsigned char* data, uint8_t length)
{
	bringForwardNextTick();
	return queueMessage((uint8_t*)data, (uint8_t)length);
}
bool treacleClass::sendMessage(uint8_t* data, uint8_t length)
{
	bringForwardNextTick();
	return queueMessage(data, length);
}
bool treacleClass::retrieveWaitingMessage(uint8_t* destination)
{
	if(applicationDataPacketReceived())
	{
		memcpy(destination, &receiveBuffer[(uint8_t)headerPosition::payload], receiveBuffer[(uint8_t)headerPosition::packetLength] - (uint8_t)headerPosition::payload);
		return true;
	}
	return false;
}
uint8_t treacleClass::maxPayloadSize()
{
	return maximumPayloadSize;
}
/*
 *
 *	Utility functions
 *
 */
#if defined(TREACLE_DEBUG)
	void treacleClass::showStatus()
	{
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_nodeId);
		debugPrint(':');
		debugPrint(currentNodeId);
		debugPrint(' ');
		debugPrintString(currentNodeName);
		debugPrint(' ');
		debugPrint(treacleDebugString_up);
		debugPrint(':');
		debugPrint(millis()/60E3);
		debugPrint(' ');
		debugPrint(treacleDebugString_minutes);
		debugPrint(' ');
		debugPrintState(currentState);
		//debugPrint(' ');
		//debugPrint(treacleDebugString_for);
		//debugPrint(' ');
		debugPrint(':');
		debugPrint((millis()-lastStateChange)/60E3);
		debugPrint(' ');
		debugPrintln(treacleDebugString_minutes);
		for(uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
		{
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint("\t");
			debugPrintTransportName(transportId);
			debugPrint(treacleDebugString_SpacedutySpacecycle);
			debugPrint(':');
			debugPrint(transport[transportId].calculatedDutyCycle);
			debugPrint('%');
			debugPrint(' ');
			debugPrint(treacleDebugString_TX);
			debugPrint(':');
			debugPrint(transport[transportId].txPackets);
			debugPrint(' ');
			debugPrint(treacleDebugString_TX);
			debugPrint(treacleDebugString_drops_colon);
			debugPrint(transport[transportId].txPacketsDropped);
			debugPrint(' ');
			debugPrint(treacleDebugString_RX);
			debugPrint(':');
			debugPrint(transport[transportId].rxPackets);
			debugPrint(' ');
			debugPrint(treacleDebugString_RX);
			debugPrint(treacleDebugString_drops_colon);
			debugPrint(transport[transportId].rxPacketsDropped);
			debugPrint(' ');
			debugPrint(treacleDebugString_RX);
			debugPrint(treacleDebugString_invalid_colon);
			debugPrint(transport[transportId].rxPacketsInvalid);
			debugPrint(' ');
			debugPrint(treacleDebugString_RX);
			debugPrint(treacleDebugString_ignored_colon);
			debugPrintln(transport[transportId].rxPacketsIgnored);
		}
		for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
		{
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint("\t");
			debugPrint(treacleDebugString_nodeId);
			debugPrint(':');
			debugPrint(node[nodeIndex].id);
			debugPrint(' ');
			debugPrintStringln(node[nodeIndex].name);
			for(uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
			{
				debugPrint(treacleDebugString_treacleSpace);
				debugPrint("\t\t");
				debugPrintTransportName(transportId);
				debugPrint(' ');
				debugPrint(treacleDebugString_txReliability);
				debugPrint(':');
				debugPrint(reliabilityPercentage(node[nodeIndex].txReliability[transportId]));
				debugPrint('%');
				debugPrint(' ');
				debugPrint(treacleDebugString_rxReliability);
				debugPrint(':');
				debugPrint(reliabilityPercentage(node[nodeIndex].rxReliability[transportId]));
				debugPrint('%');
				debugPrint(' ');
				debugPrint(treacleDebugString_payload_numberColon);
				debugPrintln(node[nodeIndex].lastPayloadNumber[transportId]);
				//uint32_t* lastTick = nullptr; 			//This is per transport
				//uint16_t* nextTick = nullptr; 			//This is per transport
			}
		}
	}
#endif
/*
 *
 *	Turns a bitmask reliability measure into a percentate
 *
 */
float treacleClass::reliabilityPercentage(uint16_t bitmask)
{
	return 100.0*countBits(bitmask)/16.0;
}
/*
 *
 *	Returns the number of 1 bits in an uint32_t, used in measuring link quality
 *
 */
uint8_t treacleClass::countBits(uint32_t thingToCount)
{
  uint8_t result = 0;
  for(uint8_t i = 0; i < 32 ; i++)
  {
    result+=(0x00000001 << i) & thingToCount ? 1 : 0;
  }
  return result;
}
treacleClass treacle;	//Create an instance of the class, as only one is practically usable at a time
#endif
