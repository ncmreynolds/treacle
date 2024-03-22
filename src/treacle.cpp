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
	debugPrint(debugString_colonSpace);
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
		debugPrint(debugString_colonSpace);
		if(numberOfInitialisedProtocols == numberOfActiveProtocols)
		{
			debugPrintln(debugString_OK);
			debugPrint(debugString_treacleSpace);
			debugPrint(debugString_ActiveSpaceProtocols);
			debugPrint(debugString_colonSpace);
			debugPrintln(numberOfInitialisedProtocols);
			for(uint8_t protocol = 0; protocol < numberOfActiveProtocols; protocol++)
			{
				debugPrint(debugString_treacleSpace);
				debugPrintProtocolName(protocol);
				debugPrint(debugString_SpaceProtocolID);
				debugPrint(debugString_colonSpace);
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
bool treacleClass::noPacketInQueue()
{
	for(uint8_t index = 0; index < numberOfActiveProtocols; index++)
	{
		if(protocol[index].bufferSent == false)			//At least one protocol has not sent the buffer
		{
			return false;
		}
	}
	return true;
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
	debugPrint(debugString_colonSpace);
	if(WiFi.getMode() == WIFI_STA)
	{
		debugPrint(debugString_Client);
	}
	else if(WiFi.getMode() == WIFI_AP)
	{
		debugPrint(debugString_AP);
		debugPrint(' ');
		debugPrint(debugString_channel);
		debugPrint(debugString_colonSpace);
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
		debugPrint(debugString_colonSpace);
		if(WiFi.scanNetworks() > 0)								//A WiFi scan nicely sets everything up without joining a network
		{
			debugPrintln(debugString_OK);
			debugPrint(debugString_treacleSpace);
			debugPrint(debugString_WiFi);
			debugPrint(' ');
			debugPrint(debugString_channel);
			debugPrint(debugString_colonSpace);
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
		debugPrint(debugString_colonSpace);
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
		debugPrint(debugString_colonSpace);
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
		debugPrint(debugString_colonSpace);
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
								//if(receivedMessage[(uint8_t)treacle.headerPosition::recipient] == treacle.currentNodeId ||
								//	receivedMessage[(uint8_t)treacle.headerPosition::recipient] == 0xff)
								treacle.protocol[treacle.espNowProtocolId].rxPackets++;						//Count the packet as received
								if(true)																	//Packet is meaningful to this node
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
	debugPrint(debugString_colonSpace);
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
	debugPrint(debugString_colonSpace);
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
		LoRa.onTxDone(											//Send callback function
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
		protocol[loRaProtocolId].txStartTime = micros();
		if(LoRa.endPacket(true))
		{
			return true;
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
			if(true)													//Packet is meaningful to this node
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
	debugPrint(debugString_colonSpace);
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
	debugPrint(debugString_colonSpace);
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
		currentState = newState;
		lastStateChange = millis();
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_newSpaceState);
		debugPrint(debugString_colonSpace);
		if(currentState == state::uninitialised)
		{
			debugPrintln(debugString_uninitialised);
		}
		else if(currentState == state::starting)
		{
			debugPrintln(debugString_starting);
		}
		else if(currentState == state::selectingId)
		{
			debugPrintln(debugString_selectingId);
		}
		else if(currentState == state::selectedId)
		{
			debugPrintln(debugString_selectedId);
		}
		else if(currentState == state::online)
		{
			debugPrintln(debugString_online);
		}
		else if(currentState == state::offline)
		{
			debugPrintln(debugString_offline);
		}
		else if(currentState == state::stopped)
		{
			debugPrintln(debugString_stopped);
		}
		else
		{
			debugPrintln(debugString_unknown);
		}
		setTickTime();	//States all have their own tick times
	}
}
/*
 *
 *	Tick timers
 *
 */
void treacleClass::setTickTime()
{
	if(currentState == state::uninitialised || currentState == state::starting) return;
	for(uint8_t index = 0; index < numberOfActiveProtocols; index++)
	{
		if(currentState == state::selectingId)
		{
			if(index == espNowProtocolId) protocol[index].nextTick =	1E3;
			else if(index == loRaProtocolId) protocol[index].nextTick =	10E3;
			else if(index == cobsProtocolId) protocol[index].nextTick =	30E3;
		}
		else if(currentState == state::selectedId)
		{
			if(index == espNowProtocolId) protocol[index].nextTick = 	5E3;
			else if(index == loRaProtocolId) protocol[index].nextTick =	30E3;
			else if(index == cobsProtocolId) protocol[index].nextTick =	30E3;
		}
		else if(currentState == state::online)
		{
			if(index == espNowProtocolId) protocol[index].nextTick =	10E3;
			else if(index == loRaProtocolId) protocol[index].nextTick =	45E3;
			else if(index == cobsProtocolId) protocol[index].nextTick =	30E3;
		}
		else if(currentState == state::offline)
		{
			if(index == espNowProtocolId) protocol[index].nextTick =	5E3;
			else if(index == loRaProtocolId) protocol[index].nextTick =	30E3;
			else if(index == cobsProtocolId) protocol[index].nextTick =	30E3;
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
bool treacleClass::sendPacketOnTick()
{
	for(uint8_t index = 0; index < numberOfActiveProtocols; index++)
	{
		if(protocol[index].nextTick != 0 && millis() - protocol[index].lastTick > protocol[index].nextTick)	//nextTick = 0 implies never
		{
			protocol[index].lastTick = millis();		//Update the last tick time
			debugPrint(debugString_treacleSpace);
			debugPrintProtocolName(index);
			debugPrint(' ');
			if(noPacketInQueue())										//Nothing ready to send from the application
			{
				if(currentState == state::selectingId)							//Speed up ID selection by asking existing node IDs
				{
					if(noPacketInQueue())								//Nothing ready to send from the application
					{
						buildIdResolutionRequestPacket(index, currentNodeName);	//Ask about this node with a name->Id request
						debugPrint(debugString_idResolutionRequest);
					}
				}
				else
				{
					if(noPacketInQueue())								//Nothing else is queued to transmit
					{
						for(uint8_t index = 0; index < numberOfNodes; index++)
						{
							if(node[index].name == nullptr && noPacketInQueue())	//This node has no name yet
							{
								buildNameResolutionRequestPacket(index, node[index].id);	//Ask for the node's name with an Id->name request
								debugPrint(debugString_nameResolutionRequest);
							}
						}
					}
					if(noPacketInQueue())					//Nothing else is queued to transmit
					{
						buildKeepalivePacket(index);				//Create an empty tick as a keepalive and announcement of all known nodeIds
						debugPrint(debugString_keepalive);
					}
				}
			}
			else	//There is already data which may or many not already have been sent by other protocols
			{
				debugPrintPayloadTypeDescription(protocol[index].transmitBuffer[(uint8_t)headerPosition::payloadType]);
			}
			debugPrint(debugString_colonSpace);
			if(sendBuffer(index, protocol[index].transmitBuffer, protocol[index].transmitPacketSize))
			{
				debugPrint(protocol[index].transmitPacketSize);
				debugPrint(' ');
				debugPrint(debugString_bytes);
				debugPrint(' ');
				debugPrintln(debugString_sent);
				protocol[index].bufferSent = true;
				return true;
			}
			else
			{
				debugPrintln(debugString_failed);
				return false;
			}
		}
	}
	return false;
}
void treacleClass::timeOutTicks()
{
	uint16_t totalTxReliability = 0x0000;
	uint16_t totalRxReliability = 0x0000;
	for(uint8_t protocol = 0; protocol < numberOfActiveProtocols; protocol++)
	{
		for(uint8_t index = 0; index < numberOfNodes; index++)
		{
			if(millis() - node[index].lastTick[protocol] > node[index].lastTick[protocol] + tickTimeMargin)	//Missed the next window
			{
				node[index].rxReliability[protocol] = node[index].rxReliability[protocol] >> 1;	//Reduce rxReliability
				totalRxReliability = totalRxReliability | node[index].rxReliability[protocol];	//OR all the bits of reliability we have
				node[index].txReliability[protocol] = node[index].txReliability[protocol] >> 1;	//As we've not heard anything to the contrary also reduce txReliability
				totalTxReliability = totalTxReliability | node[index].txReliability[protocol];	//OR all the bits of reliability we have
				node[index].lastTick[protocol] = millis();										//Update last tick timer, even though one was missed
				debugPrint(debugString_treacleSpace);
				debugPrintProtocolName(protocol);
				debugPrint(' ');
				debugPrint(debugString_nodeId);
				debugPrint(debugString_colonSpace);
				debugPrint(node[index].id);
				debugPrint(' ');
				debugPrint(debugString_dropped);
				debugPrint(' ');
				debugPrint(debugString_rxReliability);
				debugPrint(debugString_colonSpace);
				debugPrintln(node[index].rxReliability[protocol]);
			}
		}
	}
	if((totalRxReliability == 0x0000 || totalTxReliability == 0x0000) && currentState == state::online)
	{
		//changeCurrentState(state::offline);
	}
}

/*
 *
 *	Packet packing
 *
 */
void treacleClass::buildStandardPacketHeader(uint8_t protocolId, uint8_t recipient, payloadType type)
{
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::sender] = currentNodeId;						//Add the nodeId
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::recipient] = recipient;						//Make it a 'flood'
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::payloadNumber] = payloadNumber;				//Payload number
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = 0;								//Payload length - starts at 0 and gets updated
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::payloadType] = (uint8_t)type;					//Payload type
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::blockIndex] = 0;								//Large payload start bits 16-23
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::blockIndex+1] = 0;							//Large payload start bits 8-15
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::blockIndex+2] = 0;							//Large payload start bits 0-7
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::nextTick] = (protocol[protocolId].nextTick & 0xff00) >> 8;	//nextTick bits 8-15
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::nextTick+1] = (protocol[protocolId].nextTick & 0x00ff);	//nextTick bits 0-7
	protocol[protocolId].transmitPacketSize = (uint8_t)headerPosition::payload;									//Set the size to just the header
	for(uint8_t index = 0; index < numberOfActiveProtocols; index++)
	{
		protocol[index].bufferSent = false;													//Mark as unsent for each protocol
	}
}
void treacleClass::buildKeepalivePacket(uint8_t protocolId)
{
	buildStandardPacketHeader(protocolId, 0xff, payloadType::keepalive);						//Set payloadType
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
	buildStandardPacketHeader(protocolId, 0xff, payloadType::idResolutionRequest);			//Set payloadType
	protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] = strlen(name);
	memcpy(&protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize], name, strlen(name));
	protocol[protocolId].transmitPacketSize += strlen(name);
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = protocol[protocolId].transmitPacketSize;				//Update packetLength field
	processPacketBeforeTransmission(protocolId);																							//Do CRC and encryption if needed
}
void treacleClass::buildNameResolutionRequestPacket(uint8_t protocolId, uint8_t id)			//Ask for a name from an ID
{
	buildStandardPacketHeader(protocolId, 0xff, payloadType::nameResolutionRequest);			//Set payloadType
	protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] = id;
	protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = protocol[protocolId].transmitPacketSize;				//Update packetLength field
	processPacketBeforeTransmission(protocolId);																							//Do CRC and encryption if needed
}
void treacleClass::buildIdAndNameResolutionResponsePacket(uint8_t protocolId, uint8_t id)		//Supply ID and name
{
	buildStandardPacketHeader(protocolId, 0xff, payloadType::idAndNameResolutionResponse);	//Set payloadType
	protocol[protocolId].transmitBuffer[protocol[protocolId].transmitPacketSize++] = id;												//Add the nodeId
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
	processPacketBeforeTransmission(protocolId);																									//Do CRC and encryption if needed
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
				debugPrint(debugString_colonSpace);
				debugPrint(receiveBuffer[(uint8_t)headerPosition::sender]);
				if(receiveBuffer[(uint8_t)headerPosition::sender] != 0 && receiveBuffer[(uint8_t)headerPosition::sender] != 255)	//Only handle valid node IDs
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
					uint8_t index = indexFromId(receiveBuffer[(uint8_t)headerPosition::sender]);								//Turn node ID into index
					if(node[index].rxReliability[receiveProtocol] != 0xffff)
					{
						debugPrint(' ');
						debugPrint(debugString_rxReliability);
						debugPrint(debugString_colonSpace);
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
							unpackKeepalivePacket(receiveProtocol);	//This might include this node's last used ID after a restart but it is impossible to tell in a keepalive
						}
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::idResolutionRequest)
					{
						unpackIdResolutionRequestPacket(receiveProtocol);
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::nameResolutionRequest)
					{
						unpackNameResolutionRequestPacket(receiveProtocol);
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::idAndNameResolutionResponse)
					{
						unpackIdAndNameResolutionResponsePacket(receiveProtocol);
						clearReceiveBuffer();
					}
					else if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::shortApplicationData)
					{
						debugPrintln(' ');
						//Nothing needed, the application will pick this up
					}
					else
					{
						debugPrint(debugString_unknown);
						debugPrint(debugString_colonSpace);
						debugPrintln(receiveBuffer[(uint8_t)headerPosition::payloadType]);
						clearReceiveBuffer();
					}
				}
				else if(receiveBuffer[(uint8_t)headerPosition::sender] == 0)	//The only likely case is that a node is starting and hasn't picked an ID
				{
					if(receiveBuffer[(uint8_t)headerPosition::payloadType] == (uint8_t)payloadType::idResolutionRequest)
					{
						debugPrint(' ');
						debugPrintln(debugString_idResolutionRequest);
						unpackIdResolutionRequestPacket(receiveProtocol);		//Answer if possible to give back the same ID to a restarted node
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
void treacleClass::unpackKeepalivePacket(uint8_t protocol)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		for(uint8_t index = (uint8_t)headerPosition::payload; index < receiveBuffer[(uint8_t)headerPosition::packetLength]; index++)
		{
			debugPrint(debugString_includes);
			debugPrint(' ');
			debugPrint(debugString_nodeId);
			debugPrint(debugString_colonSpace);
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
				debugPrint(debugString_colonSpace);
				debugPrintln(node[senderIndex].txReliability[protocol]);
				changeCurrentState(state::online);
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
			}
			index+=2;	//Skip to next node ID in keepalive (the loop already does index++)
			debugPrint(' ');
		}
		debugPrintln(' ');
	}
	else
	{
		debugPrintln(' ');
	}
}
void treacleClass::unpackIdResolutionRequestPacket(uint8_t protocol)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		uint8_t nameLength = receiveBuffer[(uint8_t)headerPosition::payload];	
		char nameToLookUp[nameLength + 1];
		strlcpy(nameToLookUp,(const char*)&receiveBuffer[1+(uint8_t)headerPosition::payload],nameLength + 1);	//Copy the name from the packet
		debugPrint(debugString_looking_up);
		debugPrint(debugString_colonSpace);
		debugPrint(nameToLookUp);
		debugPrint(' ');
		uint8_t index = indexFromName(nameToLookUp);
		if(index != maximumNumberOfNodes)
		{
			debugPrint(debugString_nodeId);
			debugPrint(debugString_colonSpace);
			if(noPacketInQueue())
			{
				debugPrint(node[index].id);
				debugPrint(' ');
				debugPrintln(debugString_responding);
				buildIdAndNameResolutionResponsePacket(protocol, node[index].id);
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
void treacleClass::unpackNameResolutionRequestPacket(uint8_t protocol)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		uint8_t id = receiveBuffer[(uint8_t)headerPosition::payload];
		debugPrint(debugString_looking_up);
		debugPrint(' ');
		debugPrint(debugString_nodeId);
		debugPrint(debugString_colonSpace);
		debugPrint(id);
		debugPrint(' ');
		if(id == currentNodeId)	//It's this node, which MUST have a name
		{
			debugPrint(debugString_node_name);
			debugPrint(debugString_colonSpace);
			debugPrint(currentNodeName);
			debugPrint(' ');
			debugPrintln(debugString_this_node);
			if(noPacketInQueue())								//Nothing currently in the queue
			{
				buildIdAndNameResolutionResponsePacket(protocol, id);	//Send a response
			}
		}
		else
		{
			uint8_t index = indexFromId(id);
			if(index != maximumNumberOfNodes && node[index].name != nullptr)	//It's known about and there's a name
			{
				debugPrint(debugString_node_name);
				debugPrint(debugString_colonSpace);
				if(noPacketInQueue())							//Nothing currently in the queue
				{
					debugPrint(node[index].name);
					debugPrint(' ');
					debugPrintln(debugString_responding);
					buildIdAndNameResolutionResponsePacket(protocol, node[index].id);	//Send a response
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
void treacleClass::unpackIdAndNameResolutionResponsePacket(uint8_t protocol)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		uint8_t nameLength = receiveBuffer[1 + (uint8_t)headerPosition::payload];
		char nameReceived[nameLength + 1];
		strlcpy(nameReceived,(const char*)&receiveBuffer[2+(uint8_t)headerPosition::payload],nameLength + 1);
		debugPrint(debugString_received);
		debugPrint(' ');
		debugPrint(nameReceived);
		debugPrint(debugString_colonSpace);
		debugPrint(debugString_nodeId);
		debugPrint(debugString_colonSpace);
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
		node[numberOfNodes].id = id;												//Simple storage of ID
		node[numberOfNodes].lastTick = new uint32_t[numberOfActiveProtocols];		//This is per protocol
		node[numberOfNodes].nextTick = new uint16_t[numberOfActiveProtocols];		//This is per protocol
		node[numberOfNodes].rxReliability = new uint16_t[numberOfActiveProtocols];	//This is per protocol
		node[numberOfNodes].txReliability = new uint16_t[numberOfActiveProtocols];	//This is per protocol
		for(uint8_t index = 0; index < numberOfActiveProtocols; index++)
		{
			node[numberOfNodes].lastTick[index] = 0;								//Clear the values at the start
			node[numberOfNodes].nextTick[index] = 0;
			node[numberOfNodes].rxReliability[index] = 0;
			node[numberOfNodes].txReliability[index] = 0;
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
	for(uint8_t index = 0; index < numberOfActiveProtocols; index++)	//This does not cope with counter rollover!
	{
		protocol[index].calculatedDutyCycle = ((float)protocol[index].txTime/(float)millis())/10.0;	//txTime is in micros so divided by 1000
		debugPrint(debugString_treacleSpace);
		debugPrintProtocolName(index);
		debugPrint(debugString_SpacedutySpacecycle);
		debugPrint(debugString_colonSpace);
		debugPrintln(protocol[index].calculatedDutyCycle);
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
	else
	{
		timeOutTicks();					//Potentially time out ticks from other nodes if they are not responding or the application is slow calling this
	}
	if(currentState == state::selectingId)
	{
		if(applicationDataPacketReceived())
		{
			clearReceiveBuffer();		//Any incoming application data is binned in this state
		}
		else if(millis() - lastStateChange > maximumTickTime + tickTimeMargin)
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
		else if(millis() - lastDutyCycleCheck > dutyCycleCheckInterval)
		{
			lastDutyCycleCheck = millis();
			checkDutyCycle();
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
	if(length < maximumPayloadSize && noPacketInQueue())
	{
		for (uint8_t protocolId = 0; protocolId < numberOfActiveProtocols; protocolId++)
		{
			if(protocol[protocolId].initialised == true)
			{
				buildStandardPacketHeader(protocolId, 0xff, payloadType::shortApplicationData);							//Make an application data packet
				memcpy(&protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::payload], data, length);			//Copy the data starting at headerPosition::payload
				protocol[protocolId].transmitBuffer[(uint8_t)headerPosition::packetLength] = 							//Update packetLength field
				(uint8_t)headerPosition::payload + length;
				protocol[protocolId].transmitPacketSize += length;														//Update the length of the transmit buffer
				processPacketBeforeTransmission(protocolId);															//Do CRC and encryption if needed
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
treacleClass treacle;	//Create an instance of the class, as only one is practically usable at a time
#endif
