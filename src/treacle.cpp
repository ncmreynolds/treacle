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

bool treacleClass::begin()
{
	debugPrint(debugString_treacleSpace);
	debugPrintln(debugString_starting);
	changeCurrentState(state::starting);
	if(numberOfActiveProtocols > 0)
	{
		//Allocate heap for the data structures rather than allocating it statically
		protocolEnabled =		new bool[numberOfActiveProtocols];
		memset(protocolEnabled,0,numberOfActiveProtocols*sizeof(bool));
		
		protocolInitialised =	new bool[numberOfActiveProtocols];
		memset(protocolInitialised,0,numberOfActiveProtocols*sizeof(bool));
		
		txPackets =				new uint32_t[numberOfActiveProtocols];
		memset(txPackets,0,numberOfActiveProtocols*sizeof(uint32_t));
		
		txPacketsDropped =		new uint32_t[numberOfActiveProtocols];
		memset(txPacketsDropped,0,numberOfActiveProtocols*sizeof(uint32_t));
		
		rxPackets =				new uint32_t[numberOfActiveProtocols];
		memset(rxPackets,0,numberOfActiveProtocols*sizeof(uint32_t));
		
		rxPacketsDropped =		new uint32_t[numberOfActiveProtocols];
		memset(rxPacketsDropped,0,numberOfActiveProtocols*sizeof(uint32_t));
		
		txStartTime =			new uint32_t[numberOfActiveProtocols];
		memset(txStartTime,0,numberOfActiveProtocols*sizeof(uint32_t));
		
		TxTime =				new uint32_t[numberOfActiveProtocols];
		memset(TxTime,0,numberOfActiveProtocols*sizeof(uint32_t));
		
		calculatedDutyCycle =	new float[numberOfActiveProtocols];
		memset(calculatedDutyCycle,0,numberOfActiveProtocols*sizeof(float));
		
		maximumDutyCycle =		new float[numberOfActiveProtocols];
		memset(maximumDutyCycle,0,numberOfActiveProtocols*sizeof(float));
		
		lastTick = 				new uint32_t[numberOfActiveProtocols];
		memset(lastTick,0,numberOfActiveProtocols*sizeof(uint32_t));
		
		nextTick = 				new uint16_t[numberOfActiveProtocols];
		memset(nextTick,0,numberOfActiveProtocols*sizeof(uint16_t));
		
		bufferSent =			new bool[numberOfActiveProtocols];
		memset(bufferSent,0,numberOfActiveProtocols*sizeof(bool));
		
		//These buffers could perhaps be statically allocated as they are usually the same
		transmitBuffer = 		new uint8_t[maximumBufferSize];
		receiveBuffer = 		new uint8_t[maximumBufferSize];
		//Initialise all the protocols
		uint8_t numberOfInitialisedProtocols = 0;
		for(uint8_t protocol = 0; protocol < numberOfActiveProtocols; protocol++)
		{
			if(protocol == espNowProtocolId)
			{
				protocolInitialised[protocol] = initialiseEspNow();
				if(protocolInitialised[protocol])
				{
					numberOfInitialisedProtocols++;
				}
			}
			else if(protocol == loRaProtocolId)
			{
				protocolInitialised[protocol] = initialiseLoRa();
				if(protocolInitialised[protocol])
				{
					numberOfInitialisedProtocols++;
				}
			}
			else if(protocol == cobsProtocolId)
			{
				protocolInitialised[protocol] = initialiseCobs();
				if(protocolInitialised[protocol])
				{
					numberOfInitialisedProtocols++;
				}
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
				if(protocol == espNowProtocolId)
				{
					debugPrint(debugString_treacleSpace);
					debugPrint(debugString_ESPNow);
					debugPrint(debugString_SpaceProtocolID);
					debugPrint(debugString_colonSpace);
					debugPrintln(protocol);
				}
				else if(protocol == loRaProtocolId)
				{
					debugPrint(debugString_treacleSpace);
					debugPrint(debugString_LoRa);
					debugPrint(debugString_SpaceProtocolID);
					debugPrint(debugString_colonSpace);
					debugPrintln(protocol);
				}
				else if(protocol == cobsProtocolId)
				{
					debugPrint(debugString_treacleSpace);
					debugPrint(debugString_COBS);
					debugPrint(debugString_SpaceProtocolID);
					debugPrint(debugString_colonSpace);
					debugPrintln(protocol);
				}
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
bool treacleClass::espNowInitialised()
{
	return protocolInitialised[espNowProtocolId];
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
		if(esp_now_init() == ESP_OK)
		{
			esp_now_peer_info_t newPeer;
			newPeer.peer_addr[0] = broadcastMacAddress[0];
			newPeer.peer_addr[1] = broadcastMacAddress[1];
			newPeer.peer_addr[2] = broadcastMacAddress[2];
			newPeer.peer_addr[3] = broadcastMacAddress[3];
			newPeer.peer_addr[4] = broadcastMacAddress[4];
			newPeer.peer_addr[5] = broadcastMacAddress[5];
			if(WiFi.getMode() == WIFI_STA)
			{
				newPeer.ifidx = WIFI_IF_STA;
			}
			else if(WiFi.getMode() == WIFI_AP_STA)
			{
				newPeer.ifidx = WIFI_IF_AP;
			}
			else if(WiFi.getMode() == WIFI_AP)
			{
				if(WiFi.channel() != preferredespNowChannel)
				{
					changeWiFiChannel(preferredespNowChannel);
				}
				newPeer.ifidx = WIFI_IF_AP;
			}
			else
			{
				protocolInitialised[espNowProtocolId] = false;
				debugPrintln(debugString_WiFiSpacenotSpaceenabled);
				return false;
			}
			newPeer.channel = WiFi.channel();
			currentEspNowChannel = newPeer.channel;
			if(esp_now_add_peer(&newPeer) == ESP_OK)
			{
				if(esp_now_register_recv_cb(	//ESP-Now receive function
					[](const uint8_t *macAddress, const uint8_t *receivedMessage, int receivedMessageLength)
					{
						treacle.debugPrint(treacle.debugString_ESPNow);
						treacle.debugPrint(' ');
						treacle.debugPrint(treacle.debugString_packetSpace);
						if(treacle.receivePayloadSize == 0)	//Check the receive buffer is empty
						{
							memcpy(&treacle.receiveBuffer,receivedMessage,receivedMessageLength);
							treacle.receivePayloadSize = receivedMessageLength;
							treacle.rxPackets[treacle.espNowProtocolId]++;
							treacle.debugPrintln(treacle.debugString_received);
						}
						else
						{
							treacle.rxPacketsDropped[treacle.espNowProtocolId]++;
							treacle.debugPrintln(treacle.debugString_dropped);
						}
					}
				) == ESP_OK)
				{
					protocolInitialised[espNowProtocolId] = true;
					debugPrintln(debugString_OK);
					return true;
				}
			}
		}
	}
	protocolInitialised[espNowProtocolId] = false;
	debugPrintln(debugString_failed);
	return false;
}
bool treacleClass::sendBufferByEspNow(uint8_t* buffer, uint8_t payloadSize)
{
	if(appendChecksumToPacket(buffer, payloadSize)) //Add the checksum, if there is space
	{
		//espNowPacketSent = millis();
		esp_err_t espNowSendResult = esp_now_send(broadcastMacAddress, buffer, (size_t)payloadSize);
		if(espNowSendResult == ESP_OK)
		{
			txPackets[espNowProtocolId]++;
			return true;
		}
		else
		{
			txPacketsDropped[espNowProtocolId]++;
			/*
			if(WiFi.channel() != espNowChannel) //Channel has changed, move peer address
			{
				if(deleteBroadcastPeer())
				{
					addBroadcastPeer();
				}
			}
			*/
		}
		//espNowPacketSent = 0;
	}
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
		protocolInitialised[loRaProtocolId] = true;				//Mark as initialised
	}
	else
	{
		debugPrintln(debugString_failed);
		protocolInitialised[loRaProtocolId] = false;			//Mark as not initialised
	}
	return protocolInitialised[loRaProtocolId];
}
bool treacleClass::loRaInitialised()
{
	return protocolInitialised[loRaProtocolId];
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
	return protocolInitialised[cobsProtocolId];
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
	//bool idIsUnique = false;
	//while(idIsUnique == false && millis() - lastStateChange < 5000)	//Check against any received ID, this will die after 5s if none work out
	{
		currentNodeId = random(minimumNodeId,maximumNodeId);
		//idIsUnique = checkNodeIdIsUnique(currentNodeId);
	}
	//if(idIsUnique == true)
	{
		debugPrintln(currentNodeId);
		return true;
	}
	debugPrintln(debugString_failed);
	return false;
}
bool treacleClass::checkNodeIdIsUnique(uint8_t id)
{
	return true;
}
/*
 *
 *	Checksums
 *
 */
bool treacleClass::appendChecksumToPacket(uint8_t* buffer, uint8_t& payloadSize)
{
	if(payloadSize < maximumBufferSize - 2)	//The CRC will expand the packet by two bytes
	{
		CRC16 crc(treaclePolynome);
		crc.add((uint8_t *)buffer, payloadSize);
		uint16_t packetCrc = crc.calc();
		buffer[payloadSize++] = (packetCrc & 0xff00) >> 8;	//Append this to the packet
		buffer[payloadSize++] = packetCrc & 0xff;
		return true;
	}
	return false;
}
bool treacleClass::validatePacketChecksum(uint8_t* buffer, uint8_t& payloadSize)
{
	CRC16 crc(treaclePolynome);
	crc.add((uint8_t *)buffer, payloadSize - 2);
	uint16_t expectedChecksum = crc.calc();
	uint16_t packetChecksum = (buffer[payloadSize - 2] << 8) + buffer[payloadSize - 1];
	if(expectedChecksum == packetChecksum)
	{
		payloadSize -= 2;
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
/*
 *
 *	Tick timers
 *
 */
void treacleClass::setTickTime()
{
	for(uint8_t protocol = 0; protocol < numberOfActiveProtocols; protocol++)
	{
		if(currentState == state::uninitialised || currentState == state::starting)
		{
			//Do nothing as none of the data structures are initialised yet
		}
		else if(currentState == state::selectingId)
		{
			if(protocol == espNowProtocolId) nextTick[protocol] =		1E3;
			else if(protocol == loRaProtocolId) nextTick[protocol] =	10E3;
			else if(protocol == cobsProtocolId) nextTick[protocol] =	30E3;
		}
		else if(currentState == state::selectedId)
		{
			if(protocol == espNowProtocolId) nextTick[protocol] = 		5E3;
			else if(protocol == loRaProtocolId) nextTick[protocol] =	30E3;
			else if(protocol == cobsProtocolId) nextTick[protocol] =	30E3;
		}
		else if(currentState == state::online)
		{
			if(protocol == espNowProtocolId) nextTick[protocol] =		10E3;
			else if(protocol == loRaProtocolId) nextTick[protocol] =	45E3;
			else if(protocol == cobsProtocolId) nextTick[protocol] =	30E3;
		}
		else if(currentState == state::offline)
		{
			if(protocol == espNowProtocolId) nextTick[protocol] =		5E3;
			else if(protocol == loRaProtocolId) nextTick[protocol] =	30E3;
			else if(protocol == cobsProtocolId) nextTick[protocol] =	30E3;
		}
		else if(currentState == state::stopped)
		{
			nextTick[protocol] =										0;		//Never send
		}
		else
		{
			if(protocol == espNowProtocolId) nextTick[protocol] =		maximumTickTime;
			else if(protocol == loRaProtocolId) nextTick[protocol] =	maximumTickTime;
			else if(protocol == cobsProtocolId) nextTick[protocol] =	maximumTickTime;
		}
	}
}
bool treacleClass::sendPacketOnTick()
{
	for(uint8_t protocol = 0; protocol < numberOfActiveProtocols; protocol++)
	{
		if(nextTick[protocol] != 0 && millis() - lastTick[protocol] > nextTick[protocol])	//nextTick = 0 implies never
		{
			lastTick[protocol] = millis();		//Update the last tick time
			if(protocol == espNowProtocolId)	//Manage ESP-Now packets
			{
				debugPrint(debugString_ESPNow);
				debugPrint(' ');
				if(transmitPayloadSize == 0)	//Nothing ready to send from the application
				{
					debugPrint(debugString_tick);
					buildEmptyTickPacket(protocol);	//Create an empty tick as a keepalive and NodeId announcement
					appendChecksumToPacket(transmitBuffer, transmitPayloadSize);
				}
				else	//There is already data which may or many not already have been sent by other protocols
				{
					debugPrint(debugString_data);
				}
				debugPrint(debugString_colonSpace);
				if(sendBufferByEspNow(transmitBuffer, transmitPayloadSize))
				{
					debugPrintln(debugString_sent);
					transmitPayloadSize = 0;	//Succesful send, mark it as such
					return true;
				}
				else
				{
					debugPrintln(debugString_failed);
					transmitPayloadSize = 0;	//Failed to send
					return false;
				}
			}
			else if(protocol == loRaProtocolId)
			{
				debugPrint(debugString_LoRa);
				debugPrint(' ');
				debugPrint(debugString_tick);
				debugPrint(debugString_colonSpace);
				debugPrintln(debugString_sent);
				return true;
			}
			else if(protocol == cobsProtocolId)
			{
				debugPrint(debugString_COBS);
				debugPrint(' ');
				debugPrint(debugString_tick);
				debugPrint(debugString_colonSpace);
				debugPrintln(debugString_sent);
				return true;
			}
		}
	}
	return false;
}
void treacleClass::buildEmptyTickPacket(uint8_t protocol)
{
	transmitPayloadSize = 0;											//Reset to start of buffer
	transmitBuffer[transmitPayloadSize++] = currentNodeId;				//Add the nodeId
	transmitBuffer[transmitPayloadSize++] = 0xff;						//Make it a 'flood'
	transmitBuffer[transmitPayloadSize++] = (packetNumber & 0xff00)>>8;	//MSB of packet number
	transmitBuffer[transmitPayloadSize++] = (packetNumber & 0x00ff);	//LSB of packet number
	packetNumber++;
}
/*
 *
 *	Messaging functions
 *
 */ 
bool treacleClass::online()
{
	return false;
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
	return receivePayloadSize != 0;	//Check the buffer payload
}
void treacleClass::clearReceiveBuffer()
{
	receivePayloadSize = 0;	//Clear the buffer payload
}
uint32_t treacleClass::messageWaiting()
{
	if(currentState == state::uninitialised || currentState == state::starting || currentState == state::stopped)
	{
		if(packetReceived())
		{
			clearReceiveBuffer();
		}
		return 0;	//Nothing can be sent or received
	}
	else if(currentState == state::selectingId)
	{
		if(packetReceived())
		{
			clearReceiveBuffer();
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
	else if(currentState == state::selectedId || currentState == state::online || currentState == state::offline)
	{
		if(packetReceived())
		{
			clearReceiveBuffer();
		}
		else if(sendPacketOnTick() == true)	//Send a packet if it needs to happen now
		{
			return 0;	//A tick has been sent
		}
	}
	return 0;
}
uint8_t treacleClass::messageSender()
{
	return 0;
}
bool treacleClass::sendMessage()
{
	return false;
}
treacleClass treacle;	//Create an instance of the class, as only one is practically usable at a time
#endif
