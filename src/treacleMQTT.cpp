/*
 *	An Arduino library for treacle support
 *
 *	https://github.com/ncmreynolds/treacle
 *
 *	Released under LGPL-2.1 see https://github.com/ncmreynolds/treacle/LICENSE for full license
 *
 */
#ifndef treacleMQTT_cpp
#define treacleMQTT_cpp
#include "treacle.h"
/*
 *
 *	MQTT functions
 *
 */
#if defined(TREACLE_SUPPORT_MQTT)
void treacleClass::setMQTTserver(String server)
{
	setMQTTserver(server.c_str());
}
void treacleClass::setMQTTserver(char* server)
{
	if(server != nullptr)
	{
		if(MQTTserver != nullptr)
		{
			delete MQTTserver;
		}
		MQTTserver = new char[strlen(server) + 1];
		strlcpy(MQTTserver, server, strlen(server) + 1);
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_MQTTspace);
			debugPrint(treacleDebugString_server);
			debugPrint(':');
			debugPrintStringln(MQTTserver);
		#endif
	}
}
void treacleClass::setMQTTserver(IPAddress address)
{
	MQTTserverIp = address;
}
void treacleClass::setMQTTtopic(String topic)
{
	setMQTTtopic(topic.c_str());
}
void treacleClass::setMQTTtopic(char* topic)
{
	if(topic != nullptr)
	{
		if(MQTTtopic != nullptr)
		{
			delete MQTTtopic;
		}
		MQTTtopic = new char[strlen(topic) + 1];
		strlcpy(MQTTtopic, topic, strlen(topic) + 1);
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_MQTTspace);
			debugPrint(treacleDebugString_topic);
			debugPrint(':');
			debugPrintStringln(MQTTtopic);
		#endif
	}
}
void treacleClass::setMQTTport(uint16_t port)
{
	MQTTport = port;
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_MQTTspace);
		debugPrint(treacleDebugString_port);
		debugPrint(':');
		debugPrintln(MQTTport);
	#endif
}
void treacleClass::setMQTTusername(String username)
{
	setMQTTusername(username.c_str());
}
void treacleClass::setMQTTusername(char* username)
{
	if(username != nullptr)
	{
		if(MQTTusername != nullptr)
		{
			delete MQTTusername;
		}
		MQTTusername = new char[strlen(username) + 1];
		strlcpy(MQTTusername, username, strlen(username) + 1);
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_MQTTspace);
			debugPrint(treacleDebugString_username);
			debugPrint(':');
			debugPrintStringln(MQTTusername);
		#endif
	}
}
void treacleClass::setMQTTpassword(String password)
{
	setMQTTpassword(password.c_str());
}
void treacleClass::setMQTTpassword(char* password)
{
	if(password != nullptr)
	{
		if(MQTTpassword != nullptr)
		{
			delete MQTTpassword;
		}
		MQTTpassword = new char[strlen(password) + 1];
		strlcpy(MQTTpassword, password, strlen(password) + 1);
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_treacleSpace);
			debugPrint(treacleDebugString_MQTTspace);
			debugPrint(treacleDebugString_password);
			debugPrint(':');
			debugPrintStringln(MQTTpassword);
		#endif
	}
}
void treacleClass::enableMQTT()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_enablingSpace);
		debugPrintln(treacleDebugString_MQTT);
	#endif
	MQTTTransportId = numberOfActiveTransports++;
}
bool treacleClass::MQTTEnabled()
{
	return MQTTTransportId != 255;	//Not necessarily very useful, but it can be checked
}
bool treacleClass::initialiseMQTT()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_initialisingSpace);
		debugPrint(treacleDebugString_MQTT);
		debugPrint(':');
	#endif
	if(MQTTserver != nullptr || (MQTTserverIp[0] != 0 && MQTTserverIp[1] != 0 && MQTTserverIp[2] != 0 && MQTTserverIp[3] != 0))
	{
		//mqttClient = new WiFiClient;
		mqtt = new PubSubClient(mqttClient);
		if(MQTTserver != nullptr)
		{
			mqtt->setServer(MQTTserver, MQTTport);
		}
		else
		{
			mqtt->setServer(MQTTserverIp, MQTTport);
		}
		mqtt->setCallback(
			[](char* topic, byte* receivedMessage, unsigned int receivedMessageLength)
			{
				#if defined(TREACLE_DEBUG)
					/*
					treacle.debugPrint(PSTR("MQTT message on topic: "));
					treacle.debugPrint(topic);
					treacle.debugPrint(' ');
					for (uint16_t i = 0; i < receivedMessageLength; i++)
					{
						treacle.debugPrint((uint8_t)receivedMessage[i]);
						treacle.debugPrint(' ');
					}
					treacle.debugPrintln();
					*/
				#endif
				if(treacle.receiveBufferSize == 0 && receivedMessageLength < treacle.maximumBufferSize)
				{
					if(receivedMessage[1] != treacle.currentNodeId)	//MQTT will send you your own messages unless configured specifically not to, so ignore them
					{
						treacle.transport[treacle.MQTTTransportId].rxPackets++;						//Count the packet as received
						if(receivedMessage[0] == (uint8_t)treacle.nodeId::allNodes ||
							receivedMessage[0] == treacle.currentNodeId)							//Packet is meaningful to this node
						{
							memcpy(treacle.receiveBuffer, receivedMessage, receivedMessageLength);	//Copy the MQTT payload into the receive buffer
							treacle.receiveBufferSize = receivedMessageLength;						//Record the amount of payload
							treacle.receiveBufferCrcChecked = false;								//Mark the payload as unchecked
							treacle.receiveTransport = treacle.MQTTTransportId;						//Record that it was received by ESP-Now
							treacle.transport[treacle.MQTTTransportId].rxPacketsProcessed++;		//Count the packet as processed
						}
						else
						{
							treacle.transport[treacle.MQTTTransportId].rxPacketsIgnored++;			//Count the ignore
						}
					}
					return;
				}
				else
				{
					treacle.transport[treacle.MQTTTransportId].rxPacketsDropped++;					//Count the drop
				}
			}
		);
		if(true)
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_OK);
			#endif
			transport[MQTTTransportId].initialised = true;					//Mark as initialised
			transport[MQTTTransportId].defaultTick = maximumTickTime/10;	//Set default tick timer
			transport[MQTTTransportId].minimumTick = maximumTickTime/100;	//Set minimum tick timer
		}
		else
		{
			#if defined(TREACLE_DEBUG)
				debugPrintln(treacleDebugString_failed);
			#endif
			transport[MQTTTransportId].initialised = false;				//Mark as not initialised
		}
	}
	else
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_failed);
		#endif
		transport[MQTTTransportId].initialised = false;					//Mark as not initialised
	}
	return transport[MQTTTransportId].initialised;
}
void treacleClass::connectToMQTTserver()
{
	#if defined(TREACLE_DEBUG)
		debugPrint(treacleDebugString_treacleSpace);
		debugPrint(treacleDebugString_initialisingSpace);
		debugPrint(treacleDebugString_MQTTspace);
		debugPrint(treacleDebugString_connectionSpace);
		debugPrint(treacleDebugString_toSpace);
		if(MQTTserver != nullptr)
		{
			debugPrint(MQTTserver);
		}
		else
		{
			debugPrint(MQTTserverIp);
		}
		debugPrint(' ');
	#endif
	if(MQTTtopic == nullptr)
	{
		MQTTtopic = new char[strlen(MQTTdefaultTopic) + 1];
		strlcpy(MQTTtopic, MQTTdefaultTopic, strlen(MQTTdefaultTopic)+1);
	}
	if(mqtt->connect(currentNodeName))
	{
		#if defined(TREACLE_DEBUG)
			debugPrintln(treacleDebugString_OK);
		#endif
		mqtt->subscribe(MQTTtopic);
	}
	else
	{
		#if defined(TREACLE_DEBUG)
			debugPrint(treacleDebugString_failed);
			debugPrintln(mqtt->state());
		#endif
	}
}
bool treacleClass::sendBufferByMQTT(uint8_t* buffer, uint8_t packetSize)
{
	transport[MQTTTransportId].txStartTime = micros();
	if(mqtt->publish(MQTTtopic, buffer, packetSize))
	{
		transport[MQTTTransportId].txTime += micros()			//Add to the total transmit time
			- transport[MQTTTransportId].txStartTime;
		transport[MQTTTransportId].txStartTime = 0;				//Clear the initial send time
		transport[MQTTTransportId].txPackets++;					//Count the packet
		return true;
	}
	transport[MQTTTransportId].txStartTime = 0;
	return false;
}
bool treacleClass::MQTTInitialised()
{
	if(MQTTTransportId != 255)
	{
		return transport[MQTTTransportId].initialised;
	}
	return false;
}
uint32_t treacleClass::getMQTTRxPackets()
{
	if(MQTTInitialised())
	{
		return transport[MQTTTransportId].rxPackets;
	}
	return 0;
}
uint32_t treacleClass::getMQTTTxPackets()
{
	if(MQTTInitialised())
	{
		return transport[MQTTTransportId].txPackets;
	}
	return 0;
}
uint32_t treacleClass::getMQTTRxPacketsDropped()
{
	if(MQTTInitialised())
	{
		return transport[MQTTTransportId].rxPacketsDropped;
	}
	return 0;
}
uint32_t treacleClass::getMQTTTxPacketsDropped()
{
	if(MQTTInitialised())
	{
		return transport[MQTTTransportId].txPacketsDropped;
	}
	return 0;
}
float treacleClass::getMQTTDutyCycle()
{
	if(MQTTInitialised())
	{
		return transport[MQTTTransportId].calculatedDutyCycle;
	}
	return 0;
}
uint32_t treacleClass::getMQTTDutyCycleExceptions()
{
	if(MQTTInitialised())
	{
		return transport[MQTTTransportId].dutyCycleExceptions;
	}
	return 0;
}
void treacleClass::setMQTTTickInterval(uint16_t tick)
{
	transport[MQTTTransportId].defaultTick = tick;
	transport[MQTTTransportId].minimumTick = tick/10;
}
uint16_t treacleClass::getMQTTTickInterval()
{
	if(MQTTInitialised())
	{
		return transport[MQTTTransportId].nextTick;
	}
	return 0;
}
#endif
#endif