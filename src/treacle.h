/*
 *	An Arduino library for treacle support
 *
 *	https://github.com/ncmreynolds/treacle
 *
 *	Released under LGPL-2.1 see https://github.com/ncmreynolds/treacle/LICENSE for full license
 *
 */
#ifndef treacle_h
#define treacle_h
#include <Arduino.h>
#include <ArduinoUniqueID.h>

#if !defined(AVR)
	#define TREACLE_DEBUG
#endif

//#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_MICRO)
//#if defined(AVR)
//	#define ARDUINO_AVR
//#endif

#if defined(ESP8266) || defined(ESP32)
	#define TREACLE_SUPPORT_ESPNOW
#endif
#if defined(AVR) || defined(ESP8266) || defined(ESP32)
	#define TREACLE_SUPPORT_LORA
#endif
#if defined(AVR) || defined(ESP8266) || defined(ESP32)
	#define TREACLE_SUPPORT_UDP
#endif
#if defined(AVR) || defined(ESP8266) || defined(ESP32)
	#define TREACLE_SUPPORT_MQTT
#endif
#define TREACLE_SUPPORT_COBS
//#define TREACLE_DEBUG_COBS

#define TREACLE_ENCRYPT_WITH_CBC
//#define TREACLE_ENCRYPT_WITH_EAX

#if defined(TREACLE_ENCRYPT_WITH_CBC)
	#if defined(ESP32)
		#include <aes/esp_aes.h>
	#else
		#include <CryptoAES_CBC.h>
		#include <AES.h>
		#include <CBC.h>
	#endif
#endif

#if defined(TREACLE_SUPPORT_ESPNOW)
	#if defined(ESP8266)
		#include <ESP8266WiFi.h>
		#include <espnow.h>
		#define ESP_OK 0
	#elif defined(ESP32)
		#include <WiFi.h>
		#include <esp_wifi.h> //Only needed for esp_wifi_set_channel()
		extern "C"
		{
			#include <esp_now.h>
		}
	#endif
#endif

#if defined(TREACLE_SUPPORT_UDP)
	#if defined(ESP8266)
		#include <ESP8266WiFi.h>
		#include <WiFiUdp.h>
	#elif defined(ESP32)
		#include <AsyncUDP.h>
	#elif defined(AVR)
		#include <EthernetUdp.h>
	#endif
#endif

#if defined(TREACLE_SUPPORT_MQTT)
	#if defined(ESP8266) || defined(ESP32)
		#include <PubSubClient.h>
	#elif defined(AVR)
		#include <Ethernet.h>
		#include <PubSubClient.h>
	#endif
#endif

#if defined(TREACLE_SUPPORT_LORA)
	#include <SPI.h>
	#include <LoRa.h>
#endif

#include "CRC16.h" //A CRC16 is used to check the packet is LIKELY to be sent in a known format
#include "CRC.h"

#if defined(TREACLE_DEBUG)
	const char treacleDebugString_treacleSpace[] PROGMEM = {"treacle "};
	const char treacleDebugString_starting[] PROGMEM = "starting";
	const char treacleDebugString_start[] PROGMEM = "start";
	const char treacleDebugString_ended[] PROGMEM = "ended";
	const char treacleDebugString_enablingSpace[] PROGMEM = "enabling ";
	const char treacleDebugString_checkingSpace[] PROGMEM = "checking ";
	const char treacleDebugString_initialisingSpace[] PROGMEM = "initialising ";
	const char treacleDebugString_notInitialised[] PROGMEM = "not initialised";
	const char treacleDebugString_selectingSpace[] PROGMEM = "selecting ";
	const char treacleDebugString_OK[] PROGMEM = "OK";
	const char treacleDebugString_unknown[] PROGMEM = "unknown";
	const char treacleDebugString_failed[] PROGMEM = "failed";
	const char treacleDebugString_WiFi[] PROGMEM = "WiFi";
	const char treacleDebugString_Client[]  = "Client";
	const char treacleDebugString_AP[] PROGMEM = "AP";
	const char treacleDebugString_ClientAndAP[] PROGMEM = "Client & AP";
	#if defined(TREACLE_SUPPORT_ESPNOW)
		const char treacleDebugString_ESPNow[] PROGMEM = "ESP-Now";
	#endif
	const char treacleDebugString_channel[] PROGMEM = "channel";
	const char treacleDebugString_changedSpaceTo[] PROGMEM = "changed to";
	const char treacleDebugString_WiFiSpacenotSpaceenabled[] PROGMEM = "WiFi not enabled";
	#if defined(TREACLE_SUPPORT_LORA)
		const char treacleDebugString_LoRa[] PROGMEM = "LoRa";
	#endif
	#if defined(TREACLE_SUPPORT_COBS)
		const char treacleDebugString_COBS[] PROGMEM = "COBS";
	#endif
	const char treacleDebugString_newSpaceState[] PROGMEM = "new state";
	const char treacleDebugString_state[] PROGMEM = "state";
	const char treacleDebugString_uninitialised[] PROGMEM = "uninitialised";
	const char treacleDebugString_selectingId[] PROGMEM = "selectingId";
	const char treacleDebugString_selectedId[] PROGMEM = "selectedId";
	const char treacleDebugString_online[] PROGMEM = "online";
	const char treacleDebugString_offline[] PROGMEM = "offline";
	const char treacleDebugString_stopped[] PROGMEM = "stopped";
	const char treacleDebugString_nodeId[] PROGMEM = "nodeId";
	const char treacleDebugString_tick[] PROGMEM = "tick";
	const char treacleDebugString_keepalive[] PROGMEM = "keepalive";
	const char treacleDebugString_short_application_data[] PROGMEM = "short application data";
	const char treacleDebugString_sent[] PROGMEM = "sent";
	const char treacleDebugString_received[] PROGMEM = "received";
	const char treacleDebugString_toSpace[] PROGMEM = "to ";
	const char treacleDebugString_fromSpace[] PROGMEM = "from ";
	const char treacleDebugString_ActiveSpaceTransports[] PROGMEM = "active transports";
	const char treacleDebugString_SpaceTransportID[] PROGMEM = " transport ID";
	const char treacleDebugString_packetSpace[] PROGMEM = "packet ";
	const char treacleDebugString_dropped[] PROGMEM = "dropped";
	const char treacleDebugString_addingSpace[] PROGMEM = "adding ";
	const char treacleDebugString_deletingSpace[] PROGMEM = "deleting ";
	const char treacleDebugString_peer[] PROGMEM = "peer";
	const char treacleDebugString_SpacedutySpacecycle[] PROGMEM = " duty cycle";
	const char treacleDebugString_tooShort[] PROGMEM = "too short";
	const char treacleDebugString_inconsistent[] PROGMEM = "inconsistent";
	const char treacleDebugString_bytes[] PROGMEM = "bytes";
	const char treacleDebugString_SpacenewCommaadded[] PROGMEM = " new, added";
	const char treacleDebugString__too_many_nodes[] PROGMEM = " too many nodes";
	const char treacleDebugString_includes[] PROGMEM = "includes";
	const char treacleDebugString_checksum_invalid[] PROGMEM = "checksum invalid";
	const char treacleDebugString_this_node[] PROGMEM = "this node";
	const char treacleDebugString_node_name[] PROGMEM = "node name";
	const char treacleDebugString_idResolutionRequest[] PROGMEM = "name->ID request";
	const char treacleDebugString_nameResolutionRequest[] PROGMEM = "ID->name request";
	const char treacleDebugString_nameResolutionResponse[] PROGMEM = "ID->name response";
	const char treacleDebugString_looking_up[] PROGMEM = "looking up";
	const char treacleDebugString_responding[] PROGMEM = "responding";
	const char treacleDebugString_rxReliability[] PROGMEM = "rxReliability";
	const char treacleDebugString_txReliability[] PROGMEM = "txReliability";
	const char treacleDebugString_message[] PROGMEM = "message";
	const char treacleDebugString_cleared[] PROGMEM = "cleared";
	const char treacleDebugString_padded_by[] PROGMEM = "padded by";
	const char treacleDebugString_encrypted[] PROGMEM = "encrypted";
	const char treacleDebugString_decrypted[] PROGMEM = "decrypted";
	const char treacleDebugString_encryption_key[] PROGMEM = "encryption key";
	const char treacleDebugString_duplicate[] PROGMEM = "duplicate";
	const char treacleDebugString_payload_numberColon[] PROGMEM = "payload number:";
	const char treacleDebugString_after[] PROGMEM = "after";
	const char treacleDebugString_minutes[] PROGMEM = "minutes";
	const char treacleDebugString_expediting_[] PROGMEM = "expediting ";
	const char treacleDebugString_for[] PROGMEM = "for";
	const char treacleDebugString_response[] PROGMEM = "response";
	const char treacleDebugString_all[] PROGMEM = "all";
	const char treacleDebugString_nodes[] PROGMEM = "nodes";
	const char treacleDebugString_reached[] PROGMEM = "reached";
	const char treacleDebugString_with[] PROGMEM = "with";
	const char treacleDebugString_duty_cycle_exceeded[] PROGMEM = "duty cycle exceeded";
	const char treacleDebugString_TX[] PROGMEM = "TX";
	const char treacleDebugString_RX[] PROGMEM = "RX";
	const char treacleDebugString_drops_colon[] PROGMEM = " drops:";
	const char treacleDebugString_invalid_colon[] PROGMEM = " invalid:";
	const char treacleDebugString_ignored_colon[] PROGMEM = " ignored:";
	const char treacleDebugString_up[] PROGMEM = "up";
	const char treacleDebugString_suggested_message_interval[] PROGMEM = "suggested message interval";
	#if defined(TREACLE_SUPPORT_MQTT)
		const char treacleDebugString_MQTT[] PROGMEM = "MQTT";
		const char treacleDebugString_MQTTspace[] PROGMEM = "MQTT ";
		const char treacleDebugString_connectionSpace[] PROGMEM = "connection ";
		const char treacleDebugString_server[] PROGMEM = "server";
		const char treacleDebugString_topic[] PROGMEM = "topic";
		const char treacleDebugString_username[] PROGMEM = "username";
		const char treacleDebugString_password[] PROGMEM = "password";
	#endif
	#if defined(TREACLE_SUPPORT_UDP)
		const char treacleDebugString_UDP[] PROGMEM = "UDP";
		const char treacleDebugString_UDPspace[] PROGMEM = "UDP ";
	#endif
	#if defined(TREACLE_SUPPORT_MQTT) || defined(TREACLE_SUPPORT_MQTT)
		const char treacleDebugString_port[] PROGMEM = "port";
	#endif
#endif

class treacleClass	{

	public:
		treacleClass();										//Constructor function
		~treacleClass();									//Destructor function
		//ESP-Now
		#if defined(TREACLE_SUPPORT_ESPNOW)
			void enableEspNow();							//Enable the ESP-Now radio
			bool espNowEnabled();							//Is ESP-Now radio enabled?
			#if defined(ESP32)
			bool enableEspNow11bMode();
			bool enableEspNowLrMode();
			#endif
			void enableEspNowEncryption();					//Enable encryption for ESP-Now
			void disableEspNowEncryption();					//Disable encryption for ESP-Now
			bool espNowInitialised();						//Is ESP-Now radio correctly initialised?
			void setEspNowChannel(uint8_t);					//Set the WiFi channel used for ESP-Now
			bool espNowChannelChanged();					//Check if the ESP-Now channel changed, resets on read if true
			uint8_t getEspNowChannel();						//Get the WiFi channel used for ESP-Now
			void setEspNowTickInterval(uint16_t tick);		//Set the ESP-Now tick interval
			uint32_t getEspNowRxPackets();					//Get packet stats
			uint32_t getEspNowTxPackets();					//Get packet stats
			uint32_t getEspNowRxPacketsDropped();			//Get packet stats
			uint32_t getEspNowTxPacketsDropped();			//Get packet stats
			float getEspNowDutyCycle();						//Get packet stats
			uint32_t getEspNowDutyCycleExceptions();		//Get packet stats
			uint16_t getEspNowTickInterval();				//Get time between packets
			uint16_t espNowRxReliability(uint8_t);
			uint16_t espNowTxReliability(uint8_t);
			#if defined(ESP8266)
			void esp8266sendCallback(uint8_t* macAddress,	//ESP-Now send callback is used to measure airtime for duty cycle calculations
				uint8_t status);
			void esp8266receiveCallback(uint8_t *macAddress,
				uint8_t *receivedMessage, uint8_t receivedMessageLength);
			#endif
		#endif
		//LoRa
		#if defined(TREACLE_SUPPORT_LORA)
			void setLoRaPins(int8_t cs, int8_t reset,		//Set the GPIO for the LoRa radio
				int8_t irq = -1);
			void setLoRaFrequency(uint32_t mhz);			//Set the LoRa frequency
			void enableLoRa();								//Enable the LoRa radio
			bool loRaEnabled();								//Is LoRa radio enabled?
			bool loRaInitialised();							//Is LoRa radio correctly initialised?
			uint32_t getLoRaRxPackets();					//Get packet stats
			uint32_t getLoRaTxPackets();					//Get packet stats
			uint32_t getLoRaRxPacketsDropped();				//Get packet stats
			uint32_t getLoRaTxPacketsDropped();				//Get packet stats
			float getLoRaDutyCycle();						//Get packet stats
			uint32_t getLoRaDutyCycleExceptions();			//Get packet stats
			void setLoRaTickInterval(uint16_t tick);		//Set the LoRa tick interval
			uint16_t getLoRaTickInterval();					//Get time between packets
			uint8_t getLoRaTxPower();						//LoRa TX power
			uint8_t getLoRaSpreadingFactor();				//LoRa spreading factor
			uint32_t getLoRaSignalBandwidth();				//Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3(default), 250E3, and 500E3.
			void setLoRaTxPower(uint8_t);					//LoRa TX power 2-20
			void setLoRaSpreadingFactor(uint8_t);			//LoRa spreading factor
			void setLoRaSignalBandwidth(uint32_t);			//Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3(default), 250E3, and 500E3.
			void setLoRaRxGain(uint8_t);					//0-6, 0 = auto
			uint16_t loRaRxReliability(uint8_t);
			uint16_t loRaTxReliability(uint8_t);
			int16_t  loRaRSSI(uint8_t);
			float    loRaSNR(uint8_t);
		#endif
		//MQTT
		#if defined(TREACLE_SUPPORT_MQTT)
			void enableMQTT();								//Enable MQTT
			bool MQTTEnabled();								//Is MQTT enabled?
			void setMQTTserver(char*);						//Set the MQTT server name
			void setMQTTserver(String);						//Set the MQTT server
			void setMQTTserver(IPAddress);					//Set the MQTT server IP address (IPv4)
			void setMQTTport(uint16_t);						//Set the MQTT server, if not default
			void setMQTTtopic(char*);						//Set the MQTT base topic, if not default (/treacle)
			void setMQTTtopic(String);						//Set the MQTT base topic, if not default (/treacle)
			void setMQTTusername(char*);					//Set the MQTT username
			void setMQTTusername(String);					//Set the MQTT username
			void setMQTTpassword(char*);					//Set the MQTT password
			void setMQTTpassword(String);					//Set the MQTT password
			bool MQTTInitialised();							//Is MQTT correctly initialised?
			uint32_t getMQTTRxPackets();					//Get packet stats
			uint32_t getMQTTTxPackets();					//Get packet stats
			uint32_t getMQTTRxPacketsDropped();				//Get packet stats
			uint32_t getMQTTTxPacketsDropped();				//Get packet stats
			float getMQTTDutyCycle();						//Get packet stats
			uint32_t getMQTTDutyCycleExceptions();			//Get packet stats
			void setMQTTTickInterval(uint16_t tick);		//Set the interval between packets
			uint16_t getMQTTTickInterval();					//Get interval between packets
		#endif
		//UDP
		#if defined(TREACLE_SUPPORT_UDP)
			void enableUDP();								//Enable UDP
			bool UDPEnabled();								//Is UDP enabled?
			bool UDPInitialised();							//Is UDP correctly initialised?
			void setUDPMulticastAddress(					//Set the multicast address to use (default 224.0.1.38)
				IPAddress address);
			void setUDPport(uint16_t);						//Set the UDP port (default 47625)
			uint32_t getUDPRxPackets();						//Get packet stats
			uint32_t getUDPTxPackets();						//Get packet stats
			uint32_t getUDPRxPacketsDropped();				//Get packet stats
			uint32_t getUDPTxPacketsDropped();				//Get packet stats
			float getUDPDutyCycle();						//Get packet stats
			uint32_t getUDPDutyCycleExceptions();			//Get packet stats
			void setUDPTickInterval(uint16_t tick);			//Set the interval between packets
			uint16_t getUDPTickInterval();					//Get interval between packets
		#endif
		//COBS/Serial
		#if defined(TREACLE_SUPPORT_COBS)
			void enableCobs();
			bool cobsEnabled();
			bool cobsInitialised();
			void setCobsStream(Stream &);
			void setCobsTickInterval(uint16_t tick);
		#endif
		//Messaging
		bool online();										//Is treacle online? ie. has this node heard back from a peer that has heard it recently
		void forceOffline();								//Actively force the node offline
		void goOnline();									//Go online (if possible and this will take a few seconds)
		uint8_t nodes();									//Number of nodes
		bool nodesChanged();								//Inform application if number of nodes has changed, resets on read if true
		uint8_t reachableNodes();							//Number of reachable nodes
		bool reachableNodesChanged();						//Inform application if number of reachable nodes has changed, resets on read if true
		uint8_t maxPayloadSize();							//Maximum single packet payload size
		uint32_t messageWaiting();							//Is there a message waiting?
		void clearWaitingMessage();							//Trash an incoming message
		uint8_t messageSender();							//The sender of the waiting message
		uint32_t suggestedQueueInterval();					//Suggest a delay before the next message
		bool queueMessage(char*);							//Queue a short message
		bool queueMessage(uint8_t*, uint8_t);				//Queue a short message
		bool queueMessage(const unsigned char*,				//Queue a short message
			uint8_t);
		bool sendMessage(char*);							//Send a short message ASAP
		bool sendMessage(uint8_t*, uint8_t);				//Send a short message ASAP
		bool sendMessage(const unsigned char*,				//Send a short message ASAP
			uint8_t);
		bool retrieveWaitingMessage(uint8_t*);				//Retrieve a message. The buffer must be large enough for it, no checking can be done
		//Encryption
		void setEncryptionKey(uint8_t* key);				//Set the encryption key
		//General
		void setNodeName(char* name);						//Set the node name
		char* getNodeName();								//Get the node name
		char* getNodeNameFromId(uint8_t id);				//Get the node name of another node from its ID
		char* getNodeNameFromIndex(uint8_t index);			//Get the node name of another node from its index
		void setNodeId(uint8_t id);							//Set the nodeId
		bool nodeIdChanged();								//Find out if the node name changed, resets on read if true
		uint8_t getNodeId();								//Get the nodeId, which may have been autonegotiated
		uint8_t getNodeId(uint8_t index);					//Get the nodeId of another node based on index
		//Transport stats
		uint8_t numberOfTransports();						//Return number of transports
		const char* transportName(uint8_t index);			//Printable transport name
		uint32_t getRxPackets(uint8_t index);				//Get transport stats
		uint32_t getTxPackets(uint8_t index);				//Get transport stats
		uint32_t getRxPacketsProcessed(uint8_t index);		//Get transport stats
		uint32_t getRxPacketsDropped(uint8_t index);		//Get transport stats
		uint32_t getTxPacketsDropped(uint8_t index);		//Get transport stats
		float getDutyCycle(uint8_t index);					//Get transport stats
		float getMaxDutyCycle(uint8_t index);				//Get transport stats
		//Node status & stats
		bool online(uint8_t);								//Is a specific treacle node online? ie. has this node heard from it recently
		//uint32_t rxAge(uint8_t);
		uint16_t nodeRxReliability(uint8_t index);							//Get node stats
		uint16_t nodeTxReliability(uint8_t index);							//Get node stats
		uint32_t nodeLastSeen(uint8_t index);								//Get node stats
		uint32_t nodeLastTick(uint8_t index, uint8_t transport);			//Get node stats
		uint16_t nodeNextTick(uint8_t index, uint8_t transport);			//Get node stats
		uint16_t nodeTxReliability(uint8_t index, uint8_t transport);		//Get node stats
		uint16_t nodeRxReliability(uint8_t index, uint8_t transport);		//Get node stats
		uint8_t  nodeLastPayloadNumber(uint8_t index, uint8_t transport);	//Get node stats
		//Start, stop and debug
		bool begin(uint8_t maxNodes = 8);					//Start treacle, optionally specify a max number of nodes
		void end();											//Stop treacle
		void enableDebug(Stream &);							//Start debugging on a stream
		void disableDebug();								//Stop debugging
		bool debugEnabled();								//Check if debug is enabled
	protected:
	private:
		//State machine
		enum class state : uint8_t {uninitialised,			//State tracking
			starting,
			selectingId,
			selectedId,
			online,
			offline,
			forcedOffline,
			stopped};
		state currentState = state::uninitialised;			//Current state
		uint32_t lastStateChange = 0;						//Track time of state changes
		void changeCurrentState(state);						//Change state and track the time of change
		
		//Transport information
		uint8_t numberOfActiveTransports = 0;				//Used to track transport IDs

		//Transmit packet buffers
		static const uint8_t maximumBufferSize= 250;		//Maximum buffer size, which is based off ESP-Now max size
		static const uint8_t maximumPayloadSize = 238;		//Maximum application payload size, which is based off ESP-Now max size
		
		//Ticks
		static const uint16_t maximumTickTime = 60E3;		//Absolute longest time something can be scheduled in the future
		//Tick functions
		void setNextTickTime();								//Set a next tick time for all transports, done at startup
		void setNextTickTime(uint8_t);						//Set a next tick time immediately before sending for a specific transport
		uint16_t tickRandomisation(uint8_t);				//Random factor for timing
		void bringForwardNextTick();						//Hurry up the tick time for urgent things
		bool sendPacketOnTick();							//Send a single packet if it is due, returns true if this happens
		void timeOutTicks();								//Potentially time out ticks from other nodes if they stop responding

		struct transportData
		{
			bool initialised = false;						//Has the transport initialised OK?
			bool encrypted = false;							//Is the transport encrypted?
			uint32_t txPackets = 0;							//Simple stats for successfully transmitted packets
			uint32_t txPacketsDropped = 0;					//Simple stats for failed transmit packets
			uint32_t rxPackets = 0;							//Simple stats for successfully received packets
			uint32_t rxPacketsProcessed = 0;				//Simple stats for successfully received packets that were passed on for processing
			uint32_t rxPacketsDropped = 0;					//Simple stats for received packets that were dropped, probably due to a full buffer
			uint32_t rxPacketsIgnored = 0;					//Simple stats for received packets that were ignored, probably due to being for another node
			uint32_t rxPacketsInvalid = 0;					//Simple stats for received packets that were invalid, probably due to a wrong encryption key
			uint32_t txStartTime = 0;						//Used to calculate TX time for each packet using micros()
			uint32_t txTime = 0;							//Total time in micros() spent transmitting
			float calculatedDutyCycle = 0;					//Calculated from txTime and millis()
			float maximumDutyCycle = 1;						//Used as a hard brake on TX if exceeded
			uint32_t dutyCycleExceptions = 0;				//Count any time it goes over duty cycle
			uint32_t lastTick = 0;							//Track this node's ticks
			uint16_t defaultTick = maximumTickTime;			//Frequency of ticks for each transport, which is important
			uint16_t minimumTick = maximumTickTime/2;		//Minimum frequency of ticks for each transport, which is important
			uint16_t nextTick = 0;							//How long until the next tick for each transport, which is important. This varies slightly from the default.
			uint8_t transmitBuffer[maximumBufferSize];		//General transmit buffer
			uint8_t transmitPacketSize = 0;					//Current transmit packet size
			bool bufferSent = true;							//Per transport marker for when something is sent
			uint8_t payloadNumber = 0;						//Sequence number for payloads, this will overflow regularly
		};
		transportData* transport = nullptr;					//This will be allocated from heap during begin()
		
		
		//Node information
		uint8_t maximumNumberOfNodes = 8;					//Expected max number of nodes
		static const uint8_t absoluteMaximumNumberOfNodes = 80;	//Absolute max number of nodes
		uint8_t numberOfNodes = 0;							//Track number of nodes
		bool numberOfNodesChanged = false;					//Flag to show application if nodes have changed, resets on read if true
		uint8_t numberOfReachableNodes = 0;					//Track number of reachable nodes
		bool numberOfReachableNodesChanged = false;			//Flag to show application if nodes have changed, resets on read if true
		struct nodeInfo
		{
			uint8_t id = 0;
			char* name = nullptr;
			uint32_t lastSeen = 0;
			uint32_t* lastTick = nullptr; 					//This is per transport
			uint16_t* nextTick = nullptr; 					//This is per transport
			uint16_t* txReliability = nullptr;				//This is per transport
			uint16_t* rxReliability = nullptr;				//This is per transport
			uint8_t* lastPayloadNumber = nullptr;			//This is per transport
		};
		nodeInfo* node;										//Chunky struct could overwhelm a small microcontroller, so be careful with maxNodes
		//Node management functions
		bool nodeExists(uint8_t id);						//Check if a node ID exists
		uint8_t nodeIndexFromId(uint8_t id);				//Get an index into nodeInfo from a node ID
		bool addNode(uint8_t id,							//Create a node. Default to excellent symmetric reliability
			uint16_t reliability = 0xffff);					//Create a node with symmetric reliability
		uint8_t nodeIndexFromName(char* name);				//Get an index into nodeInfo from a node name
		
		//Node ID management
		char* currentNodeName = nullptr;					//Everything has a name, don't use numerical addresses
		bool currentNodeIdChanged = false;					//Flag to show application if node ID has changed
		uint8_t currentNodeId = 0;							//Current node ID, 0 implies not set
		static const uint8_t minimumNodeId = 1;				//Lowest a node ID can be
		static const uint8_t maximumNodeId = 126;			//Highest a node ID can be
		bool selectNodeId();								//Select a node ID for this node
		void calculateNumberOfReachableNodes();				//Track how many nodes are currently reachable
		
		//Duty cycle monitoring
		void calculateDutyCycle(uint8_t);					//Calculate the duty cycle for a specific transport based off current txTime, done just before sending
		
		//Receive packet buffers
		uint8_t receiveBuffer[maximumBufferSize];			//General receive buffer
		uint8_t receiveBufferSize = 0;						//Current receive payload size
		uint8_t receiveTransport = 0;						//Transport that received the packet
		bool receiveBufferDecrypted = false;				//Has the decryption been done?
		bool receiveBufferCrcChecked = false;				//Has the CRC been checked and removed?
		//Packet receiving functions
		bool packetReceived();								//Check for a packet in the buffer
		bool applicationDataPacketReceived();				//Check for an application data packet in the buffer
		void clearReceiveBuffer();							//Clear the receive buffer

		//Packet encoding/decoding
		enum class payloadType:uint8_t{						//These are all a bit TBC
			keepalive =						0x00,			//Lower four bits are a numeric type, making for 16 types
			idResolutionRequest =			0x01,			//Upper four bits are a bitmask
			nameResolutionRequest =			0x02,
			idAndNameResolutionResponse =	0x03,
			duplicateId =					0x04,
			shutdown =						0x05,
			//shortApplicationData =		0x06,
			//idAndNameResolutionResponse =	0x07,
			shortApplicationData =			0x08,
			//idAndNameResolutionResponse =	0x09,
			//idAndNameResolutionResponse =	0x0a,
			//idAndNameResolutionResponse =	0x0b,
			//idAndNameResolutionResponse =	0x0c,
			//idAndNameResolutionResponse =	0x0d,
			//idAndNameResolutionResponse =	0x0e,
			//idAndNameResolutionResponse =	0x0f,
			//These below are bitmask flags
			encrypted =						0x10
			//encrypted =					0x20
			//encrypted =					0x40
			//encrypted =					0x80
			};
		enum class nodeId:uint8_t{							//These are all a bit TBC
			unknownNode =					0x00,
			allNodes =						0xff
		};
		enum class headerPosition:uint8_t{					//These are all a bit TBC
			recipient =			0,							//ID of recipient, 0x00 or 0xFF. This is first so a 'peek' can see it
			sender =			1,							//ID of sender, 0x00 if not yet set
			payloadType =		2,							//See previous enum
			payloadNumber =		3,							//Sequence number which will overflow pretty regularly
			packetLength =		4,							//This is raw packet length without CRC, padding or encryption!
			blockIndex =		5,							//A 24-bit number used for large transfers or other flags
			nextTick =			8,							//A 16-bit measure of the milliseconds to next scheduled packer
			payload =			10							//Payload starts here!
			};
		//Encoding/decoding functions
		void buildPacketHeader(uint8_t,						//Put standard packet header in first X bytes
			uint8_t, payloadType);
		void buildKeepalivePacket(uint8_t);					//Keepalive packet
		void buildIdResolutionRequestPacket(				//ID resolution request - which ID has this name?
			uint8_t, char*);
		void buildNameResolutionRequestPacket(				//Name resolution request - which name has this ID?
			uint8_t, uint8_t);
		void buildIdAndNameResolutionResponsePacket(		//ID resolution response - ID maps to name
			uint8_t, uint8_t, uint8_t);
		void unpackPacket();								//Unpack the packet in the receive buffer
		void unpackKeepalivePacket(							//Unpack a keepalive packet
			uint8_t, uint8_t);
		void unpackIdResolutionRequestPacket(				//Unpack an ID resolution request
			uint8_t, uint8_t);
		void unpackNameResolutionRequestPacket(				//Unpack a name resolution request
			uint8_t, uint8_t);
		void unpackIdAndNameResolutionResponsePacket(		//Unpack an ID resolution response
			uint8_t, uint8_t);

		//General packet handling
		bool processPacketBeforeTransmission(uint8_t transport);//Add CRC then encrypt, if necessary and possible
		
		//Encryption
		void enableEncryption(uint8_t transport);			//Enable encryption for a specific transport
		void disableEncryption(uint8_t transport);			//Disable encryption for a specific transport
		uint8_t* encryptionKey = nullptr;					//Left null until set
		#if defined(TREACLE_ENCRYPT_WITH_CBC)
			#if defined(ESP32)
				esp_aes_context context;					//AES context
			#endif
		#endif
		uint8_t encryptionBlockSize = 16;					//Have to pad to this
		bool encryptPayload(uint8_t*,						//Pad the buffer if necessary and encrypt the payload
			uint8_t&);
		bool decryptPayload(uint8_t*,						//Decrypt the payload and remove the padding, if necessary
			uint8_t&);
		
		//Checksums
		const uint16_t treaclePolynome = 0xac9a;			//Taken from https://users.ece.cmu.edu/~koopman/crc/ as a 'good' polynome
		//Checksum functions
		bool appendChecksumToPacket(uint8_t*,				//Append a checksum to the packet if possible. Also increases the payload size!
			uint8_t&);
		bool validatePacketChecksum(uint8_t*,				//Check the checksum of a packet. Also decreases the payload size!
			uint8_t&);
					
		//Transport abstraction helpers
		bool sendBuffer(uint8_t, uint8_t*,					//Picks the appropriate sendBuffer function based on transport
			uint8_t payloadSize);
		bool packetInQueue();								//Check queue for every transport
		bool packetInQueue(uint8_t);						//Check queue for a specific transport
		bool online(uint8_t, uint8_t);						//Is a specific treacle node online for a specific protocol? ie. has this node heard from it recently
		
		//ESP-Now specific settings
		#if defined(TREACLE_SUPPORT_ESPNOW)
			uint8_t espNowTransportId = 255;				//ID assigned to this transport if enabled, 255 implies it is not
			uint8_t broadcastMacAddress[6] = {				//Most ESP-Now communications is broadcast
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
			uint8_t preferredespNowChannel = 1;				//It may not be possible to switch to the preferred channel if it is a WiFi client
			uint8_t currentEspNowChannel = 0;				//Track this, as it's not fixed if the device is a WiFi client
			bool currentEspNowChannelChanged = true;		//Flag to inform the application if the channel changes
			//ESP-Now specific functions
			bool initialiseWiFi();							//Initialise WiFi and return result. Only changes things if WiFi is not already set up when treacle begins
			bool changeWiFiChannel(uint8_t channel);		//Change the WiFi channel
			bool initialiseEspNow();						//Initialise ESP-Now and return result
			bool addEspNowPeer(uint8_t*);					//Add a peer, including relevant channel/interface for the time of addition
			bool deleteEspNowPeer(uint8_t*);				//Delete a peer
			bool sendBufferByEspNow(uint8_t*,				//Send a buffer using ESP-Now
				uint8_t);
		#endif
		
		//LoRa specific settings
		#if defined(TREACLE_SUPPORT_LORA)
			uint8_t loRaTransportId = 255;					//ID assigned to this transport if enabled, 255 implies it is not
			int8_t loRaCSpin = -1;							//LoRa radio chip select pin
			int8_t loRaResetPin = -1;						//LoRa radio reset pin
			int8_t loRaIrqPin = -1;							//LoRa radio interrupt pin
			uint32_t loRaFrequency = 868E6;					//LoRa frequency, broadly 868 in the EU, US is 915E6, Asia 433E6
			uint8_t loRaTxPower = 17;						//LoRa TX power
			uint8_t loRaSpreadingFactor = 9;				//LoRa spreading factor
			uint32_t loRaSignalBandwidth= 62.5E3;			//Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3(default), 250E3, and 500E3.
			uint8_t loRaRxGain = 0;							//0-6, 0 = auto
			uint8_t loRaSyncWord = 0x12;					//Valid options are 0x12, 0x56, 0x78, don't use 0x34 as that is LoRaWAN
			int16_t lastLoRaRssi = 0;						//Track RSSI as an extra indication of reachability
			float lastLoRaSNR = 0;							//Track SNR as an extra indication of reachability
			int16_t* rssi;									//Store last RSSI for each node, IF LoRa is enabled
			float* snr;										//Store last SNR for each node, IF LoRa is enabled
			//LoRa specific functions
			bool initialiseLoRa();							//Initialise LoRa and return result
			bool sendBufferByLoRa(uint8_t*,					//Send a buffer using ESP-Now
				uint8_t);
			bool receiveLoRa();								//Polling receive function
		#endif
		
		//COBS/Serial specific settings
		#if defined(TREACLE_SUPPORT_COBS)
			uint8_t cobsTransportId = 255;					//ID assigned to this transport if enabled, 255 implies it is not
			Stream *cobsStream_ = nullptr;					//COBS happens over a UART
			uint32_t cobsNominalBaudrate = 9600;			//Nominal baudrate for sending packets to avoid overfilling buffers
			//COBS/Serial specific functions
			bool initialiseCobs();							//Initialise Cobs and return result
			bool sendBufferByCobs(uint8_t*,					//Send a buffer using COBS
				uint8_t);
			bool receiveCobs();								//Polling receive function
		#endif
			
		//MQTT specific settings
		#if defined(TREACLE_SUPPORT_MQTT)
			uint8_t MQTTTransportId = 255;					//ID assigned to this transport if enabled, 255 implies it is not
			char* MQTTserver = nullptr;						//MQTT server IP address/name
			IPAddress MQTTserverIp = {0,0,0,0};				//Allow configuration by IP
			uint16_t MQTTport = 1883;						//MQTT server port
			char* MQTTusername = nullptr;					//MQTT server username
			char* MQTTpassword = nullptr;					//MQTT server password
			char* MQTTtopic = nullptr;						//MQTT server topic
			char* MQTTunicastTopic = nullptr;				//MQTT server unicast topic for this node
			#if defined(ESP8266)
			const char MQTTdefaultTopic[9]					//MQTT server default topic
				= "/treacle";
			#else
			const char MQTTdefaultTopic[9] PROGMEM			//MQTT server default topic
				= "/treacle";
			#endif
			//WiFiClient* mqttClient = nullptr;				//TCP/IP client
			#if defined(ESP8266) || defined(ESP32)
				WiFiClient mqttClient;						//TCP/IP client
			#elif defined(AVR)
				EthernetClient mqttClient;					//TCP/IP client
			#endif
			PubSubClient* mqtt = nullptr;					//MQTT client
			bool initialiseMQTT();							//Initialise MQTT
			void connectToMQTTserver();						//Attempt to (re)connect to the server
			bool sendBufferByMQTT(uint8_t*,					//Send a buffer using COBS
				uint8_t);
		#endif
		
		//UDP specific settings/functions
		#if defined(TREACLE_SUPPORT_UDP)
			uint8_t UDPTransportId = 255;					//ID assigned to this transport if enabled, 255 implies it is not
			#if defined(ESP8266)
				WiFiUDP* udp;								//UDP instance
				bool receiveUDP();							//Polling receiver
			#elif defined(ESP32)
				AsyncUDP* udp;								//UDP instance
			#elif defined(AVR)
				EthernetUDP* udp;							//UDP instance
				bool receiveUDP();							//Polling receiver
			#endif
			IPAddress udpMulticastAddress = {224,0,1,38};	//Multicast address
			uint16_t udpPort = 47625;						//UDP port number
			bool initialiseUDP();							//Initialise UDP
			bool sendBufferByUDP(uint8_t*,					//Send a buffer using COBS
				uint8_t);
		#endif
		
		//Utility functions
		uint8_t countBits(uint32_t thingToCount);			//Number of set bits in an uint32_t, or anything else
		float reliabilityPercentage(uint16_t);				//Turn an uint32_t bitmask into a printable reliability measure
		/*
		 *
		 *	Debugging helpers
		 *
		 */
		uint32_t lastStatusMessage = 0;
		#if defined(TREACLE_DEBUG)
			void showStatus();								//Show general status information
			Stream *debug_uart_ = nullptr;					//The stream used for any debugging
			template <class T>
			void debugPrint(T thingToPrint)
			{
				if(debug_uart_ != nullptr)
				{
					debug_uart_->print(thingToPrint);
				}
			}
			template <class T>
			void debugPrintln(T thingToPrint)
			{
				if(debug_uart_ != nullptr)
				{
					debug_uart_->println(thingToPrint);
				}
			}
			template <class T>
			void debugPrint(T thingToPrint, int base)
			{
				if(debug_uart_ != nullptr)
				{
					if(base == HEX)
					{
						debug_uart_->print("0x");
					}
					debug_uart_->print(thingToPrint, base);
				}
			}
			template <class T>
			void debugPrintln(T thingToPrint, int base)
			{
				if(debug_uart_ != nullptr)
				{
					if(base == HEX)
					{
						debug_uart_->print("0x");
					}
					debug_uart_->println(thingToPrint, base);
				}
			}
			void debugPrintln()
			{
				if(debug_uart_ != nullptr)
				{
					debug_uart_->println();
				}
			}			
			void debugPrintTransportName(uint8_t transport)
			{
				#if defined(TREACLE_SUPPORT_ESPNOW)
					if(transport == espNowTransportId){debugPrint(treacleDebugString_ESPNow);return;}
				#endif
				#if defined(TREACLE_SUPPORT_LORA)
					if(transport == loRaTransportId){debugPrint(treacleDebugString_LoRa);return;}
				#endif
				#if defined(TREACLE_SUPPORT_MQTT)
					if(transport == MQTTTransportId){debugPrint(treacleDebugString_MQTT);return;}
				#endif
				#if defined(TREACLE_SUPPORT_UDP)
					if(transport == UDPTransportId){debugPrint(treacleDebugString_UDP);return;}
				#endif
				#if defined(TREACLE_SUPPORT_COBS)
					if(transport == cobsTransportId){debugPrint(treacleDebugString_COBS);return;}
				#endif
			}
			void debugPrintPayloadTypeDescription(uint8_t type)
			{
				if(type & (uint8_t)payloadType::encrypted){debugPrint(treacleDebugString_encrypted);debugPrint(' ');}
				type = type & (0xff ^ (uint8_t)payloadType::encrypted);	//Remove the encrypted flag!
				if(type == (uint8_t)payloadType::keepalive){debugPrint(treacleDebugString_keepalive);}
				else if(type == (uint8_t)payloadType::idResolutionRequest){debugPrint(treacleDebugString_idResolutionRequest);}
				else if(type == (uint8_t)payloadType::nameResolutionRequest){debugPrint(treacleDebugString_nameResolutionRequest);}
				else if(type == (uint8_t)payloadType::idAndNameResolutionResponse){debugPrint(treacleDebugString_nameResolutionResponse);}
				else if(type == (uint8_t)payloadType::shortApplicationData){debugPrint(treacleDebugString_short_application_data);}
			}
			void debugPrintState(state theState)
			{
				if(theState == state::uninitialised){debugPrint(treacleDebugString_uninitialised);}
				else if(theState == state::starting){debugPrint(treacleDebugString_starting);}
				else if(theState == state::selectingId){debugPrint(treacleDebugString_selectingId);}
				else if(theState == state::selectedId){debugPrint(treacleDebugString_selectedId);}
				else if(theState == state::online){debugPrint(treacleDebugString_online);}
				else if(theState == state::offline){debugPrint(treacleDebugString_offline);}
				else if(theState == state::stopped){debugPrint(treacleDebugString_stopped);}
				else{debugPrint(treacleDebugString_unknown);}
			}
			void debugPrintString(char* text)
			{
				if(text != nullptr)
				{
					debugPrint('"');
					debugPrint(text);
					debugPrint('"');
					debugPrint(' ');
				}
			}
			void debugPrintStringln(char* text)
			{
				if(text != nullptr)
				{
					debugPrint('"');
					debugPrint(text);
					debugPrintln('"');
				}
				else
				{
					debugPrintln();
				}
			}
		#endif
};
extern treacleClass treacle;	//Create an instance of the class, as only one is practically usable at a time
#endif
