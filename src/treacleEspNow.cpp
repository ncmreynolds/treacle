/*
 *	An Arduino library for treacle support
 *
 *	https://github.com/ncmreynolds/treacle
 *
 *	Released under LGPL-2.1 see https://github.com/ncmreynolds/treacle/LICENSE for full license
 *
 */
#ifndef treacleEspNow_cpp
#define treacleEspNow_cpp
#include "treacle.h"
/*
 *
 *	ESP-Now functions
 *
 */
#if defined(TREACLE_SUPPORT_ESPNOW)
void treacleClass::enableEspNow()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_enablingSpace);
		debugPrintln(treacleDebugString_ESPNow);
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
	transport[espNowTransportId].minimumTick = tick/10;
}
void treacleClass::setEspNowChannel(uint8_t channel)
{
	preferredespNowChannel = channel;							//Sets the preferred channel. It will only be used if practical.
}
uint8_t treacleClass::getEspNowChannel()
{
	return currentEspNowChannel;								//Gets the current channel
}
#if defined(ESP32)
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
#endif
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
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_checkingSpace);
		debugPrint(treacleDebugString_WiFi);
		debugPrint(':');
	#endif
	if(WiFi.getMode() == WIFI_STA)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_Client);
		#endif
		return true;
	}
	else if(WiFi.getMode() == WIFI_AP)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_AP);
			debugPrint(' ');
			debugPrint(treacleDebugString_channel);
			debugPrint(':');
			debugPrintln(WiFi.channel());
			debugPrint(' ');
			debugPrintln(treacleDebugString_OK);
		#endif
		return true;
	}
	else if(WiFi.getMode() == WIFI_AP_STA)
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_ClientAndAP);
		#endif
		return true;
	}
	#if defined(ESP8266)
	else if(WiFi.getMode() == WIFI_OFF)
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_notInitialised);
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_initialisingSpace);
			debugPrint(treacleDebugString_WiFi);
			debugPrint(':');
		#endif
		wl_status_t status = WiFi.begin();
		if(status == WL_IDLE_STATUS || status == WL_CONNECTED)
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_OK);
				debugPrint(treacleDebugString_treacleSpace);
				debugPrint(treacleDebugString_WiFi);
				debugPrint(' ');
				debugPrint(treacleDebugString_channel);
				debugPrint(':');
				debugPrintln(WiFi.channel());
			#endif
			return true;
		}
		else
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_failed);
			#endif
		}
	}
	#elif defined(ESP32)
	else if(WiFi.getMode() == WIFI_MODE_NULL)
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_notInitialised);
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_initialisingSpace);
			debugPrint(treacleDebugString_WiFi);
			debugPrint(':');
		#endif
		if(WiFi.scanNetworks() >= 0)							//A WiFi scan nicely sets everything up without joining a network
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_OK);
				debugPrint(treacleDebugString_treacleSpace);
				debugPrint(treacleDebugString_WiFi);
				debugPrint(' ');
				debugPrint(treacleDebugString_channel);
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
				debugPrintln(treacleDebugString_failed);
			#endif
		}
	}
	#endif
	else
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_unknown);
			debugPrint('/');
			debugPrint(WiFi.getMode());
			debugPrint(' ');
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
				#if defined(TREACLE_DEBUG)
					debugPrintln(F("\n\rUnable to set country to JP for channel 14 use"));
				#endif
				return false;
			}
		#elif defined ESP32
		#endif
	}
	#if defined(ESP8266)
	#elif defined ESP32
		if(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE) == ESP_OK)
		{
			#if defined(TREACLE_DEBUG)
				debugPrint(treacleDebugString_treacleSpace);
				debugPrint(treacleDebugString_WiFi);
				debugPrint(' ');
				debugPrint(treacleDebugString_channel);
				debugPrint(' ');
				debugPrint(treacleDebugString_changedSpaceTo);
				debugPrint(':');
				debugPrintln(WiFi.channel());
			#endif
			return true;
		}
	#endif
	return false;
}
#if defined(ESP8266)
void treacleEsp8266receiveCallbackWrapper(uint8_t *macAddress, uint8_t *receivedMessage, uint8_t receivedMessageLength)
{
	treacle.esp8266receiveCallback(macAddress, receivedMessage, receivedMessageLength);
}
void treacleClass::esp8266receiveCallback(uint8_t *macAddress, uint8_t *receivedMessage, uint8_t receivedMessageLength)
{
	if(currentState != state::starting)	//Must not receive packets before the buffers are allocated
	{
		if(receiveBufferSize == 0 && receivedMessageLength < maximumBufferSize)	//Check the receive buffer is empty first
		{
			transport[espNowTransportId].rxPackets++;						//Count the packet as received
			if(receivedMessage[(uint8_t)headerPosition::recipient] == (uint8_t)nodeId::allNodes ||
				receivedMessage[(uint8_t)headerPosition::recipient] == currentNodeId)	//Packet is meaningful to this node
			{
				memcpy(&receiveBuffer,receivedMessage,receivedMessageLength);	//Copy the ESP-Now payload
				receiveBufferSize = receivedMessageLength;						//Record the amount of payload
				receiveBufferCrcChecked = false;								//Mark the payload as unchecked
				receiveTransport = espNowTransportId;							//Record that it was received by ESP-Now
				transport[espNowTransportId].rxPacketsProcessed++;				//Count the packet as processed
			}
			else
			{
				transport[espNowTransportId].rxPacketsIgnored++;			//Count the ignore
			}
		}
		else
		{
			transport[espNowTransportId].rxPacketsDropped++;				//Count the drop
		}
	}
}
void treacleEsp8266sendCallbackWrapper(uint8_t* macAddress, uint8_t status)	//ESP-Now send callback is used to measure airtime for duty cycle calculations
{
	treacle.esp8266sendCallback(macAddress, status);
}
void treacleClass::esp8266sendCallback(uint8_t* macAddress, uint8_t status)	//ESP-Now send callback is used to measure airtime for duty cycle calculations
{
	if(status == ESP_OK)
	{
		if(transport[espNowTransportId].txStartTime != 0)				//Check the initial send time was recorded
		{
			transport[espNowTransportId].txTime += micros()			//Add to the total transmit time
				- transport[espNowTransportId].txStartTime;
			transport[espNowTransportId].txStartTime = 0;				//Clear the initial send time
		}
		transport[espNowTransportId].txPackets++;						//Count the packet
	}
	else
	{
		transport[espNowTransportId].txPacketsDropped++;				//Count the drop
	}
}
#endif
bool treacleClass::initialiseEspNow()
{
	if(initialiseWiFi())
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_initialisingSpace);
			debugPrint(treacleDebugString_ESPNow);
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
				#if defined(ESP8266)
				esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
				if(esp_now_register_recv_cb(treacleEsp8266receiveCallbackWrapper) == ESP_OK)	//ESP-Now receive callback
				{
					if(esp_now_register_send_cb(treacleEsp8266sendCallbackWrapper) == ESP_OK)	//ESP-Now send callback
					{
						transport[espNowTransportId].initialised = true;
						#if defined(TREACLE_DEBUG)
							debugPrintln(treacleDebugString_OK);
						#endif
					}
				}
				#elif defined(ESP32)
				#if ESP_IDF_VERSION_MAJOR >= 5
				if(esp_now_register_recv_cb(
					[](const esp_now_recv_info *rcvinfo, const uint8_t *receivedMessage, int receivedMessageLength)
					{
						uint8_t *macAddress = rcvinfo->src_addr;
				#elif ESP_IDF_VERSION_MAJOR > 3
				if(esp_now_register_recv_cb(	//ESP-Now receive callback
					[](const uint8_t *macAddress, const uint8_t *receivedMessage, int receivedMessageLength)
					{
				#endif
						if(treacle.currentState != treacle.state::starting)	//Must not receive packets before the buffers are allocated
						{
							if(treacle.receiveBufferSize == 0 && receivedMessageLength < treacle.maximumBufferSize)	//Check the receive buffer is empty first
							{
								treacle.transport[treacle.espNowTransportId].rxPackets++;					//Count the packet as received
								if(receivedMessage[(uint8_t)treacle.headerPosition::recipient] == (uint8_t)treacle.nodeId::allNodes ||
									receivedMessage[(uint8_t)treacle.headerPosition::recipient] == treacle.currentNodeId)	//Packet is meaningful to this node
								{
									memcpy(&treacle.receiveBuffer,receivedMessage,receivedMessageLength);	//Copy the ESP-Now payload
									treacle.receiveBufferSize = receivedMessageLength;						//Record the amount of payload
									treacle.receiveBufferCrcChecked = false;								//Mark the payload as unchecked
									treacle.receiveTransport = treacle.espNowTransportId;					//Record that it was received by ESP-Now
									treacle.transport[treacle.espNowTransportId].rxPacketsProcessed++;		//Count the packet as processed
								}
								else
								{
									treacle.transport[treacle.espNowTransportId].rxPacketsIgnored++;		//Count the ignore
								}
							}
							else
							{
								treacle.transport[treacle.espNowTransportId].rxPacketsDropped++;			//Count the drop
							}
						}
					}
				) == ESP_OK)
				{
					#if ESP_IDF_VERSION_MAJOR >= 5
					if(esp_now_register_send_cb(															//ESP-Now send callback
						[](const uint8_t* macAddress, esp_now_send_status_t status)							//ESP-Now send callback is used to measure airtime for duty cycle calculations
					#elif ESP_IDF_VERSION_MAJOR > 3
					if(esp_now_register_send_cb(															//ESP-Now send callback
						[](const uint8_t* macAddress, esp_now_send_status_t status)							//ESP-Now send callback is used to measure airtime for duty cycle calculations
					#endif
						{
							if(status == ESP_OK)
							{
								if(treacle.transport[treacle.espNowTransportId].txStartTime != 0)			//Check the initial send time was recorded
								{
									treacle.transport[treacle.espNowTransportId].txTime += micros()			//Add to the total transmit time
										- treacle.transport[treacle.espNowTransportId].txStartTime;
									treacle.transport[treacle.espNowTransportId].txStartTime = 0;			//Clear the initial send time
								}
								treacle.transport[treacle.espNowTransportId].txPackets++;					//Count the packet
							}
							else
							{
								treacle.transport[treacle.espNowTransportId].txPacketsDropped++;			//Count the drop
							}
						}
					) == ESP_OK)
					{
						transport[espNowTransportId].initialised = true;
						#if defined(TREACLE_DEBUG)
							debugPrintln(treacleDebugString_OK);
						#endif
					}
				}
				#endif
			}
		}
		if(transport[espNowTransportId].initialised == true)
		{
			transport[espNowTransportId].defaultTick = maximumTickTime/10;
			transport[espNowTransportId].minimumTick = maximumTickTime/100;
			return true;
		}
	}
	transport[espNowTransportId].initialised = false;
	#if defined(TREACLE_DEBUG)
		debugPrintln(treacleDebugString_failed);
	#endif
	return transport[espNowTransportId].initialised;
}
bool treacleClass::addEspNowPeer(uint8_t* macaddress)
{
	/*
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_addingSpace);
		debugPrint(treacleDebugString_ESPNow);
		debugPrint(treacleDebugString_peer);
		debugPrint(':');
	#endif
	*/
	#if defined(ESP8266)
	if(esp_now_add_peer(macaddress, ESP_NOW_ROLE_COMBO, currentEspNowChannel, NULL, 0) == ESP_OK)
	{
		#if defined(TREACLE_DEBUG)
			//debugPrintln(treacleDebugString_OK);
		#endif
		return true;
	}
	#elif defined(ESP32)
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
			//debugPrintln(treacleDebugString_OK);
		#endif
		return true;
	}
	#endif
	#if defined(TREACLE_DEBUG)
		//debugPrintln(treacleDebugString_failed);
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
	
	#if defined(ESP8266)
	int8_t espNowSendResult = esp_now_send(broadcastMacAddress, buffer, (size_t)packetSize);
	#elif defined(ESP32)
	esp_err_t espNowSendResult = esp_now_send(broadcastMacAddress, buffer, (size_t)packetSize);
	#endif
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
uint16_t treacleClass::espNowRxReliability(uint8_t id)
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
uint16_t treacleClass::espNowTxReliability(uint8_t id)
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
#endif
#endif