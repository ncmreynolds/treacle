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

bool treacleClass::begin(uint8_t maxNodes)
{
	//The maximum number of nodes is used in creating a load of data structures
	//maximumNumberOfNodes = maxNodes;
	//node = new nodeInfo[maximumNumberOfNodes];	//Assign at start
	//The name is important so assign one if it is not set. This is based off MAC address on ESP32
	if(currentNodeName == nullptr)
	{
		uint8_t localMacAddress[6];
		WiFi.macAddress(localMacAddress);
		currentNodeName = new char[9];
		sprintf_P(currentNodeName, PSTR("node%02X%02X"), localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
	}
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_node_name);
	debugPrint(':');
	debugPrintln(currentNodeName);
	debugPrint(debugString_treacleSpace);
	debugPrintln(debugString_starting);
	changeCurrentState(state::starting);
	if(numberOfActiveProtocols > 0)
	{
		protocol = new protocolData[numberOfActiveProtocols];
		
		//Initialise all the protocols
		uint8_t numberOfInitialisedProtocols = 0;
		for(uint8_t index = 0; index < numberOfActiveProtocols; index++)	//Initialise every protocol that is enabled
		{
			if(index == espNowProtocolId)
			{
				protocol[index].initialised = initialiseEspNow();
			}
			else if(index == loRaProtocolId)
			{
				protocol[index].initialised = initialiseLoRa();
			}
			else if(index == cobsProtocolId)
			{
				protocol[index].initialised = initialiseCobs();
			}
			if(protocol[index].initialised == true)
			{
				numberOfInitialisedProtocols++;
			}
			if(encryptionKey != nullptr)
			{
				protocol[index].encrypted = true;	//Default to encrypted if a key is set
			}
		}
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_start);
		debugPrint(':');
		if(numberOfInitialisedProtocols == numberOfActiveProtocols)
		{
			debugPrintln(debugString_OK);
			debugPrint(debugString_treacleSpace);
			debugPrint(debugString_ActiveSpaceProtocols);
			debugPrint(':');
			debugPrintln(numberOfInitialisedProtocols);
			for(uint8_t protocol = 0; protocol < numberOfActiveProtocols; protocol++)
			{
				debugPrint(debugString_treacleSpace);
				debugPrintProtocolName(protocol);
				debugPrint(debugString_SpaceProtocolID);
				debugPrint(':');
				debugPrintln(protocol);
			}
			changeCurrentState(state::selectingId);
			return true;
		}
	}
	debugPrintln(debugString_failed);
	changeCurrentState(state::stopped);
	return false;
}
void treacleClass::end()
{
	debugPrint(debugString_treacleSpace);
	debugPrintln(debugString_ended);
}
void treacleClass::enableDebug(Stream &debugStream)
{
	debug_uart_ = &debugStream;				//Set the stream used for debug
	#if defined(ESP8266)
	if(&debugStream == &Serial)
	{
		  debug_uart_->write(17);			//Send an XON to stop the hung terminal after reset on ESP8266, which often emits an XOFF
	}
	#endif
}
void treacleClass::disableDebug()
{
	debug_uart_ = nullptr;
}
/*
 *
 *	Protocol abstraction/genericisation helpers
 *
 */
bool treacleClass::packetInQueue()
{
	for(uint8_t index = 0; index < numberOfActiveProtocols; index++)
	{
		if(protocol[index].bufferSent == false)			//At least one protocol has not sent the buffer
		{
			return true;
		}
	}
	return false;
}
bool treacleClass::packetInQueue(uint8_t protocolId)
{
	if(protocol[protocolId].bufferSent == false)		//Protocol has not sent the buffer
	{
		return true;
	}
	return false;
}
bool treacleClass::sendBuffer(uint8_t protocolId, uint8_t* buffer, uint8_t packetSize)
{
	if(protocolId == espNowProtocolId)
	{
		return sendBufferByEspNow(buffer, packetSize);
	}
	else if(protocolId == loRaProtocolId)
	{
		return sendBufferByLoRa(buffer, packetSize);
	}
	else if(protocolId == cobsProtocolId)
	{
		return sendBufferByCobs(buffer, packetSize);
	}
}
/*
 *
 *	Generic protocol functions
 *
 */
void treacleClass::enableEncryption(uint8_t protocolId)				//Enable encryption for a specific protocol
{
	if(protocol != nullptr && protocolId != 255)
	{
		protocol[protocolId].encrypted = true;
	}
}
void treacleClass::disableEncryption(uint8_t protocolId)				//Disable encryption for a specific protocol
{
	if(protocol != nullptr && protocolId != 255)
	{
		protocol[protocolId].encrypted = false;
	}
}
/*
 *
 *	ESP-Now functions
 *
 */
void treacleClass::enableEspNow()
{
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_enablingSpace);
	debugPrintln(debugString_ESPNow);
	espNowProtocolId = numberOfActiveProtocols++;
}
bool treacleClass::espNowEnabled()
{
	return espNowProtocolId != 255;	//Not necessarily very useful, but it can be checked
}
void treacleClass::enableEspNowEncryption()				//Enable encryption for ESP-Now
{
	enableEncryption(espNowProtocolId);
}
void treacleClass::disableEspNowEncryption()			//Disable encryption for ESP-Now
{
	disableEncryption(espNowProtocolId);
}
bool treacleClass::espNowInitialised()
{
	if(espNowProtocolId != 255)
	{
		return protocol[espNowProtocolId].initialised;
	}
	return false;
}
void treacleClass::setEspNowChannel(uint8_t channel)
{
	preferredespNowChannel = channel;							//Sets the preferred channel. It will only be used if practical.
}
uint8_t treacleClass::getEspNowChannel()
{
	return currentEspNowChannel;								//Gets the current channel
}
bool treacleClass::initialiseWiFi()								//Checks to see the state of the WiFi
{
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_checkingSpace);
	debugPrint(debugString_WiFi);
	debugPrint(':');
	if(WiFi.getMode() == WIFI_STA)
	{
		debugPrint(debugString_Client);
	}
	else if(WiFi.getMode() == WIFI_AP)
	{
		debugPrint(debugString_AP);
		debugPrint(' ');
		debugPrint(debugString_channel);
		debugPrint(':');
		debugPrintln(WiFi.channel());
		debugPrint(' ');
		debugPrintln(debugString_OK);
		return true;
	}
	else if(WiFi.getMode() == WIFI_AP_STA)
	{
		debugPrint(debugString_ClientAndAP);
	}
	else if(WiFi.getMode() == WIFI_MODE_NULL)
	{
		debugPrintln(debugString_notInitialised);
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_initialisingSpace);
		debugPrint(debugString_WiFi);
		debugPrint(':');
		if(WiFi.scanNetworks() > 0)								//A WiFi scan nicely sets everything up without joining a network
		{
			debugPrintln(debugString_OK);
			debugPrint(debugString_treacleSpace);
			debugPrint(debugString_WiFi);
			debugPrint(' ');
			debugPrint(debugString_channel);
			debugPrint(':');
			debugPrintln(WiFi.channel());
			if(WiFi.channel() != preferredespNowChannel)		//Change channel if needed
			{
				changeWiFiChannel(preferredespNowChannel);
			}
			return true;
		}
		else
		{
			debugPrintln(debugString_failed);
		}
	}
	else
	{
		debugPrint(debugString_unknown);
		debugPrint(':');
		debugPrint(WiFi.getMode());
	}
	return false;
}
bool treacleClass::changeWiFiChannel(uint8_t channel)
{
	if (channel > 13)
	{
		#if defined(ESP8266)
		wifi_country_t wiFiCountryConfiguration;
		wiFiCountryConfiguration.cc[0] = 'J';
		wiFiCountryConfiguration.cc[1] = 'P';
		wiFiCountryConfiguration.cc[2] = '\0';
		wiFiCountryConfiguration.schan = 1;
		wiFiCountryConfiguration.nchan = 14;
		wiFiCountryConfiguration.policy = WIFI_COUNTRY_POLICY_MANUAL;
		if (wifi_set_country(&wiFiCountryConfiguration) == false)
		{
			if(debug_uart_ != nullptr)
			{
				debug_uart_->print(F("\n\rUnable to set country to JP for channel 14 use"));
			}
			return false;
		}
		#elif defined ESP32
		#endif
	}
	if(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE) == ESP_OK)
	{
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_WiFi);
		debugPrint(' ');
		debugPrint(debugString_channel);
		debugPrint(' ');
		debugPrint(debugString_changedSpaceTo);
		debugPrint(':');
		debugPrintln(WiFi.channel());
		return true;
	}
	return false;
}
bool treacleClass::initialiseEspNow()
{
	if(initialiseWiFi())
	{
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_initialisingSpace);
		debugPrint(debugString_ESPNow);
		debugPrint(':');
		if(WiFi.getMode() == WIFI_AP)
		{
			if(WiFi.channel() != preferredespNowChannel)
			{
				changeWiFiChannel(preferredespNowChannel);
			}
		}
		currentEspNowChannel = WiFi.channel();
		if(esp_now_init() == ESP_OK)
		{
			if(addEspNowPeer(broadcastMacAddress))
			{
				if(esp_now_register_recv_cb(	//ESP-Now receive callback
					[](const uint8_t *macAddress, const uint8_t *receivedMessage, int receivedMessageLength)
					{
						if(treacle.currentState != treacle.state::starting)	//Must not receive packets before the buffers are allocated
						{
							if(treacle.receiveBufferSize == 0 && receivedMessageLength < treacle.maximumBufferSize)	//Check the receive buffer is empty first
							{
								treacle.protocol[treacle.espNowProtocolId].rxPackets++;						//Count the packet as received
								if(receivedMessage[(uint8_t)treacle.headerPosition::recipient] == (uint8_t)treacle.nodeId::allNodes ||
									receivedMessage[(uint8_t)treacle.headerPosition::recipient] == treacle.currentNodeId)	//Packet is meaningful to this node
								{
									memcpy(&treacle.receiveBuffer,receivedMessage,receivedMessageLength);	//Copy the ESP-Now payload
									treacle.receiveBufferSize = receivedMessageLength;						//Record the amount of payload
									treacle.receiveBufferCrcChecked = false;								//Mark the payload as unchecked
									treacle.receiveProtocol = treacle.espNowProtocolId;						//Record that it was received by ESP-Now
									treacle.protocol[treacle.espNowProtocolId].rxPacketsProcessed++;		//Count the packet as processed
								}
							}
							else
							{
								treacle.protocol[treacle.espNowProtocolId].rxPacketsDropped++;				//Count the drop
							}
						}
					}
				) == ESP_OK)
				{
					if(esp_now_register_send_cb(
						[](const uint8_t* macAddress, esp_now_send_status_t status)	//ESP-Now send callback is used to measure airtime for duty cycle calculations
						{
							if(status == ESP_OK)
							{
								if(treacle.protocol[treacle.espNowProtocolId].txStartTime != 0)				//Check the initial send time was recorded
								{
									treacle.protocol[treacle.espNowProtocolId].txTime += micros()			//Add to the total transmit time
										- treacle.protocol[treacle.espNowProtocolId].txStartTime;
									treacle.protocol[treacle.espNowProtocolId].txStartTime = 0;				//Clear the initial send time
								}
								treacle.protocol[treacle.espNowProtocolId].txPackets++;						//Count the packet
							}
							else
							{
								treacle.protocol[treacle.espNowProtocolId].txPacketsDropped++;				//Count the drop
							}
						}
					) == ESP_OK)
					{
						protocol[espNowProtocolId].initialised = true;
						debugPrintln(debugString_OK);
						return true;
					}
				}
			}
		}
	}
	protocol[espNowProtocolId].initialised = false;
	debugPrintln(debugString_failed);
	return false;
}
bool treacleClass::addEspNowPeer(uint8_t* macaddress)
{
	/*
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_addingSpace);
	debugPrint(debugString_ESPNow);
	debugPrint(debugString_peer);
	debugPrint(':');
	*/
	esp_now_peer_info_t newPeer;
	newPeer.peer_addr[0] = macaddress[0];
	newPeer.peer_addr[1] = macaddress[1];
	newPeer.peer_addr[2] = macaddress[2];
	newPeer.peer_addr[3] = macaddress[3];
	newPeer.peer_addr[4] = macaddress[4];
	newPeer.peer_addr[5] = macaddress[5];
	if(WiFi.getMode() == WIFI_STA)
	{
		newPeer.ifidx = WIFI_IF_STA;
	}
	else if(WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
	{
		newPeer.ifidx = WIFI_IF_AP;
	}
	else	//WiFi not initialised
	{
		return false;
	}
	newPeer.channel = currentEspNowChannel;
	newPeer.encrypt = false;
	if(esp_now_add_peer(&newPeer) == ESP_OK)
	{
		//debugPrintln(debugString_OK);
		return true;
	}
	//debugPrintln(debugString_failed);
	return false;
}
bool treacleClass::deleteEspNowPeer(uint8_t* macaddress)
{
	if(esp_now_del_peer(macaddress) == ESP_OK)
	{
		return true;
	}
	return false;
}

bool treacleClass::sendBufferByEspNow(uint8_t* buffer, uint8_t packetSize)
{
	protocol[espNowProtocolId].txStartTime = micros();
	esp_err_t espNowSendResult = esp_now_send(broadcastMacAddress, buffer, (size_t)packetSize);
	if(espNowSendResult == ESP_OK)
	{
		return true;	//The send callback function records success/fail and txTime from here on
	}
	else
	{
		protocol[espNowProtocolId].txPacketsDropped++;		//Record the drop
		if(WiFi.channel() != currentEspNowChannel)			//Channel has changed, alter the peer address
		{
			if(deleteEspNowPeer(broadcastMacAddress))		//This could perhaps be changed to modify the existing peer but this should be infrequent
			{
				addEspNowPeer(broadcastMacAddress);
			}
		}
	}
	protocol[espNowProtocolId].txStartTime = 0;
	return false;
}
/*
 *
 *	LoRa functions
 *
 */
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
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_enablingSpace);
	debugPrintln(debugString_LoRa);
	loRaProtocolId = numberOfActiveProtocols++;
}
bool treacleClass::loRaEnabled()
{
	return loRaProtocolId != 255;	//Not necessarily very useful, but it can be checked
}
bool treacleClass::initialiseLoRa()
{
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_initialisingSpace);
	debugPrint(debugString_LoRa);
	debugPrint(':');
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
		LoRa.setTxPower(defaultLoRaTxPower);					//Set TX power
		LoRa.setSpreadingFactor(defaultLoRaSpreadingFactor);	//Set spreading factor
		LoRa.setSignalBandwidth(defaultLoRaSignalBandwidth);	//Set badwidth
		LoRa.setSyncWord(loRaSyncWord);							//Set sync word
		LoRa.enableCrc();										//Enable CRC check
		LoRa.receive();											//Start LoRa reception
		debugPrintln(debugString_OK);
		protocol[loRaProtocolId].initialised = true;			//Mark as initialised
		if(loRaIrqPin != -1)									//Callbacks on IRQ pin
		{
			LoRa.onTxDone(										//Send callback function
				[]() {
					if(treacle.protocol[treacle.loRaProtocolId].txStartTime != 0)			//Check the initial send time was recorded
					{
						treacle.protocol[treacle.loRaProtocolId].txTime += micros()			//Add to the total transmit time
							- treacle.protocol[treacle.loRaProtocolId].txStartTime;
						treacle.protocol[treacle.loRaProtocolId].txStartTime = 0;			//Clear the initial send time
					}
					treacle.protocol[treacle.loRaProtocolId].txPackets++;					//Count the packet
				}
			);
		}
	}
	else
	{
		debugPrintln(debugString_failed);
		protocol[loRaProtocolId].initialised = false;			//Mark as not initialised
	}
	return protocol[loRaProtocolId].initialised;
}
bool treacleClass::loRaInitialised()
{
	if(loRaProtocolId != 255)
	{
		return protocol[loRaProtocolId].initialised;
	}
	return false;
}
bool treacleClass::sendBufferByLoRa(uint8_t* buffer, uint8_t packetSize)
{
	if(LoRa.beginPacket())
	{
		LoRa.write(buffer, packetSize);
		if(loRaIrqPin == -1)															//No LoRa IRQ, do this synchronously
		{
			protocol[loRaProtocolId].txStartTime = micros();
			if(LoRa.endPacket())
			{
				protocol[loRaProtocolId].txTime += micros()			//Add to the total transmit time
					- protocol[loRaProtocolId].txStartTime;
				protocol[loRaProtocolId].txStartTime = 0;			//Clear the initial send time
				protocol[loRaProtocolId].txPackets++;				//Count the packet
				return true;
			}
		}
		else
		{
			protocol[loRaProtocolId].txStartTime = micros();
			if(LoRa.endPacket(true))
			{
				return true;
			}
		}
	}
	protocol[loRaProtocolId].txStartTime = 0;
	return false;
}
bool treacleClass::receiveLoRa()
{
	uint8_t receivedMessageLength = LoRa.parsePacket();
	if(receivedMessageLength > 0)
	{
		if(receiveBufferSize == 0 && receivedMessageLength < treacle.maximumBufferSize)
		{
			protocol[loRaProtocolId].rxPackets++;						//Count the packet as received
			if(LoRa.peek() == (uint8_t)nodeId::allNodes ||
				LoRa.peek() == currentNodeId)							//Packet is meaningful to this node
			{
				LoRa.readBytes(receiveBuffer, receivedMessageLength);	//Copy the LoRa payload
				receiveBufferSize = receivedMessageLength;				//Record the amount of payload
				receiveBufferCrcChecked = false;						//Mark the payload as unchecked
				receiveProtocol = loRaProtocolId;						//Record that it was received by ESP-Now
				protocol[loRaProtocolId].rxPacketsProcessed++;			//Count the packet as processed
				return true;
			}
		}
		else
		{
			protocol[loRaProtocolId].rxPacketsDropped++;				//Count the drop
		}
		while(LoRa.available())											//Drop the packet
		{
			LoRa.read();
		}
	}
	return false;
}
/*
 *
 *	COBS functions
 *
 */
void treacleClass::enableCobs()
{
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_enablingSpace);
	debugPrint(debugString_COBS);
	cobsProtocolId = numberOfActiveProtocols++;
}
bool treacleClass::cobsEnabled()
{
	return cobsProtocolId != 255;	//Not necessarily very useful, but it can be checked
}
bool treacleClass::cobsInitialised()
{
	if(cobsProtocolId != 255)
	{
		return protocol[cobsProtocolId].initialised;
	}
	return false;
}
bool treacleClass::initialiseCobs()
{
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_initialisingSpace);
	debugPrint(debugString_COBS);
	debugPrint(':');
	debugPrintln(debugString_failed);
	return false;
}
bool treacleClass::sendBufferByCobs(uint8_t* buffer, uint8_t packetSize)
{
	return false;
}
/*
 *
 * ID management
 *
 */
bool treacleClass::selectNodeId()
{
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_selectingSpace);
	debugPrint(debugString_nodeId);
	debugPrint(':');
	bool idIsUnique = false;
	while(idIsUnique == false)
	{
		currentNodeId = random(minimumNodeId,maximumNodeId);
		idIsUnique = (nodeExists(currentNodeId) == false);
	}
	if(idIsUnique == true)
	{
		debugPrintln(currentNodeId);
		return true;
	}
	debugPrintln(debugString_failed);
	return false;
}
/*
 *
 *	General packet handling
 *
 */
bool treacleClass::processPacketBeforeTransmission(uint8_t protocolId)
{
	if(appendChecksumToPacket(protocol[protocolId].transmitBuffer, protocol[protocolId].transmitPacketSize))		//Append checksum after making the packet, but do not increment the packetLength field
	{
		if(protocol[protocolId].encrypted == true)
		{
			return encryptPayload(protocol[protocolId].transmitBuffer, protocol[protocolId].transmitPacketSize);	//Encrypt the payload but not the header
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
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_addingSpace);
	debugPrintln(debugString_encryption_key);
	encryptionKey = key;
	#ifdef ESP32
		esp_aes_init(&context);							//Initialise the AES context
		esp_aes_setkey(&context, encryptionKey, 128);	//Set the key
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
		debugPrint(debugString_padded_by);
		debugPrint(' ');
		debugPrint(padding);
		debugPrint(' ');
		debugPrint(debugString_toSpace);
		debugPrint(packetSize - (uint8_t)headerPosition::blockIndex);
		debugPrint(' ');
		debugPrint(debugString_bytes);
		debugPrint(' ');
	}
	#if defined(TREACLE_OBFUSCATE_ONLY)
		for(uint8_t index = (uint8_t)headerPosition::blockIndex; index < packetSize; index++)
		{
			buffer[index] = buffer[index] ^ index;	//This is obfuscation only, for testing.
		}
	#else
		uint8_t initialisationVector[16];												//Allocate an initialisation vector
		//memset(initialisationVector, 0, sizeof(initialisationVector));
		memcpy(&initialisationVector[0],  buffer, 4);									//Use the first four bytes of the packet, repeated for the IV
		memcpy(&initialisationVector[4],  buffer, 4);
		memcpy(&initialisationVector[8],  buffer, 4);
		memcpy(&initialisationVector[12], buffer, 4);
		uint8_t encryptedData[packetSize - (uint8_t)headerPosition::blockIndex];		//Temporary location for the encryted data
		esp_aes_crypt_cbc(&context, ESP_AES_ENCRYPT,									//Do the encryption
			packetSize - (uint8_t)headerPosition::blockIndex,							//Length of the data to encrypt
			initialisationVector,														//Initialisation vector
			&buffer[(uint8_t)headerPosition::blockIndex],								//The point to start encrypting from
			encryptedData);																//Destination for the encrypted version
		memcpy(&buffer[(uint8_t)headerPosition::blockIndex], encryptedData, packetSize - (uint8_t)headerPosition::blockIndex); //Copy the encrypted version back over the buffer
	#endif
	debugPrint(debugString_encrypted);
	debugPrint(' ');
	buffer[(uint8_t)headerPosition::payloadType] = buffer[(uint8_t)headerPosition::payloadType] | (uint8_t)payloadType::encrypted;	//Mark as encrypted
	return true;
}
	
bool treacleClass::decryptPayload(uint8_t* buffer, uint8_t& packetSize)	//Decrypt the payload and remove the padding, if necessary
{
	if(packetSize != buffer[(uint8_t)headerPosition::packetLength] + 2)
	{
		debugPrint(debugString_padded_by);
		debugPrint(' ');
		debugPrint(packetSize - (buffer[(uint8_t)headerPosition::packetLength] + 2));
		debugPrint(' ');
		debugPrint(debugString_bytes);
		debugPrint(' ');
		//packetSize = buffer[(uint8_t)headerPosition::packetLength] + 2;
	}
	//if(packetSize >= (uint8_t)headerPosition::blockIndex && packetSize <= maximumBufferSize - 2)
	{
		debugPrint(debugString_decrypted);
		debugPrint(' ');
		#if defined(TREACLE_OBFUSCATE_ONLY)
			for(uint8_t index = (uint8_t)headerPosition::blockIndex; index < packetSize; index++)
			{
				buffer[index] = buffer[index] ^ index;	//This is obfuscation only, for testing.
			}
		#else
			uint8_t initialisationVector[16];												//Allocate an initialisation vector
			//memset(initialisationVector, 0, sizeof(initialisationVector));
			memcpy(&initialisationVector[0],  buffer, 4);									//Use the first four bytes of the packet, repeated for the IV
			memcpy(&initialisationVector[4],  buffer, 4);
			memcpy(&initialisationVector[8],  buffer, 4);
			memcpy(&initialisationVector[12], buffer, 4);
			uint8_t decryptedData[packetSize - (uint8_t)headerPosition::blockIndex];		//Temporary location for the decryted data
			esp_aes_crypt_cbc(&context, ESP_AES_DECRYPT,									//Do the decryption
				packetSize - (uint8_t)headerPosition::blockIndex,							//Length of the data to encrypt
				initialisationVector,														//Initialisation vector
				&buffer[(uint8_t)headerPosition::blockIndex],								//The point to start encrypting from
				decryptedData);																//Destination for the encrypted version
			memcpy(&buffer[(uint8_t)headerPosition::blockIndex], decryptedData, packetSize - (uint8_t)headerPosition::blockIndex); //Copy the decrypted version back over the buffer
		#endif
		buffer[(uint8_t)headerPosition::payloadType] = buffer[(uint8_t)headerPosition::payloadType] & (0xff ^ (uint8_t)payloadType::encrypted);	//Mark as not encrypted, otherwise the CRC is invalid
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
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_newSpaceState);
		debugPrint(':');
		debugPrintState(newState);
		debugPrint(' ');
		debugPrint(debugString_after);
		debugPrint(' ');
		debugPrint((millis()-lastStateChange)/60E3);
		debugPrint(' ');
		debugPrintln(debugString_minutes);
		currentState = newState;
		lastStateChange = millis();
	}
}
/*
 *
 *	Tick timers
 *
 */
uint16_t treacleClass::minimumTickTime(uint8_t protocolId)
{
	if(protocolId == espNowProtocolId) return	100;
	else if(protocolId == loRaProtocolId) return 500;
	else if(protocolId == cobsProtocolId) return 250;
	else return 1000;
}
void treacleClass::setTickTime()
{
	if(currentState == state::uninitialised || currentState == state::starting) return;
	for(uint8_t index = 0; index < numberOfActiveProtocols; index++)
	{
		if(currentState == state::selectingId)
		{
			if(index == espNowProtocolId) protocol[index].nextTick =	1E3 -	tickRandomisation(index);
			else if(index == loRaProtocolId) protocol[index].nextTick =	45E3 -	tickRandomisation(index);
			else if(index == cobsProtocolId) protocol[index].nextTick =	10E3 -	tickRandomisation(index);
		}
		else if(currentState == state::selectedId)
		{
			if(index == espNowProtocolId) protocol[index].nextTick = 	1E3 -	tickRandomisation(index);
			else if(index == loRaProtocolId) protocol[index].nextTick =	45E3 -	tickRandomisation(index);
			else if(index == cobsProtocolId) protocol[index].nextTick =	15E3 -	tickRandomisation(index);
		}
		else if(currentState == state::online)
		{
			if(index == espNowProtocolId) protocol[index].nextTick =	10E3 -	tickRandomisation(index);
			else if(index == loRaProtocolId) protocol[index].nextTick =	45E3 -	tickRandomisation(index);
			else if(index == cobsProtocolId) protocol[index].nextTick =	100E3 -	tickRandomisation(index);
		}
		else if(currentState == state::offline)
		{
			if(index == espNowProtocolId) protocol[index].nextTick =	5E3 -	tickRandomisation(index);
			else if(index == loRaProtocolId) protocol[index].nextTick =	45E3 -	tickRandomisation(index);
			else if(index == cobsProtocolId) protocol[index].nextTick =	15E3 -	tickRandomisation(index);
		}
		else if(currentState == state::stopped)
		{
			protocol[index].nextTick =									0;		//Never send
		}
		else
		{
			if(index == espNowProtocolId) protocol[index].nextTick =	maximumTickTime;
			else if(index == loRaProtocolId) protocol[index].nextTick =	maximumTickTime;
			else if(index == cobsProtocolId) protocol[index].nextTick =	maximumTickTime;
		}
	}
}
uint16_t treacleClass::tickRandomisation(uint8_t protocolId)
{
	return random(minimumTickTime(protocolId), minimumTickTime(protocolId)*2);
}
void treacleClass::bringForwardNextTick()
{
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_expediting_);
	debugPrintln(debugString_response);
	for(uint8_t index = 0; index < numberOfActiveProtocols; index++)
	{
		if(protocol[index].nextTick != 0)
		{
			protocol[index].lastTick = millis() - (protocol[index].nextTick + tickRandomisation(index));
		}
	}
}
bool treacleClass::sendPacketOnTick()
{
	for(uint8_t protocolIndex = 0; protocolIndex < numberOfActiveProtocols; protocolIndex++)
	{
		if(protocol[protocolIndex].nextTick != 0 && millis() - protocol[protocolIndex].lastTick > protocol[protocolIndex].nextTick)	//nextTick = 0 implies never
		{
			protocol[protocolIndex].lastTick = millis();									//Update the last tick time
			debugPrint(debugString_treacleSpace);
			debugPrintProtocolName(protocolIndex);
			debugPrint(' ');
			if(protocol[protocolIndex].calculatedDutyCycle < protocol[protocolIndex].maximumDutyCycle)
			{
				if(packetInQueue(protocolIndex) == false)									//Nothing ready to send from the application for _this_ protocol
				{
					if(currentState == state::selectingId)							//Speed up ID selection by asking existing node IDs
					{
						buildIdResolutionRequestPacket(protocolIndex, currentNodeName);		//Ask about this node with a name->Id request
						debugPrint(debugString_idResolutionRequest);
					}
					else
					{
						if(packetInQueue() == false && protocol[protocolIndex].payloadNumber%4 == 0)						//If nothing else is queued to transmit over any protocol, 25% of packets can be name requests to backfill names
						{
							for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
							{
								if(node[nodeIndex].name == nullptr && packetInQueue() == false)		//This node has no name yet
								{
									buildNameResolutionRequestPacket(protocolIndex, node[nodeIndex].id);	//Ask for the node's name with an id->name request
									debugPrint(debugString_nameResolutionRequest);
								}
							}
						}
						if(packetInQueue(protocolIndex) == false)				//Nothing else is queued to transmit over _this_ protocol, keepalives can go over a higher priority protocol with something in a lower one queued
						{
							buildKeepalivePacket(protocolIndex);				//Create an empty tick as a keepalive and announcement of all known nodeIds
							debugPrint(debugString_keepalive);
						}
					}
				}
				else	//There is already data which may or many not already have been sent by other protocols
				{
					debugPrintPayloadTypeDescription(protocol[protocolIndex].transmitBuffer[(uint8_t)headerPosition::payloadType]);
				}
				debugPrint(':');
				if(sendBuffer(protocolIndex, protocol[protocolIndex].transmitBuffer, protocol[protocolIndex].transmitPacketSize))
				{
					debugPrint(protocol[protocolIndex].transmitPacketSize);
					debugPrint(' ');
					debugPrint(debugString_bytes);
					debugPrint(' ');
					debugPrint(debugString_sent);
					debugPrint(' ');
					debugPrint(debugString_toSpace);
					debugPrint(debugString_nodeId);
					debugPrint(':');
					if(protocol[protocolIndex].transmitBuffer[0] == (uint8_t)nodeId::allNodes)
					{
						debugPrintln(debugString_all);
					}
					else if(protocol[protocolIndex].transmitBuffer[0] == (uint8_t)nodeId::unknownNode)
					{
						debugPrintln(debugString_unknown);
					}
					else
					{
						debugPrintln(protocol[protocolIndex].transmitBuffer[0]);
					}
					protocol[protocolIndex].bufferSent = true;
					return true;
				}
				else
				{
					debugPrintln(debugString_failed);
					return false;
				}
			}
			else
			{
				debugPrint(debugString_duty_cycle_exceeded);
				debugPrint(':');
				debugPrint(protocol[protocolIndex].calculatedDutyCycle);
				debugPrintln('%');
			}
		}
	}
	return false;
}
void treacleClass::timeOutTicks()
{
	uint16_t totalTxReliability = 0x0000;
	uint16_t totalRxReliability = 0x0000;
	for(uint8_t protocolId = 0; protocolId < numberOfActiveProtocols; protocolId++)
	{
		for(uint8_t index = 0; index < numberOfNodes; index++)
		{
			if(millis() - node[index].lastTick[protocolId] > node[index].nextTick[protocolId] + minimumTickTime(protocolId))	//Missed the next window
			{
				node[index].rxReliability[protocolId] = node[index].rxReliability[protocolId] >> 1;	//Reduce rxReliability
				node[index].txReliability[protocolId] = node[index].txReliability[protocolId] >> 1;	//As we've not heard anything to the contrary also reduce txReliability
				node[index].lastTick[protocolId] = millis();										//Update last tick timer, even though one was missed
				debugPrint(debugString_treacleSpace);
				debugPrintProtocolName(protocolId);
				debugPrint(' ');
				debugPrint(debugString_nodeId);
				debugPrint(':');
				debugPrint(node[index].id);
				debugPrint(' ');
				debugPrint(debugString_dropped);
				debugPrint(' ');
				debugPrint(debugString_rxReliability);
				debugPrint(':');
				debugPrintln(node[index].rxReliability[protocolId]);
			}
			totalTxReliability = totalTxReliability | node[index].txReliability[protocolId];		//OR all the bits of transmit reliability we have
			totalRxReliability = totalRxReliability | node[index].rxReliability[protocolId];		//OR all the bits of receive reliability we have
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
void treacleClass::buildPacketHeader(uint8_t protocolId, uint8_t recipient, payloadType type)
{
	setTickTime();																								//States and protocols all have their own tick times
	if(recipient == (uint8_t)nodeId::unknownNode)
	{
		bringForwardNextTick();																					//Bring forward the next tick ASAP for any starting nodes
	}
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::recipient] = recipient;						//Add the recipient Id
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::sender] = currentNodeId;						//Add the current nodeId
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::payloadType] = (uint8_t)type;					//Payload type
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::payloadNumber] = protocol[protocolId].payloadNumber++;				//Payload number, which post-increments
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = 0;								//Payload length - starts at 0 and gets updated
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::blockIndex] = 0;								//Large payload start bits 16-23
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::blockIndex+1] = 0;								//Large payload start bits 8-15
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::blockIndex+2] = 0;								//Large payload start bits 0-7
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::nextTick] = (protocol[protocolId].nextTick & 0xff00) >> 8;	//nextTick bits 8-15
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::nextTick+1] = (protocol[protocolId].nextTick & 0x00ff);	//nextTick bits 0-7
	protocol[protocolId].transmitPacketSize = (uint8_t)headerPosition::payload;									//Set the size to just the header
	protocol[protocolId].bufferSent = false;																	//Mark as unsent for this protocol
	/*
	for(uint8_t index = 0; index < numberOfActiveProtocols; index++)
	{
		protocol[index].bufferSent = false;													//Mark as unsent for each protocol
	}
	*/
}
void treacleClass::buildKeepalivePacket(uint8_t protocolId)
{
	buildPacketHeader(protocolId, (uint8_t)nodeId::allNodes, payloadType::keepalive);						//Set payloadType
	for(uint8_t index = 0; index < numberOfNodes; index++)									//Add all nodes with a known name
	{
		if(node[index].name != nullptr && node[index].rxReliability > 0)					//Include nodes with names that have non-zero receive history
		{
			protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] = node[index].id;							//Include node ID
			protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] =											//Include node RX reliability MSB
				(uint8_t)((node[index].rxReliability[protocolId]&0xff00)>>8);
			protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] =											//Include node RX reliability LSB
				(uint8_t)(node[index].rxReliability[protocolId]&0x00ff);
			
		}
	}
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = protocol[protocolId].transmitPacketSize;				//Update packetLength field
	processPacketBeforeTransmission(protocolId);																							//Do CRC and encryption if needed
}
void treacleClass::buildIdResolutionRequestPacket(uint8_t protocolId, char* name)				//Ask for a ID from a name
{
	buildPacketHeader(protocolId, (uint8_t)nodeId::allNodes, payloadType::idResolutionRequest);			//Set payloadType
	protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] = strlen(name);
	memcpy(&protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize], name, strlen(name));
	protocol[protocolId].transmitPacketSize += strlen(name);
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = protocol[protocolId].transmitPacketSize;						//Update packetLength field
	processPacketBeforeTransmission(protocolId);																								//Do CRC and encryption if needed
}
void treacleClass::buildNameResolutionRequestPacket(uint8_t protocolId, uint8_t id)			//Ask for a name from an ID
{
	buildPacketHeader(protocolId, (uint8_t)nodeId::allNodes, payloadType::nameResolutionRequest);			//Set payloadType
	protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] = id;
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = protocol[protocolId].transmitPacketSize;						//Update packetLength field
	processPacketBeforeTransmission(protocolId);																								//Do CRC and encryption if needed
}
void treacleClass::buildIdAndNameResolutionResponsePacket(uint8_t protocolId, uint8_t id, uint8_t senderId)		//Supply ID and name
{
	buildPacketHeader(protocolId, senderId, payloadType::idAndNameResolutionResponse);															//Set recipient and payloadType
	protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] = id;														//Add the nodeId
	if(id == currentNodeId && currentNodeName != nullptr)
	{
		protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] = strlen(currentNodeName);								//Add the name length
		memcpy(&protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize], currentNodeName, strlen(currentNodeName));		//Add the name
		protocol[protocolId].transmitPacketSize += strlen(currentNodeName);
		protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = protocol[protocolId].transmitPacketSize;					//Update packetLength field
	}
	else
	{
		uint8_t index = indexFromId(id);
		if(index != maximumNumberOfNodes)
		{
			protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] = strlen(node[index].name);							//Add the name length
			memcpy(&protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize], node[index].name, strlen(node[index].name));	//Add the name
			protocol[protocolId].transmitPacketSize += strlen(node[index].name);
			protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = protocol[protocolId].transmitPacketSize;				//Update packetLength field
		}
	}
	processPacketBeforeTransmission(protocolId);																								//Do CRC and encryption if needed
}
/*
 *
 *	Packet unpacking
 *
 */
void treacleClass::unpackPacket()
{
	debugPrint(debugString_treacleSpace);
	debugPrintProtocolName(receiveProtocol);
	debugPrint(' ');
	debugPrint(debugString_received);
	debugPrint(' ');
	debugPrint(receiveBufferSize);
	debugPrint(' ');
	debugPrint(debugString_bytes);
	debugPrint(' ');
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] >= (uint8_t)headerPosition::payload)	//Must reach a minimum size of just the header and CRC
	{
		if(receiveBufferSize >= receiveBuffer[(uint8_t)headerPosition::packetLength] + 2)			//Must also be a consistent length for a packet with a CRC at the end, but allow for padding
		{
			debugPrintPayloadTypeDescription((uint8_t)receiveBuffer[(uint8_t)headerPosition::payloadType]);
			debugPrint(' ');
			if(receiveBuffer[(uint8_t)headerPosition::payloadType] & (uint8_t)payloadType::encrypted)	//Check for encrypted packet
			{
				decryptPayload(receiveBuffer, receiveBufferSize);			//Decrypt payload after the header, note this is not shown as valid until the CRC is checked
			}
			if(validatePacketChecksum(receiveBuffer, receiveBufferSize))	//Checksum must be valid. This also strips the checksum from the end of the packet!
			{
				receiveBufferCrcChecked = true;
				debugPrint(debugString_fromSpace);
				debugPrint(debugString_nodeId);
				debugPrint(':');
				uint8_t senderId = receiveBuffer[(uint8_t)headerPosition::sender];
				debugPrint(senderId);
				if(senderId != 0 && receiveBuffer[(uint8_t)headerPosition::sender] != 255)	//Only handle valid node IDs
				{
					if(nodeExists(receiveBuffer[(uint8_t)headerPosition::sender]) == false)		//Check if it doesn't exist
					{
						if(addNode(receiveBuffer[(uint8_t)headerPosition::sender]))				//Add node if possible
						{
							debugPrint(debugString_SpacenewCommaadded);
						}
						else	//Abandon process, there are already the maximum number of nodes
						{
							debugPrint(debugString__too_many_nodes);
							clearReceiveBuffer();
							return;
						}
					}
					debugPrint(' ');
					uint8_t index = indexFromId(receiveBuffer[(uint8_t)headerPosition::sender]);								//Turn node ID into index
					if(node[index].name != nullptr)
					{
						debugPrint('"');
						debugPrint(node[index].name);
						debugPrint('"');
						debugPrint(' ');
					}
					if(receiveBuffer[(uint8_t)headerPosition::payloadNumber] == node[index].lastPayloadNumber[receiveProtocol])	//Check for duplicate packets
					{
						debugPrintln(debugString_duplicate);
						clearReceiveBuffer();
						return;
					}
					node[index].lastPayloadNumber[receiveProtocol] = receiveBuffer[(uint8_t)headerPosition::payloadNumber];
					debugPrint(debugString_payload_numberColon);
					debugPrint(node[index].lastPayloadNumber[receiveProtocol]);
					if(node[index].rxReliability[receiveProtocol] != 0xffff)
					{
						debugPrint(' ');
						debugPrint(debugString_rxReliability);
						debugPrint(':');
						debugPrint(node[index].rxReliability[receiveProtocol]);
					}
					node[index].rxReliability[receiveProtocol] = (node[index].rxReliability[receiveProtocol] >> 1) | 0x8000;	//Potentially improve rxReliability
					node[index].lastTick[receiveProtocol] = millis();															//Update last tick time
					node[index].nextTick[receiveProtocol] = ((uint16_t)receiveBuffer[(uint8_t)headerPosition::nextTick])<<8;	//Update next tick time MSB
					node[index].nextTick[receiveProtocol] += ((uint16_t)receiveBuffer[1+(uint8_t)headerPosition::nextTick]);	//Update next tick time LSB
					debugPrint(' ');
					if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::keepalive)
					{
						if(currentState != state::selectingId)
						{
							unpackKeepalivePacket(receiveProtocol, senderId);	//This might include this node's last used ID after a restart but it is impossible to tell in a keepalive
						}
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::idResolutionRequest)
					{
						unpackIdResolutionRequestPacket(receiveProtocol, senderId);
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::nameResolutionRequest)
					{
						unpackNameResolutionRequestPacket(receiveProtocol, senderId);
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::idAndNameResolutionResponse)
					{
						unpackIdAndNameResolutionResponsePacket(receiveProtocol, senderId);
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::shortApplicationData)
					{
						debugPrintln();
						//Nothing needed, the application will pick this up
					}
					else
					{
						debugPrint(debugString_unknown);
						debugPrint(':');
						debugPrintln(receiveBuffer[(uint8_t)headerPosition::payloadType]);
						clearReceiveBuffer();
					}
				}
				else if(senderId == 0)	//The only likely case is that a node is starting and hasn't picked an ID
				{
					if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::idResolutionRequest)
					{
						debugPrint(' ');
						debugPrintln(debugString_idResolutionRequest);
						unpackIdResolutionRequestPacket(receiveProtocol, senderId);		//Answer if possible to give back the same ID to a restarted node
					}
					clearReceiveBuffer();
				}
			}
			else
			{
				debugPrintln(debugString_checksum_invalid);
				clearReceiveBuffer();
			}
		}
		else
		{
			debugPrintln(debugString_inconsistent);
			clearReceiveBuffer();
		}
	}
	else
	{
		debugPrintln(debugString_tooShort);
		clearReceiveBuffer();
	}
}
void treacleClass::unpackKeepalivePacket(uint8_t protocol, uint8_t senderId)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		debugPrintln(debugString_includes);
		for(uint8_t index = (uint8_t)headerPosition::payload; index < receiveBuffer[(uint8_t)headerPosition::packetLength]; index++)
		{
			debugPrint("\t");
			debugPrint(debugString_nodeId);
			debugPrint(':');
			debugPrint(receiveBuffer[index]);
			if(receiveBuffer[index] == currentNodeId)
			{
				uint8_t senderIndex = indexFromId(receiveBuffer[(uint8_t)headerPosition::sender]);
				node[senderIndex].txReliability[protocol] = receiveBuffer[index+1]<<8;	//Retrieve txReliability LSB
				node[senderIndex].txReliability[protocol] += receiveBuffer[index+2];	//Retrieve txReliability MSB
				debugPrint(' ');
				debugPrint(debugString_this_node);
				debugPrint(' ');
				debugPrint(debugString_txReliability);
				debugPrint(':');
				debugPrintln(node[senderIndex].txReliability[protocol]);
			}
			else
			{
				if(nodeExists(receiveBuffer[index]) == false)
				{
					if(addNode(receiveBuffer[index]))
					{
						debugPrintln(debugString_SpacenewCommaadded);
					}
					else	//Abandon process, there are already the maximum number of nodes
					{
						debugPrintln(debugString__too_many_nodes);
					}
				}
				else
				{
					debugPrintln();
				}
			}
			index+=2;	//Skip to next node ID in keepalive (the loop already does index++)
		}
	}
	else
	{
		debugPrintln();
	}
}
void treacleClass::unpackIdResolutionRequestPacket(uint8_t protocol, uint8_t senderId)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		uint8_t nameLength = receiveBuffer[(uint8_t)headerPosition::payload];	
		char nameToLookUp[nameLength + 1];
		strlcpy(nameToLookUp,(const char*)&receiveBuffer[1+(uint8_t)headerPosition::payload],nameLength + 1);	//Copy the name from the packet
		debugPrint(debugString_looking_up);
		debugPrint(':');
		debugPrint(nameToLookUp);
		debugPrint(' ');
		uint8_t index = indexFromName(nameToLookUp);
		if(index != maximumNumberOfNodes)
		{
			debugPrint(debugString_nodeId);
			debugPrint(':');
			if(packetInQueue() == false)
			{
				debugPrint(node[index].id);
				debugPrint(' ');
				debugPrintln(debugString_responding);
				buildIdAndNameResolutionResponsePacket(protocol, node[index].id, senderId);
			}
			else
			{
				debugPrintln(node[index].id);
			}
		}
		else
		{
			debugPrintln(debugString_unknown);
		}
	}
}
void treacleClass::unpackNameResolutionRequestPacket(uint8_t protocol, uint8_t senderId)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		uint8_t id = receiveBuffer[(uint8_t)headerPosition::payload];
		debugPrint(debugString_looking_up);
		debugPrint(' ');
		debugPrint(debugString_nodeId);
		debugPrint(':');
		debugPrint(id);
		debugPrint(' ');
		if(id == currentNodeId)	//It's this node, which MUST have a name
		{
			debugPrint(debugString_node_name);
			debugPrint(':');
			debugPrint(currentNodeName);
			debugPrint(' ');
			debugPrintln(debugString_this_node);
			if(packetInQueue() == false)										//Nothing currently in the queue
			{
				buildIdAndNameResolutionResponsePacket(protocol, id, senderId);	//Send a response
			}
		}
		else
		{
			uint8_t index = indexFromId(id);
			if(index != maximumNumberOfNodes && node[index].name != nullptr)	//It's known about and there's a name
			{
				debugPrint(debugString_node_name);
				debugPrint(':');
				if(packetInQueue() == false)							//Nothing currently in the queue
				{
					debugPrint(node[index].name);
					debugPrint(' ');
					debugPrintln(debugString_responding);
					buildIdAndNameResolutionResponsePacket(protocol, node[index].id, senderId);	//Send a response
				}
				else
				{
					debugPrintln(node[index].name);	//Will have to respond later
				}
			}
			else
			{
				debugPrintln(debugString_unknown);
			}
		}
	}
}
void treacleClass::unpackIdAndNameResolutionResponsePacket(uint8_t protocol, uint8_t senderId)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		uint8_t nameLength = receiveBuffer[1 + (uint8_t)headerPosition::payload];
		char nameReceived[nameLength + 1];
		strlcpy(nameReceived,(const char*)&receiveBuffer[2+(uint8_t)headerPosition::payload],nameLength + 1);
		debugPrint(debugString_received);
		debugPrint(' ');
		debugPrint(nameReceived);
		debugPrint(':');
		debugPrint(debugString_nodeId);
		debugPrint(':');
		if(currentState == state::selectingId)	//Will have asked for this node's ID
		{
			if(currentNodeId == 0 && strcmp(nameReceived, currentNodeName) == 0)	//It's this node's name and nodeId is unset
			{
				debugPrint(receiveBuffer[(uint8_t)headerPosition::payload]);
				debugPrint(' ');
				debugPrintln(debugString_this_node);
				currentNodeId = receiveBuffer[(uint8_t)headerPosition::payload];	//Use the ID
				changeCurrentState(state::selectedId);
			}
		}
		else
		{
			uint8_t index = indexFromId(receiveBuffer[(uint8_t)headerPosition::payload]);	//Find the node index this is for
			if(index != maximumNumberOfNodes)
			{
				bool copyName = false;
				if(node[index].name != nullptr && strcmp(node[index].name, nameReceived) != 0)	//Name has changed
				{
					delete node[index].name;
					copyName = true;
				}
				else if(node[index].name == nullptr)	//No name set
				{
					copyName = true;
				}
				if(copyName == true)	//Copy the name
				{
					node[index].name = new char[nameLength];
					strlcpy(node[index].name,(const char*)&receiveBuffer[2+(uint8_t)headerPosition::payload],nameLength + 1);
					debugPrint(receiveBuffer[(uint8_t)headerPosition::payload]);
					debugPrintln(debugString_SpacenewCommaadded);
				}
				else
				{
					debugPrintln(receiveBuffer[(uint8_t)headerPosition::payload]);
				}
			}
			else
			{
				debugPrintln(receiveBuffer[(uint8_t)headerPosition::payload]);
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
	for(uint8_t index = 0; index < numberOfNodes; index++)	//Naive iteration, it's not like the data is large
	{
		if(node[index].id == id)
		{
			return true;
		}
	}
	return false;
	*/
	return indexFromId(id) != maximumNumberOfNodes;
}
uint8_t treacleClass::indexFromId(uint8_t id)
{
	for(uint8_t index = 0; index < numberOfNodes; index++)	//Naive iteration, it's not like the data is large
	{
		if(node[index].id == id)
		{
			return index;
		}
	}
	return maximumNumberOfNodes;
}
uint8_t treacleClass::indexFromName(char* name)
{
	for(uint8_t index = 0; index < numberOfNodes; index++)	//Naive iteration, it's not like the data is large
	{
		if(node[index].name != nullptr && strcmp(node[index].name, name) == 0)
		{
			return index;
		}
	}
	return maximumNumberOfNodes;
}
bool treacleClass::addNode(uint8_t id)
{
	if(numberOfNodes < maximumNumberOfNodes)
	{
		node[numberOfNodes].id = id;													//Simple storage of ID
		node[numberOfNodes].lastTick = new uint32_t[numberOfActiveProtocols];			//This is per protocol
		node[numberOfNodes].nextTick = new uint16_t[numberOfActiveProtocols];			//This is per protocol
		node[numberOfNodes].rxReliability = new uint16_t[numberOfActiveProtocols];		//This is per protocol
		node[numberOfNodes].txReliability = new uint16_t[numberOfActiveProtocols];		//This is per protocol
		node[numberOfNodes].lastPayloadNumber = new uint8_t[numberOfActiveProtocols];	//This is per protocol
		for(uint8_t index = 0; index < numberOfActiveProtocols; index++)
		{
			node[numberOfNodes].lastTick[index] = 0;								//Clear the values at the start
			node[numberOfNodes].nextTick[index] = maximumTickTime;
			node[numberOfNodes].rxReliability[index] = 0;
			node[numberOfNodes].txReliability[index] = 0;
			node[numberOfNodes].lastPayloadNumber[index] = 0;
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
void treacleClass::checkDutyCycle()
{
	/*
	debugPrint(debugString_treacleSpace);
	debugPrintState(currentState);
	debugPrint(' ');
	debugPrint(debugString_for);
	debugPrint(' ');
	debugPrint((millis()-lastStateChange)/60E3);
	debugPrint(' ');
	debugPrintln(debugString_minutes);
	*/
	for(uint8_t index = 0; index < numberOfActiveProtocols; index++)	//This does not cope with counter rollover!
	{
		protocol[index].calculatedDutyCycle = ((float)protocol[index].txTime/(float)millis())/10.0;	//txTime is in micros so divided by 1000 to get percentage
		/*
		debugPrint(debugString_treacleSpace);
		debugPrintProtocolName(index);
		debugPrint(debugString_SpacedutySpacecycle);
		debugPrint(':');
		debugPrint(protocol[index].calculatedDutyCycle);
		debugPrintln('%');
		*/
	}
}

/*
 *
 *	Messaging functions
 *
 */ 
bool treacleClass::online()
{
	return currentState == state::online;
}
bool treacleClass::online(uint8_t)
{
	return false;
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
	if(millis() - lastDutyCycleCheck > dutyCycleCheckInterval)
	{
		lastDutyCycleCheck = millis();
		checkDutyCycle();
	}
	if(debug_uart_ != nullptr && millis() - lastStatusMessage > 60E3)
	{
		lastStatusMessage = millis();
		showStatus();
	}
	if(loRaProtocolId != 255 && protocol[loRaProtocolId].initialised == true && loRaIrqPin == -1)	//Polling method for loRa packets, must be enabled and initialised
	{
		receiveLoRa();
	}
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
	//else
	{
		timeOutTicks();					//Potentially time out ticks from other nodes if they are not responding or the application is slow calling this
	}
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
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_message);
	debugPrint(' ');
	debugPrintln(debugString_cleared);
}
uint8_t treacleClass::messageSender()
{
	return 0;
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
	bool nodeReached[numberOfNodes] = {};	//Used to track which nodes _should_ have been reached, in protocol priority order and avoid sending using lower priority protocols, if possible
	uint8_t numberOfNodesReached = 0;
	if(length < maximumPayloadSize)
	{
		for (uint8_t protocolId = 0; protocolId < numberOfActiveProtocols; protocolId++)
		{
			if(protocol[protocolId].initialised == true &&	//It's initialised
				packetInQueue(protocolId) == false) 		//It's got nothing waiting to go
				//node[nodeIndex].txReliability[protocolId] > 0x0000)	//Don't queue up unless _some_ keepalives are getting through
			{
				buildPacketHeader(protocolId, (uint8_t)nodeId::allNodes, payloadType::shortApplicationData);			//Make an application data packet
				memcpy(&protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::payload], data, length);			//Copy the data starting at headerPosition::payload
				protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = 							//Update packetLength field
				(uint8_t)headerPosition::payload + length;
				protocol[protocolId].transmitPacketSize += length;														//Update the length of the transmit buffer
				processPacketBeforeTransmission(protocolId);															//Do CRC and encryption if needed
				for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
				{
					if(nodeReached[nodeIndex] == false)
					{
						if(node[nodeIndex].txReliability[protocolId] > 0xff00)	//This node is PROBABLY reachable on this protocol!
						{
							nodeReached[nodeIndex] == true;
							numberOfNodesReached++;
						}
					}
				}
			}
			if(numberOfNodesReached == numberOfNodes)
			{
				debugPrint(debugString_treacleSpace);
				debugPrint(debugString_all);
				debugPrint(' ');
				debugPrint(debugString_nodes);
				debugPrint(' ');
				debugPrint(debugString_reached);
				debugPrint(' ');
				debugPrint(debugString_with);
				debugPrint(' ');
				debugPrintProtocolName(protocolId);
				debugPrintln();
				return true;	//We have almost certainly reached all the nodes with this protocol, do not queue the message for lower priority (or higher cost) protocols
			}
		}
		return true;
	}
	return false;
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
void treacleClass::showStatus()
{
	debugPrint(debugString_treacleSpace);
	debugPrint(debugString_up);
	debugPrint(':');
	debugPrint(millis()/60E3);
	debugPrint(' ');
	debugPrint(debugString_minutes);
	debugPrint(' ');
	debugPrintState(currentState);
	//debugPrint(' ');
	//debugPrint(debugString_for);
	//debugPrint(' ');
	debugPrint(':');
	debugPrint((millis()-lastStateChange)/60E3);
	debugPrint(' ');
	debugPrintln(debugString_minutes);
	for(uint8_t protocolIndex = 0; protocolIndex < numberOfActiveProtocols; protocolIndex++)
	{
		debugPrint(debugString_treacleSpace);
		debugPrint("\t");
		debugPrintProtocolName(protocolIndex);
		debugPrint(debugString_SpacedutySpacecycle);
		debugPrint(':');
		debugPrint(protocol[protocolIndex].calculatedDutyCycle);
		debugPrint('%');
		debugPrint(' ');
		debugPrint(debugString_TXcolon);
		debugPrint(protocol[protocolIndex].txPackets);
		debugPrint(' ');
		debugPrint(debugString_TX_drops_colon);
		debugPrint(protocol[protocolIndex].txPacketsDropped);
		debugPrint(' ');
		debugPrint(debugString_RXcolon);
		debugPrint(protocol[protocolIndex].rxPackets);
		debugPrint(' ');
		debugPrint(debugString_RX_drops_colon);
		debugPrintln(protocol[protocolIndex].rxPacketsDropped);
	}
	for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
	{
		debugPrint(debugString_treacleSpace);
		debugPrint("\t");
		debugPrint(debugString_nodeId);
		debugPrint(':');
		debugPrint(node[nodeIndex].id);
		if(node[nodeIndex].name != nullptr)
		{
			debugPrint(' ');
			debugPrint('"');
			debugPrint(node[nodeIndex].name);
			debugPrint('"');
		}
		debugPrintln();
		for(uint8_t protocolIndex = 0; protocolIndex < numberOfActiveProtocols; protocolIndex++)
		{
			debugPrint(debugString_treacleSpace);
			debugPrint("\t\t");
			debugPrintProtocolName(protocolIndex);
			debugPrint(' ');
			debugPrint(debugString_txReliability);
			debugPrint(':');
			debugPrint(100.0*countBits(node[nodeIndex].txReliability[protocolIndex])/16.0);
			debugPrint('%');
			debugPrint(' ');
			debugPrint(debugString_rxReliability);
			debugPrint(':');
			debugPrint(100.0*countBits(node[nodeIndex].rxReliability[protocolIndex])/16.0);
			debugPrint('%');
			debugPrint(' ');
			debugPrint(debugString_payload_numberColon);
			debugPrintln(node[nodeIndex].lastPayloadNumber[protocolIndex]);
			//uint32_t* lastTick = nullptr; 			//This is per protocol
			//uint16_t* nextTick = nullptr; 			//This is per protocol
		}
	}
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
