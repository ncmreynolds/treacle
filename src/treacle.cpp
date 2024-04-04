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
			debugPrint(debugString_treacleSpace);
			debugPrint(debugString_node_name);
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
	//The name is important so assign one if it is not set. This is based off MAC address on ESP32
	if(currentNodeName == nullptr)
	{
		uint8_t localMacAddress[6];
		WiFi.macAddress(localMacAddress);
		if(espNowEnabled() && loRaEnabled() && cobsEnabled())
		{
			currentNodeName = new char[22];
			sprintf_P(currentNodeName, PSTR("EspNow_LoRa_COBS_%02X%02X"), localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
		}
		else if(espNowEnabled() && loRaEnabled())
		{
			currentNodeName = new char[17];
			sprintf_P(currentNodeName, PSTR("EspNow_LoRa_%02X%02X"), localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
		}
		else if(espNowEnabled()&& cobsEnabled())
		{
			currentNodeName = new char[17];
			sprintf_P(currentNodeName, PSTR("EspNow_COBS_%02X%02X"), localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
		}
		else if(loRaEnabled() && cobsEnabled())
		{
			currentNodeName = new char[15];
			sprintf_P(currentNodeName, PSTR("LoRa_COBS_%02X%02X"), localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
		}
		else if(espNowEnabled())
		{
			currentNodeName = new char[12];
			sprintf_P(currentNodeName, PSTR("EspNow_%02X%02X"), localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
		}
		else if(loRaEnabled())
		{
			currentNodeName = new char[10];
			sprintf_P(currentNodeName, PSTR("LoRa_%02X%02X"), localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
		}
		else if(cobsEnabled())
		{
			currentNodeName = new char[10];
			sprintf_P(currentNodeName, PSTR("COBS_%02X%02X"), localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
		}
		else
		{
			currentNodeName = new char[10];
			sprintf_P(currentNodeName, PSTR("node_%02X%02X"), localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
		}
	}
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_node_name);
		debugPrint(':');
		debugPrintln(currentNodeName);
		debugPrint(debugString_treacleSpace);
		debugPrintln(debugString_starting);
	#endif
	changeCurrentState(state::starting);
	if(numberOfActiveTransports > 0)
	{
		transport = new transportData[numberOfActiveTransports];
		//Initialise all the transports
		uint8_t numberOfInitialisedTransports = 0;
		for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)	//Initialise every transport that is enabled
		{
			if(transportIndex == espNowTransportId)
			{
				transport[transportIndex].initialised = initialiseEspNow();
			}
			else if(transportIndex == loRaTransportId)
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
			else if(transportIndex == cobsTransportId)
			{
				transport[transportIndex].initialised = initialiseCobs();
			}
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
			debugPrint(debugString_treacleSpace);
			debugPrint(debugString_start);
			debugPrint(':');
		#endif
		if(numberOfInitialisedTransports == numberOfActiveTransports)
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(debugString_OK);
				debugPrint(debugString_treacleSpace);
				debugPrint(debugString_ActiveSpaceTransports);
				debugPrint(':');
				debugPrintln(numberOfInitialisedTransports);
				for(uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
				{
					debugPrint(debugString_treacleSpace);
					debugPrintTransportName(transportId);
					debugPrint(debugString_SpaceTransportID);
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
			setTickTime();	//Set initial tick times
			return true;
		}
	}
	#if defined(TREACLE_DEBUG)
		debugPrintln(debugString_failed);
	#endif
	changeCurrentState(state::stopped);
	return false;
}
void treacleClass::end()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrintln(debugString_ended);
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
	if(transportId == espNowTransportId)
	{
		return sendBufferByEspNow(buffer, packetSize);
	}
	else if(transportId == loRaTransportId)
	{
		return sendBufferByLoRa(buffer, packetSize);
	}
	else if(transportId == cobsTransportId)
	{
		return sendBufferByCobs(buffer, packetSize);
	}
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
 *	ESP-Now functions
 *
 */
void treacleClass::enableEspNow()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_enablingSpace);
		debugPrintln(debugString_ESPNow);
	#endif
	espNowTransportId = numberOfActiveTransports++;
}
bool treacleClass::espNowEnabled()
{
	return espNowTransportId != 255;	//Not necessarily very useful, but it can be checked
}
void treacleClass::enableEspNowEncryption()				//Enable encryption for ESP-Now
{
	enableEncryption(espNowTransportId);
}
void treacleClass::disableEspNowEncryption()			//Disable encryption for ESP-Now
{
	disableEncryption(espNowTransportId);
}
bool treacleClass::espNowInitialised()
{
	if(espNowTransportId != 255)
	{
		return transport[espNowTransportId].initialised;
	}
	return false;
}
void treacleClass::setEspNowTickInterval(uint16_t tick)
{
	transport[espNowTransportId].defaultTick = tick;
}
void treacleClass::setEspNowChannel(uint8_t channel)
{
	preferredespNowChannel = channel;							//Sets the preferred channel. It will only be used if practical.
}
uint8_t treacleClass::getEspNowChannel()
{
	return currentEspNowChannel;								//Gets the current channel
}
bool treacleClass::enableEspNowLrMode()
{
	if(WiFi.getMode() == WIFI_STA)
	{
		return esp_wifi_set_protocol( WIFI_IF_STA, WIFI_PROTOCOL_LR ) == ESP_OK;
	}
	else
	/*
	else if(WiFi.getMode() == WIFI_AP)
	{
	}
	else if(WiFi.getMode() == WIFI_AP_STA)
	*/
	{
		return esp_wifi_set_protocol( WIFI_IF_AP, WIFI_PROTOCOL_LR ) == ESP_OK;
	}
}
bool treacleClass::enableEspNow11bMode()
{
	if(WiFi.getMode() == WIFI_STA)
	{
		return esp_wifi_set_protocol( WIFI_IF_STA, WIFI_PROTOCOL_11B ) == ESP_OK;
	}
	else
	{
		return esp_wifi_set_protocol( WIFI_IF_AP, WIFI_PROTOCOL_11B ) == ESP_OK;
	}
}

uint32_t treacleClass::getEspNowRxPackets()
{
	if(espNowInitialised())
	{
		return transport[espNowTransportId].rxPackets;
	}
	return 0;
}
uint32_t treacleClass::getEspNowTxPackets()
{
	if(espNowInitialised())
	{
		return transport[espNowTransportId].txPackets;
	}
	return 0;
}
uint32_t treacleClass::getEspNowRxPacketsDropped()
{
	if(espNowInitialised())
	{
		return transport[espNowTransportId].rxPacketsDropped;
	}
	return 0;
}
uint32_t treacleClass::getEspNowTxPacketsDropped()
{
	if(espNowInitialised())
	{
		return transport[espNowTransportId].txPacketsDropped;
	}
	return 0;
}
float treacleClass::getEspNowDutyCycle()
{
	if(espNowInitialised())
	{
		return transport[espNowTransportId].calculatedDutyCycle;
	}
	return 0;
}
uint32_t treacleClass::getEspNowDutyCycleExceptions()
{
	if(espNowInitialised())
	{
		return transport[espNowTransportId].dutyCycleExceptions;
	}
	return 0;
}
uint16_t treacleClass::getEspNowTickInterval()
{
	if(espNowInitialised())
	{
		return transport[espNowTransportId].nextTick;
	}
	return 0;
}
bool treacleClass::initialiseWiFi()								//Checks to see the state of the WiFi
{
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_checkingSpace);
		debugPrint(debugString_WiFi);
		debugPrint(':');
	#endif
	if(WiFi.getMode() == WIFI_STA)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(debugString_Client);
		#endif
	}
	else if(WiFi.getMode() == WIFI_AP)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(debugString_AP);
			debugPrint(' ');
			debugPrint(debugString_channel);
			debugPrint(':');
			debugPrintln(WiFi.channel());
			debugPrint(' ');
			debugPrintln(debugString_OK);
		#endif
		return true;
	}
	else if(WiFi.getMode() == WIFI_AP_STA)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(debugString_ClientAndAP);
		#endif
	}
	else if(WiFi.getMode() == WIFI_MODE_NULL)
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(debugString_notInitialised);
			debugPrint(debugString_treacleSpace);
			debugPrint(debugString_initialisingSpace);
			debugPrint(debugString_WiFi);
			debugPrint(':');
		#endif
		if(WiFi.scanNetworks() > 0)								//A WiFi scan nicely sets everything up without joining a network
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(debugString_OK);
				debugPrint(debugString_treacleSpace);
				debugPrint(debugString_WiFi);
				debugPrint(' ');
				debugPrint(debugString_channel);
				debugPrint(':');
				debugPrintln(WiFi.channel());
			#endif
			if(WiFi.channel() != preferredespNowChannel)		//Change channel if needed
			{
				changeWiFiChannel(preferredespNowChannel);
			}
			return true;
		}
		else
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(debugString_failed);
			#endif
		}
	}
	else
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(debugString_unknown);
			debugPrint(':');
			debugPrint(WiFi.getMode());
		#endif
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
		#if defined(TREACLE_DEBUG)
			debugPrint(debugString_treacleSpace);
			debugPrint(debugString_WiFi);
			debugPrint(' ');
			debugPrint(debugString_channel);
			debugPrint(' ');
			debugPrint(debugString_changedSpaceTo);
			debugPrint(':');
			debugPrintln(WiFi.channel());
		#endif
		return true;
	}
	return false;
}
bool treacleClass::initialiseEspNow()
{
	if(initialiseWiFi())
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(debugString_treacleSpace);
			debugPrint(debugString_initialisingSpace);
			debugPrint(debugString_ESPNow);
			debugPrint(':');
		#endif
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
								treacle.transport[treacle.espNowTransportId].rxPackets++;						//Count the packet as received
								if(receivedMessage[(uint8_t)treacle.headerPosition::recipient] == (uint8_t)treacle.nodeId::allNodes ||
									receivedMessage[(uint8_t)treacle.headerPosition::recipient] == treacle.currentNodeId)	//Packet is meaningful to this node
								{
									memcpy(&treacle.receiveBuffer,receivedMessage,receivedMessageLength);	//Copy the ESP-Now payload
									treacle.receiveBufferSize = receivedMessageLength;						//Record the amount of payload
									treacle.receiveBufferCrcChecked = false;								//Mark the payload as unchecked
									treacle.receiveTransport = treacle.espNowTransportId;						//Record that it was received by ESP-Now
									treacle.transport[treacle.espNowTransportId].rxPacketsProcessed++;		//Count the packet as processed
								}
							}
							else
							{
								treacle.transport[treacle.espNowTransportId].rxPacketsDropped++;				//Count the drop
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
								if(treacle.transport[treacle.espNowTransportId].txStartTime != 0)				//Check the initial send time was recorded
								{
									treacle.transport[treacle.espNowTransportId].txTime += micros()			//Add to the total transmit time
										- treacle.transport[treacle.espNowTransportId].txStartTime;
									treacle.transport[treacle.espNowTransportId].txStartTime = 0;				//Clear the initial send time
								}
								treacle.transport[treacle.espNowTransportId].txPackets++;						//Count the packet
							}
							else
							{
								treacle.transport[treacle.espNowTransportId].txPacketsDropped++;				//Count the drop
							}
						}
					) == ESP_OK)
					{
						transport[espNowTransportId].initialised = true;
						transport[espNowTransportId].defaultTick = maximumTickTime/5;
						#if defined(TREACLE_DEBUG)
							debugPrintln(debugString_OK);
						#endif
						return true;
					}
				}
			}
		}
	}
	transport[espNowTransportId].initialised = false;
	#if defined(TREACLE_DEBUG)
		debugPrintln(debugString_failed);
	#endif
	return false;
}
bool treacleClass::addEspNowPeer(uint8_t* macaddress)
{
	/*
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_addingSpace);
		debugPrint(debugString_ESPNow);
		debugPrint(debugString_peer);
		debugPrint(':');
	#endif
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
		#if defined(TREACLE_DEBUG)
			//debugPrintln(debugString_OK);
		#endif
		return true;
	}
	#if defined(TREACLE_DEBUG)
		//debugPrintln(debugString_failed);
	#endif
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
	transport[espNowTransportId].txStartTime = micros();
	esp_err_t espNowSendResult = esp_now_send(broadcastMacAddress, buffer, (size_t)packetSize);
	if(espNowSendResult == ESP_OK)
	{
		return true;	//The send callback function records success/fail and txTime from here on
	}
	else
	{
		transport[espNowTransportId].txPacketsDropped++;		//Record the drop
		if(WiFi.channel() != currentEspNowChannel)			//Channel has changed, alter the peer address
		{
			if(deleteEspNowPeer(broadcastMacAddress))		//This could perhaps be changed to modify the existing peer but this should be infrequent
			{
				addEspNowPeer(broadcastMacAddress);
			}
		}
	}
	transport[espNowTransportId].txStartTime = 0;
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
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_enablingSpace);
		debugPrintln(debugString_LoRa);
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
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_initialisingSpace);
		debugPrint(debugString_LoRa);
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
			debugPrintln(debugString_OK);
		#endif
		transport[loRaTransportId].initialised = true;			//Mark as initialised
		transport[loRaTransportId].defaultTick =maximumTickTime;//Set default tick timer
		if(loRaIrqPin != -1)									//Callbacks on IRQ pin
		{
			LoRa.onTxDone(										//Send callback function
				[]() {
					Serial.println("LORA SENT");
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
					Serial.println("LORA RECEIVED");
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
			debugPrintln(debugString_failed);
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
	if(espNowInitialised())
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
				transport[loRaTransportId].rxPacketsProcessed++;			//Count the packet as processed
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
/*
 *
 *	COBS functions
 *
 */
void treacleClass::enableCobs()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_enablingSpace);
		debugPrint(debugString_COBS);
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
bool treacleClass::initialiseCobs()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_initialisingSpace);
		debugPrint(debugString_COBS);
		debugPrint(':');
		debugPrintln(debugString_failed);
	#endif
	transport[cobsTransportId].defaultTick = maximumTickTime - 10;	//Set default tick timer
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
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_selectingSpace);
		debugPrint(debugString_nodeId);
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
		debugPrintln(debugString_failed);
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
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_addingSpace);
		debugPrintln(debugString_encryption_key);
	#endif
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
		#if defined(TREACLE_DEBUG)
			debugPrint(debugString_padded_by);
			debugPrint(' ');
			debugPrint(padding);
			debugPrint(' ');
			debugPrint(debugString_toSpace);
			debugPrint(packetSize - (uint8_t)headerPosition::blockIndex);
			debugPrint(' ');
			debugPrint(debugString_bytes);
			debugPrint(' ');
		#endif
	}
	#if defined(TREACLE_OBFUSCATE_ONLY)
		for(uint8_t bufferIndex = (uint8_t)headerPosition::blockIndex; bufferIndex < packetSize; bufferIndex++)
		{
			buffer[bufferIndex] = buffer[bufferIndex] ^ bufferIndex;	//This is obfuscation only, for testing.
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
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_encrypted);
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
			debugPrint(debugString_padded_by);
			debugPrint(' ');
			debugPrint(packetSize - (buffer[(uint8_t)headerPosition::packetLength] + 2));
			debugPrint(' ');
			debugPrint(debugString_bytes);
			debugPrint(' ');
		#endif
		//packetSize = buffer[(uint8_t)headerPosition::packetLength] + 2;
	}
	//if(packetSize >= (uint8_t)headerPosition::blockIndex && packetSize <= maximumBufferSize - 2)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(debugString_decrypted);
			debugPrint(' ');
		#endif
		#if defined(TREACLE_OBFUSCATE_ONLY)
			for(uint8_t bufferIndex = (uint8_t)headerPosition::blockIndex; bufferIndex < packetSize; bufferIndex++)
			{
				buffer[bufferIndex] = buffer[bufferIndex] ^ bufferIndex;	//This is obfuscation only, for testing.
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
		#if defined(TREACLE_DEBUG)
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
uint16_t treacleClass::minimumTickTime(uint8_t transportId)
{
	if(transportId == espNowTransportId) return	100;
	else if(transportId == loRaTransportId) return 500;
	else if(transportId == cobsTransportId) return 250;
	else return 1000;
}
void treacleClass::setTickTime()
{
	for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)
	{
		transport[transportIndex].nextTick = transport[transportIndex].defaultTick - tickRandomisation(transportIndex);
	}
}
uint16_t treacleClass::tickRandomisation(uint8_t transportId)
{
	return random(minimumTickTime(transportId), minimumTickTime(transportId)*2);
}
void treacleClass::bringForwardNextTick()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_expediting_);
		debugPrintln(debugString_response);
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
	/*
	if(millis() - lastDutyCycleCheck > dutyCycleCheckInterval)
	{
		lastDutyCycleCheck = millis();
		calculateDutyCycle();
	}
	*/
	for(uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
	{
		if(transport[transportId].nextTick != 0 && millis() - transport[transportId].lastTick > transport[transportId].nextTick)	//nextTick = 0 implies never
		{
			transport[transportId].lastTick = millis();									//Update the last tick time
			calculateDutyCycle(transportId);
			#if defined(TREACLE_DEBUG)
				debugPrint(debugString_treacleSpace);
				debugPrintTransportName(transportId);
				debugPrint(' ');
			#endif
			if(transport[transportId].calculatedDutyCycle < transport[transportId].maximumDutyCycle)
			{
				if(packetInQueue(transportId) == false)									//Nothing ready to send from the application for _this_ transport
				{
					if(currentState == state::selectingId)							//Speed up ID selection by asking existing node IDs
					{
						buildIdResolutionRequestPacket(transportId, currentNodeName);		//Ask about this node with a name->Id request
						#if defined(TREACLE_DEBUG)
							debugPrint(debugString_idResolutionRequest);
						#endif
					}
					else
					{
						//if(packetInQueue() == false && transport[transportId].payloadNumber%4 == 0)	//If nothing else is queued to transmit over any transport, 25% of packets can be name requests to backfill names
						if(packetInQueue() == false)	//If nothing else is queued to transmit over any transport backfill names
						{
							for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
							{
								if(node[nodeIndex].name == nullptr && packetInQueue() == false)		//This node has no name yet
								{
									buildNameResolutionRequestPacket(transportId, node[nodeIndex].id);	//Ask for the node's name with an id->name request
									#if defined(TREACLE_DEBUG)
										debugPrint(debugString_nameResolutionRequest);
									#endif
								}
							}
						}
						if(packetInQueue(transportId) == false)				//Nothing else is queued to transmit over _this_ transport, keepalives can go over a higher priority transport with something in a lower one queued
						{
							buildKeepalivePacket(transportId);				//Create an empty tick as a keepalive and announcement of all known nodeIds
							#if defined(TREACLE_DEBUG)
								debugPrint(debugString_keepalive);
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
						debugPrint(debugString_bytes);
						debugPrint(' ');
						debugPrint(debugString_sent);
						debugPrint(' ');
						debugPrint(debugString_toSpace);
						debugPrint(debugString_nodeId);
						debugPrint(':');
						if(transport[transportId].transmitBuffer[0] == (uint8_t)nodeId::allNodes)
						{
							debugPrintln(debugString_all);
						}
						else if(transport[transportId].transmitBuffer[0] == (uint8_t)nodeId::unknownNode)
						{
							debugPrintln(debugString_unknown);
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
						debugPrintln(debugString_failed);
					#endif
					return false;
				}
			}
			else
			{
				transport[transportId].dutyCycleExceptions++;
				#if defined(TREACLE_DEBUG)
					debugPrint(debugString_duty_cycle_exceeded);
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
			if(millis() - node[nodeIndex].lastTick[transportId] > node[nodeIndex].nextTick[transportId] + minimumTickTime(transportId)		//Missed the next window
				&& node[nodeIndex].rxReliability[transportId] > 0																			//Actually has some reliability to begin with
				)
			{
				node[nodeIndex].rxReliability[transportId] = node[nodeIndex].rxReliability[transportId] >> 1;	//Reduce rxReliability
				node[nodeIndex].txReliability[transportId] = node[nodeIndex].txReliability[transportId] >> 1;	//As we've not heard anything to the contrary also reduce txReliability
				node[nodeIndex].lastTick[transportId] = millis();										//Update last tick timer, even though one was missed
				#if defined(TREACLE_DEBUG)
					debugPrint(debugString_treacleSpace);
					debugPrintTransportName(transportId);
					debugPrint(' ');
					debugPrint(debugString_nodeId);
					debugPrint(':');
					debugPrint(node[nodeIndex].id);
					debugPrint(' ');
					debugPrint(debugString_dropped);
					debugPrint(' ');
					debugPrint(debugString_rxReliability);
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
	setTickTime();																								//States and transports all have their own tick times
	if(recipient == (uint8_t)nodeId::unknownNode)
	{
		bringForwardNextTick();																					//Bring forward the next tick ASAP for any starting nodes
	}
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::recipient] = recipient;						//Add the recipient Id
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::sender] = currentNodeId;						//Add the current nodeId
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::payloadType] = (uint8_t)type;					//Payload type
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::payloadNumber] = transport[transportId].payloadNumber++;				//Payload number, which post-increments
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::packetLength] = 0;								//Payload length - starts at 0 and gets updated
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::blockIndex] = 0;								//Large payload start bits 16-23
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::blockIndex+1] = 0;								//Large payload start bits 8-15
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::blockIndex+2] = 0;								//Large payload start bits 0-7
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::nextTick] = (transport[transportId].nextTick & 0xff00) >> 8;	//nextTick bits 8-15
	transport[transportId].transmitBuffer[(uint8_t)headerPosition::nextTick+1] = (transport[transportId].nextTick & 0x00ff);	//nextTick bits 0-7
	transport[transportId].transmitPacketSize = (uint8_t)headerPosition::payload;									//Set the size to just the header
	transport[transportId].bufferSent = false;																	//Mark as unsent for this transport
	/*
	for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)
	{
		transport[transportIndex].bufferSent = false;													//Mark as unsent for each transport
	}
	*/
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
		debugPrint(debugString_treacleSpace);
		debugPrintTransportName(receiveTransport);
		debugPrint(' ');
		debugPrint(debugString_received);
		debugPrint(' ');
		debugPrint(receiveBufferSize);
		debugPrint(' ');
		debugPrint(debugString_bytes);
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
					debugPrint(debugString_fromSpace);
					debugPrint(debugString_nodeId);
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
								debugPrint(debugString_SpacenewCommaadded);
							#endif
						}
						else	//Abandon process, there are already the maximum number of nodes
						{
							#if defined(TREACLE_DEBUG)
								debugPrint(debugString__too_many_nodes);
							#endif
							clearReceiveBuffer();
							return;
						}
					}
					#if defined(TREACLE_DEBUG)
						debugPrint(' ');
					#endif
					uint8_t nodeIndex = nodeIndexFromId(receiveBuffer[(uint8_t)headerPosition::sender]);								//Turn node ID into nodeIndex
					if(receiveTransport == loRaTransportId)
					{
						rssi[nodeIndex] = lastLoRaRssi;																					//Record RSSI if it's a LoRa packet
						snr[nodeIndex] = lastLoRaSNR;																					//Record SNR if it's a LoRa packet
					}
					#if defined(TREACLE_DEBUG)
						debugPrintString(node[nodeIndex].name);
					#endif
					if(receiveBuffer[(uint8_t)headerPosition::payloadNumber] == node[nodeIndex].lastPayloadNumber[receiveTransport])	//Check for duplicate packets
					{
						#if defined(TREACLE_DEBUG)
							debugPrintln(debugString_duplicate);
						#endif
						clearReceiveBuffer();
						return;
					}
					node[nodeIndex].lastPayloadNumber[receiveTransport] = receiveBuffer[(uint8_t)headerPosition::payloadNumber];
					#if defined(TREACLE_DEBUG)
						debugPrint(debugString_payload_numberColon);
						debugPrint(node[nodeIndex].lastPayloadNumber[receiveTransport]);
					#endif
					if(node[nodeIndex].rxReliability[receiveTransport] != 0xffff)
					{
						#if defined(TREACLE_DEBUG)
							debugPrint(' ');
							debugPrint(debugString_rxReliability);
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
							debugPrint(debugString_unknown);
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
							debugPrintln(debugString_idResolutionRequest);
						#endif
						unpackIdResolutionRequestPacket(receiveTransport, senderId);		//Answer if possible to give back the same ID to a restarted node
					}
					clearReceiveBuffer();
				}
			}
			else
			{
				#if defined(TREACLE_DEBUG)
					debugPrintln(debugString_checksum_invalid);
				#endif
				clearReceiveBuffer();
			}
		}
		else
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(debugString_inconsistent);
			#endif
			clearReceiveBuffer();
		}
	}
	else
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(debugString_tooShort);
		#endif
		clearReceiveBuffer();
	}
}
void treacleClass::unpackKeepalivePacket(uint8_t transportId, uint8_t senderId)
{
	if(receiveBuffer[(uint8_t)headerPosition::packetLength] > (uint8_t)headerPosition::payload)
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(debugString_includes);
		#endif
		for(uint8_t bufferIndex = (uint8_t)headerPosition::payload; bufferIndex < receiveBuffer[(uint8_t)headerPosition::packetLength]; bufferIndex++)
		{
			#if defined(TREACLE_DEBUG)
				debugPrint(debugString_treacleSpace);
				debugPrint("\t");
				debugPrint(debugString_nodeId);
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
					debugPrint(debugString_this_node);
					debugPrint(' ');
					debugPrint(debugString_txReliability);
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
								debugPrintln(debugString_SpacenewCommaadded);
							#endif
						}
						else	//Abandon process, there are already the maximum number of nodes
						{
							#if defined(TREACLE_DEBUG)
								debugPrintln(debugString__too_many_nodes);
							#endif
						}
					}
				}
				else
				{
					#if defined(TREACLE_DEBUG)
						debugPrint(' ');
						debugPrint(debugString_txReliability);
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
			debugPrint(debugString_looking_up);
			debugPrint(':');
			debugPrint(nameToLookUp);
			debugPrint(' ');
		#endif
		uint8_t nodeIndex = nodeIndexFromName(nameToLookUp);
		if(nodeIndex != maximumNumberOfNodes)
		{
			#if defined(TREACLE_DEBUG)
				debugPrint(debugString_nodeId);
				debugPrint(':');
			#endif
			if(packetInQueue() == false)
			{
				#if defined(TREACLE_DEBUG)
					debugPrint(node[nodeIndex].id);
					debugPrint(' ');
					debugPrintln(debugString_responding);
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
				debugPrintln(debugString_unknown);
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
			debugPrint(debugString_looking_up);
			debugPrint(' ');
			debugPrint(debugString_nodeId);
			debugPrint(':');
			debugPrint(id);
			debugPrint(' ');
		#endif
		if(id == currentNodeId)	//It's this node, which MUST have a name
		{
			#if defined(TREACLE_DEBUG)
				debugPrint(debugString_node_name);
				debugPrint(':');
				debugPrint(currentNodeName);
				debugPrint(' ');
				debugPrintln(debugString_this_node);
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
					debugPrint(debugString_node_name);
					debugPrint(':');
				#endif
				if(packetInQueue() == false)							//Nothing currently in the queue
				{
					#if defined(TREACLE_DEBUG)
						debugPrint(node[nodeIndex].name);
						debugPrint(' ');
						debugPrintln(debugString_responding);
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
					debugPrintln(debugString_unknown);
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
					debugPrintln(debugString_this_node);
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
					debugPrintln(debugString_received);
					debugPrint(debugString_treacleSpace);
					debugPrint("\t");
					debugPrint(debugString_nodeId);
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
						debugPrintln(debugString_SpacenewCommaadded);
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
		/*
		uint32_t latestTick = 0;
		for(uint8_t transportIndex = 0; transportIndex < numberOfActiveTransports; transportIndex++)
		{
			if(node[nodeIndex].lastTick[transportIndex] > latestTick)
			{
				latestTick = node[nodeIndex].lastTick[transportIndex];
			}
		}
		return millis() - latestTick;
		*/
	}
	return 0;
}
uint32_t treacleClass::rxReliability(uint8_t id)
{
	uint8_t nodeIndex = nodeIndexFromId(id);
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
	return 0;
}
uint32_t treacleClass::txReliability(uint8_t id)
{
	uint8_t nodeIndex = nodeIndexFromId(id);
	if(nodeIndex != maximumNumberOfNodes)
	{
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
uint32_t treacleClass::espNowRxReliability(uint8_t id)
{
	if(espNowInitialised())
	{
		uint8_t nodeIndex = nodeIndexFromId(id);
		if(nodeIndex != maximumNumberOfNodes)
		{
			return node[nodeIndex].rxReliability[espNowTransportId];
		}
	}
	return 0;
}
uint32_t treacleClass::espNowTxReliability(uint8_t id)
{
	if(espNowInitialised())
	{
		uint8_t nodeIndex = nodeIndexFromId(id);
		if(nodeIndex != maximumNumberOfNodes)
		{
			return node[nodeIndex].txReliability[espNowTransportId];
		}
	}
	return 0;
}
uint32_t treacleClass::loRaRxReliability(uint8_t id)
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
uint32_t treacleClass::loRaTxReliability(uint8_t id)
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
		if(debug_uart_ != nullptr && millis() - lastStatusMessage > 60E3)
		{
			lastStatusMessage = millis();
			showStatus();
		}
	#endif
	if(loRaTransportId != 255 && transport[loRaTransportId].initialised == true && loRaIrqPin == -1)	//Polling method for loRa packets, must be enabled and initialised
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
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_message);
		debugPrint(' ');
		debugPrintln(debugString_cleared);
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
					debugPrint(debugString_treacleSpace);
					debugPrint(debugString_suggested_message_interval);
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
			debugPrint(debugString_treacleSpace);
			debugPrint(debugString_suggested_message_interval);
			debugPrint(' ');
			debugPrint((transport[0].nextTick * 2)/1000);
			debugPrintln('s');
		#endif
		return transport[0].nextTick * 2;
	}
	#if defined(TREACLE_DEBUG)
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_suggested_message_interval);
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
				packetInQueue(transportId) == false) 		//It's got nothing waiting to go
			{
				buildPacketHeader(transportId, (uint8_t)nodeId::allNodes, payloadType::shortApplicationData);			//Make an application data packet
				memcpy(&transport[transportId].transmitBuffer[(uint8_t)headerPosition::payload], data, length);			//Copy the data starting at headerPosition::payload
				transport[transportId].transmitBuffer[(uint8_t)headerPosition::packetLength] = 							//Update packetLength field
				(uint8_t)headerPosition::payload + length;
				transport[transportId].transmitPacketSize += length;														//Update the length of the transmit buffer
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
		debugPrint(debugString_treacleSpace);
		debugPrint(debugString_nodeId);
		debugPrint(':');
		debugPrint(currentNodeId);
		debugPrint(' ');
		debugPrintString(currentNodeName);
		debugPrint(' ');
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
		for(uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
		{
			debugPrint(debugString_treacleSpace);
			debugPrint("\t");
			debugPrintTransportName(transportId);
			debugPrint(debugString_SpacedutySpacecycle);
			debugPrint(':');
			debugPrint(transport[transportId].calculatedDutyCycle);
			debugPrint('%');
			debugPrint(' ');
			debugPrint(debugString_TXcolon);
			debugPrint(transport[transportId].txPackets);
			debugPrint(' ');
			debugPrint(debugString_TX_drops_colon);
			debugPrint(transport[transportId].txPacketsDropped);
			debugPrint(' ');
			debugPrint(debugString_RXcolon);
			debugPrint(transport[transportId].rxPackets);
			debugPrint(' ');
			debugPrint(debugString_RX_drops_colon);
			debugPrintln(transport[transportId].rxPacketsDropped);
		}
		for(uint8_t nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
		{
			debugPrint(debugString_treacleSpace);
			debugPrint("\t");
			debugPrint(debugString_nodeId);
			debugPrint(':');
			debugPrint(node[nodeIndex].id);
			debugPrint(' ');
			debugPrintStringln(node[nodeIndex].name);
			for(uint8_t transportId = 0; transportId < numberOfActiveTransports; transportId++)
			{
				debugPrint(debugString_treacleSpace);
				debugPrint("\t\t");
				debugPrintTransportName(transportId);
				debugPrint(' ');
				debugPrint(debugString_txReliability);
				debugPrint(':');
				debugPrint(reliabilityPercentage(node[nodeIndex].txReliability[transportId]));
				debugPrint('%');
				debugPrint(' ');
				debugPrint(debugString_rxReliability);
				debugPrint(':');
				debugPrint(reliabilityPercentage(node[nodeIndex].rxReliability[transportId]));
				debugPrint('%');
				debugPrint(' ');
				debugPrint(debugString_payload_numberColon);
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
