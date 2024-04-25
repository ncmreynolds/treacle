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

#define TREACLE_DEBUG

#define TREACLE_SUPPORT_ESPNOW
//#define TREACLE_SUPPORT_LORA
//#define TREACLE_SUPPORT_UDP
//#define TREACLE_SUPPORT_MQTT
//#define TREACLE_SUPPORT_COBS

#define TREACLE_ENCRYPT_WITH_CBC
//#define TREACLE_ENCRYPT_WITH_EAX

#if defined(ESP8266)
	#include <ESP8266WiFi.h>
	#if defined(TREACLE_SUPPORT_ESPNOW)
		#include <espnow.h>
	#endif
	#define ESP_OK 0
	#if defined(TREACLE_ENCRYPT_WITH_CBC)
		#include <CryptoAES_CBC.h>
		#include <AES.h>
		#include <CBC.h>
	#endif
#elif defined(ESP32)
	#include <WiFi.h>
	#include <esp_wifi.h> //Only needed for esp_wifi_set_channel()
	#if defined(TREACLE_SUPPORT_ESPNOW)
		extern "C"
		{
			#include <esp_now.h>
		}
	#endif
	#if defined(TREACLE_ENCRYPT_WITH_CBC)
		#include <aes/esp_aes.h>
	#endif
	#if defined(TREACLE_SUPPORT_UDP)
		#include <AsyncUDP.h>
	#endif
	#if defined(TREACLE_SUPPORT_MQTT)
		#include <PubSubClient.h>	//Support for MQTT
	#endif
#endif

#if defined(TREACLE_SUPPORT_LORA)
	#include <SPI.h>
	#include <LoRa.h>
#endif

#include "CRC16.h" //A CRC16 is used to check the packet is LIKELY to be sent in a known format
#include "CRC.h"

class treacleClass	{

	public:
		treacleClass();								//Constructor function
		~treacleClass();							//Destructor function
		//ESP-Now
		#if defined(TREACLE_SUPPORT_ESPNOW)
			void enableEspNow();						//Enable the ESP-Now radio
			bool espNowEnabled();						//Is ESP-Now radio enabled?
			#if defined(ESP32)
			bool enableEspNow11bMode();
			bool enableEspNowLrMode();
			#endif
			void enableEspNowEncryption();				//Enable encryption for ESP-Now
			void disableEspNowEncryption();				//Disable encryption for ESP-Now
			bool espNowInitialised();					//Is ESP-Now radio correctly initialised?
			void setEspNowChannel(uint8_t);				//Set the WiFi channel used for ESP-Now
			void setEspNowTickInterval(uint16_t tick);	//Set the ESP-Now tick interval
			uint8_t getEspNowChannel();					//Get the WiFi channel used for ESP-Now
			uint32_t getEspNowRxPackets();				//Get packet stats
			uint32_t getEspNowTxPackets();				//Get packet stats
			uint32_t getEspNowRxPacketsDropped();		//Get packet stats
			uint32_t getEspNowTxPacketsDropped();		//Get packet stats
			float getEspNowDutyCycle();					//Get packet stats
			uint32_t getEspNowDutyCycleExceptions();	//Get packet stats
			uint16_t getEspNowTickInterval();			//Get time between packets
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
			void setLoRaPins(int8_t cs, int8_t reset,	//Set the GPIO for the LoRa radio
				int8_t irq = -1);
			void setLoRaFrequency(uint32_t mhz);		//Set the LoRa frequency
			void enableLoRa();							//Enable the LoRa radio
			bool loRaEnabled();							//Is LoRa radio enabled?
			bool loRaInitialised();						//Is LoRa radio correctly initialised?
			uint32_t getLoRaRxPackets();				//Get packet stats
			uint32_t getLoRaTxPackets();				//Get packet stats
			uint32_t getLoRaRxPacketsDropped();			//Get packet stats
			uint32_t getLoRaTxPacketsDropped();			//Get packet stats
			float getLoRaDutyCycle();					//Get packet stats
			uint32_t getLoRaDutyCycleExceptions();		//Get packet stats
			void setLoRaTickInterval(uint16_t tick);	//Set the LoRa tick interval
			uint16_t getLoRaTickInterval();				//Get time between packets
			uint8_t getLoRaTxPower();					//LoRa TX power
			uint8_t getLoRaSpreadingFactor();			//LoRa spreading factor
			uint32_t getLoRaSignalBandwidth();			//Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3(default), 250E3, and 500E3.
			void setLoRaTxPower(uint8_t);				//LoRa TX power 2-20
			void setLoRaSpreadingFactor(uint8_t);		//LoRa spreading factor
			void setLoRaSignalBandwidth(uint32_t);		//Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3(default), 250E3, and 500E3.
			void setLoRaRxGain(uint8_t);				//0-6, 0 = auto
			uint16_t loRaRxReliability(uint8_t);
			uint16_t loRaTxReliability(uint8_t);
			int16_t  loRaRSSI(uint8_t);
			float    loRaSNR(uint8_t);
		#endif
		//MQTT
		#if defined(TREACLE_SUPPORT_MQTT)
			void enableMQTT();							//Enable MQTT
			bool MQTTEnabled();							//Is MQTT enabled?
			void setMQTTserver(char*);					//Set the MQTT server name
			void setMQTTserver(IPAddress);				//Set the MQTT server IP address (IPv4)
			void setMQTTport(uint16_t);					//Set the MQTT server, if not default
			void setMQTTtopic(char*);					//Set the MQTT base topic, if not default (/treacle)
			void setMQTTusername(char*);				//Set the MQTT username
			void setMQTTpassword(char*);				//Set the MQTT password
			void setMQTTserver(String);					//Set the MQTT server
			void setMQTTtopic(String);					//Set the MQTT base topic, if not default (/treacle)
			void setMQTTusername(String);				//Set the MQTT username
			void setMQTTpassword(String);				//Set the MQTT password
			bool MQTTInitialised();						//Is MQTT correctly initialised?
			uint32_t getMQTTRxPackets();				//Get packet stats
			uint32_t getMQTTTxPackets();				//Get packet stats
			uint32_t getMQTTRxPacketsDropped();			//Get packet stats
			uint32_t getMQTTTxPacketsDropped();			//Get packet stats
			float getMQTTDutyCycle();					//Get packet stats
			uint32_t getMQTTDutyCycleExceptions();		//Get packet stats
			void setMQTTTickInterval(uint16_t tick);	//Set the interval between packets
			uint16_t getMQTTTickInterval();				//Get interval between packets
		#endif
		//UDP
		#if defined(TREACLE_SUPPORT_UDP)
			void enableUDP();							//Enable UDP
			bool UDPEnabled();							//Is UDP enabled?
			bool UDPInitialised();						//Is UDP correctly initialised?
			void setUDPMulticastAddress(				//Set the multicast address to use (default 224.0.1.38)
				IPAddress address);
			void setUDPport(uint16_t);					//Set the UDP port (default 47625)
			uint32_t getUDPRxPackets();					//Get packet stats
			uint32_t getUDPTxPackets();					//Get packet stats
			uint32_t getUDPRxPacketsDropped();			//Get packet stats
			uint32_t getUDPTxPacketsDropped();			//Get packet stats
			float getUDPDutyCycle();					//Get packet stats
			uint32_t getUDPDutyCycleExceptions();		//Get packet stats
			void setUDPTickInterval(uint16_t tick);	//Set the interval between packets
			uint16_t getUDPTickInterval();				//Get interval between packets
		#endif
		//COBS/Serial
		#if defined(TREACLE_SUPPORT_COBS)
			void enableCobs();
			bool cobsEnabled();
			bool cobsInitialised();
		#endif
		//Messaging
		bool online();								//Is treacle online? ie. has this node heard back from a peer that has heard it recently
		void goOffline();							//Actively go offline
		void goOnline();							//Actively go online (if possible and this will take a few seconds)
		uint8_t nodes();							//Number of nodes
		uint8_t maxPayloadSize();					//Maximum single packet payload size
		uint32_t messageWaiting();					//Is there a message waiting?
		void clearWaitingMessage();					//Trash an incoming message
		uint8_t messageSender();					//The sender of the waiting message
		uint32_t suggestedQueueInterval();			//Suggest a delay before the next message
		bool queueMessage(char*);					//Queue a short message
		bool queueMessage(uint8_t*, uint8_t);		//Queue a short message
		bool queueMessage(const unsigned char*,		//Queue a short message
			uint8_t);
		bool sendMessage(char*);					//Send a short message ASAP
		bool sendMessage(uint8_t*, uint8_t);		//Send a short message ASAP
		bool sendMessage(const unsigned char*,		//Send a short message ASAP
			uint8_t);
		bool retrieveWaitingMessage(uint8_t*);		//Retrieve a message. The buffer must be large enough for it, no checking can be done
		//Encryption
		void setEncryptionKey(uint8_t* key);		//Set the encryption key
		//Node status
		bool online(uint8_t);						//Is a specific treacle node online? ie. has this node heard from it recently
		uint32_t rxAge(uint8_t);
		uint16_t rxReliability(uint8_t);
		uint16_t txReliability(uint8_t);
		//General
		void setNodeName(char* name);				//Set the node name
		void setNodeId(uint8_t id);					//Set the nodeId
		uint8_t getNodeId();						//Get the nodeId, which may have been autonegotiated
		bool begin(uint8_t maxNodes = 8);			//Start treacle, optionally specify a max number of nodes
		void end();									//Stop treacle
		void enableDebug(Stream &);					//Start debugging on a stream
		void disableDebug();						//Stop debugging
	protected:
	private:
		//State machine
		enum class state : uint8_t {uninitialised,		//State tracking
			starting,
			selectingId,
			selectedId,
			online,
			offline,
			stopped};
		state currentState = state::uninitialised;		//Current state
		uint32_t lastStateChange = 0;					//Track time of state changes
		void changeCurrentState(state);					//Change state and track the time of change
		
		//Transport information
		uint8_t numberOfActiveTransports = 0;			//Used to track transport IDs

		//Transmit packet buffers
		static const uint8_t maximumBufferSize= 250;	//Maximum buffer size, which is based off ESP-Now max size
		static const uint8_t maximumPayloadSize = 238;	//Maximum application payload size, which is based off ESP-Now max size
		
		//Ticks
		static const uint16_t maximumTickTime = 60E3;	//Absolute longest time something can be scheduled in the future
		//Tick functions
		uint16_t minimumTickTime(uint8_t);				//Absolute minimum tick time
		void setNextTickTime();							//Set a next tick time for all transports, done at startup
		void setNextTickTime(uint8_t);					//Set a next tick time immediately before sending for a specific transport
		uint16_t tickRandomisation(uint8_t);			//Random factor for timing
		void bringForwardNextTick();					//Hurry up the tick time for urgent things
		bool sendPacketOnTick();						//Send a single packet if it is due, returns true if this happens
		void timeOutTicks();							//Potentially time out ticks from other nodes if they stop responding

		struct transportData
		{
			bool initialised = false;					//Has the transport initialised OK?
			bool encrypted = false;						//Is the transport encrypted?
			uint32_t txPackets = 0;						//Simple stats for successfully transmitted packets
			uint32_t txPacketsDropped = 0;				//Simple stats for failed transmit packets
			uint32_t rxPackets = 0;						//Simple stats for successfully received packets
			uint32_t rxPacketsProcessed = 0;			//Simple stats for successfully received packets that were passed on for processing
			uint32_t rxPacketsDropped = 0;				//Simple stats for received packets that were dropped, probably due to a full buffer
			uint32_t txStartTime = 0;					//Used to calculate TX time for each packet using micros()
			uint32_t txTime = 0;						//Total time in micros() spent transmitting
			float calculatedDutyCycle = 0;				//Calculated from txTime and millis()
			float maximumDutyCycle = 1;					//Used as a hard brake on TX if exceeded
			uint32_t dutyCycleExceptions = 0;			//Count any time it goes over duty cycle
			uint32_t lastTick = 0;						//Track this node's ticks
			uint16_t defaultTick = maximumTickTime;		//Frequency of ticks for each transport, which is important
			uint16_t nextTick = 0;						//How long until the next tick for each transport, which is important. This varies slightly from the default.
			uint8_t transmitBuffer[maximumBufferSize];	//General transmit buffer
			uint8_t transmitPacketSize = 0;				//Current transmit packet size
			bool bufferSent = true;						//Per transport marker for when something is sent
			uint8_t payloadNumber = 0;					//Sequence number for payloads, this will overflow regularly
		};
		transportData* transport = nullptr;				//This will be allocated from heap during begin()
		
		
		//Node information
		uint8_t maximumNumberOfNodes = 8;				//Expected max number of nodes
		static const uint8_t absoluteMaximumNumberOfNodes = 80;	//Absolute max number of nodes
		uint8_t numberOfNodes = 0;
		struct nodeInfo
		{
			uint8_t id = 0;
			char* name = nullptr;
			uint32_t lastSeen = 0;
			uint32_t* lastTick = nullptr; 				//This is per transport
			uint16_t* nextTick = nullptr; 				//This is per transport
			uint16_t* txReliability = nullptr;			//This is per transport
			uint16_t* rxReliability = nullptr;			//This is per transport
			uint8_t* lastPayloadNumber = nullptr;		//This is per transport
		};
		nodeInfo* node;									//Chunky struct could overwhelm a small microcontroller, so be careful with maxNodes
		//Node management functions
		bool nodeExists(uint8_t id);					//Check if a node ID exists
		uint8_t nodeIndexFromId(uint8_t id);			//Get an index into nodeInfo from a node ID
		bool addNode(uint8_t id,						//Create a node. Default to excellent symmetric reliability
			uint16_t reliability = 0xffff);				//Create a node with symmetric reliability
		uint8_t nodeIndexFromName(char* name);			//Get an index into nodeInfo from a node name
		
		//Node ID management
		char* currentNodeName = nullptr;				//Everything has a name, don't use numerical addresses
		uint8_t currentNodeId = 0;						//Current node ID, 0 implies not set
		static const uint8_t minimumNodeId = 1;			//Lowest a node ID can be
		static const uint8_t maximumNodeId = 126;		//Highest a node ID can be
		bool selectNodeId();							//Select a node ID for this node
		
		//Duty cycle monitoring
		void calculateDutyCycle(uint8_t);				//Calculate the duty cycle for a specific transport based off current txTime, done just before sending
		
		//Receive packet buffers
		uint8_t receiveBuffer[maximumBufferSize];		//General receive buffer
		uint8_t receiveBufferSize = 0;					//Current receive payload size
		uint8_t receiveTransport = 0;					//Transport that received the packet
		bool receiveBufferDecrypted = false;			//Has the decryption been done?
		bool receiveBufferCrcChecked = false;			//Has the CRC been checked and removed?
		//Packet receiving functions
		bool packetReceived();							//Check for a packet in the buffer
		bool applicationDataPacketReceived();			//Check for an application data packet in the buffer
		void clearReceiveBuffer();						//Clear the receive buffer

		//Packet encoding/decoding
		enum class payloadType:uint8_t{					//These are all a bit TBC
			keepalive =						0x00,		//Lower four bits are a numeric type, making for 16 types
			idResolutionRequest =			0x01,		//Upper four bits are a bitmask
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
		enum class nodeId:uint8_t{						//These are all a bit TBC
			unknownNode =					0x00,
			allNodes =						0xff
		};
		enum class headerPosition:uint8_t{				//These are all a bit TBC
			recipient =			0,						//ID of recipient, 0x00 or 0xFF. This is first so a 'peek' can see it
			sender =			1,						//ID of sender, 0x00 if not yet set
			payloadType =		2,						//See previous enum
			payloadNumber =		3,						//Sequence number which will overflow pretty regularly
			packetLength =		4,						//This is raw packet length without CRC, padding or encryption!
			blockIndex =		5,						//A 24-bit number used for large transfers or other flags
			nextTick =			8,						//A 16-bit measure of the milliseconds to next scheduled packer
			payload =			10						//Payload starts here!
			};
		//Encoding/decoding functions
		void buildPacketHeader(uint8_t,					//Put standard packet header in first X bytes
			uint8_t, payloadType);
		void buildKeepalivePacket(uint8_t);				//Keepalive packet
		void buildIdResolutionRequestPacket(			//ID resolution request - which ID has this name?
			uint8_t, char*);
		void buildNameResolutionRequestPacket(			//Name resolution request - which name has this ID?
			uint8_t, uint8_t);
		void buildIdAndNameResolutionResponsePacket(	//ID resolution response - ID maps to name
			uint8_t, uint8_t, uint8_t);
		void unpackPacket();							//Unpack the packet in the receive buffer
		void unpackKeepalivePacket(						//Unpack a keepalive packet
			uint8_t, uint8_t);
		void unpackIdResolutionRequestPacket(			//Unpack an ID resolution request
			uint8_t, uint8_t);
		void unpackNameResolutionRequestPacket(			//Unpack a name resolution request
			uint8_t, uint8_t);
		void unpackIdAndNameResolutionResponsePacket(	//Unpack an ID resolution response
			uint8_t, uint8_t);

		//General packet handling
		bool processPacketBeforeTransmission(uint8_t transport);//Add CRC then encrypt, if necessary and possible
		
		//Encryption
		void enableEncryption(uint8_t transport);		//Enable encryption for a specific transport
		void disableEncryption(uint8_t transport);		//Disable encryption for a specific transport
		uint8_t* encryptionKey = nullptr;				//Left null until set
		#if defined(TREACLE_ENCRYPT_WITH_CBC)
			#if defined(ESP32)
				esp_aes_context context;				//AES context
			#endif
		#endif
		uint8_t encryptionBlockSize = 16;				//Have to pad to this
		bool encryptPayload(uint8_t*,					//Pad the buffer if necessary and encrypt the payload
			uint8_t&);
		bool decryptPayload(uint8_t*,					//Decrypt the payload and remove the padding, if necessary
			uint8_t&);
		
		//Checksums
		const uint16_t treaclePolynome = 0xac9a;		//Taken from https://users.ece.cmu.edu/~koopman/crc/ as a 'good' polynome
		//Checksum functions
		bool appendChecksumToPacket(uint8_t*,			//Append a checksum to the packet if possible. Also increases the payload size!
			uint8_t&);
		bool validatePacketChecksum(uint8_t*,			//Check the checksum of a packet. Also decreases the payload size!
			uint8_t&);
					
		//Transport abstraction helpers
		bool sendBuffer(uint8_t, uint8_t*,				//Picks the appropriate sendBuffer function based on transport
			uint8_t payloadSize);
		bool packetInQueue();							//Check queue for every transport
		bool packetInQueue(uint8_t);					//Check queue for a specific transport
		bool online(uint8_t, uint8_t);					//Is a specific treacle node online for a specific protocol? ie. has this node heard from it recently
		
		//ESP-Now specific settings
		#if defined(TREACLE_SUPPORT_ESPNOW)
			uint8_t espNowTransportId = 255;				//ID assigned to this transport if enabled, 255 implies it is not
			uint8_t broadcastMacAddress[6] = {				//Most ESP-Now communications is broadcast
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
			uint8_t preferredespNowChannel = 1;				//It may not be possible to switch to the preferred channel if it is a WiFi client
			uint8_t currentEspNowChannel = 0;				//Track this, as it's not fixed if the device is a WiFi client
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
			Stream *cobsUart_ = nullptr;					//COBS happens over a UART
			uint32_t cobsBaudRate = 115200;					//COBS needs a baud rate
			//COBS/Serial specific functions
			bool initialiseCobs();							//Initialise Cobs and return result
			bool sendBufferByCobs(uint8_t*,					//Send a buffer using COBS
				uint8_t);
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
			const char MQTTdefaultTopic[9] PROGMEM			//MQTT server default topic
				= "/treacle";
			//WiFiClient* mqttClient = nullptr;				//TCP/IP client
			WiFiClient mqttClient;							//TCP/IP client
			PubSubClient* mqtt = nullptr;					//MQTT client
			bool initialiseMQTT();							//Initialise MQTT
			void connectToMQTTserver();						//Attempt to (re)connect to the server
			bool sendBufferByMQTT(uint8_t*,					//Send a buffer using COBS
				uint8_t);
		#endif
		
		//UDP specific settings/functions
		#if defined(TREACLE_SUPPORT_UDP)
			uint8_t UDPTransportId = 255;					//ID assigned to this transport if enabled, 255 implies it is not
			AsyncUDP* udp;									//UDP instance
			//AsyncUDP udp;									//UDP instance
			IPAddress udpMulticastAddress = {224,0,1,38};	//Multicast address
			uint16_t udpPort = 47625;						//UDP port number
			bool initialiseUDP();							//Initialise UDP
			bool sendBufferByUDP(uint8_t*,					//Send a buffer using COBS
				uint8_t);
		#endif
		
		//Utility functions
		uint8_t countBits(uint32_t thingToCount);		//Number of set bits in an uint32_t, or anything else
		float reliabilityPercentage(uint16_t);			//Turn an uint32_t bitmask into a printable reliability measure
		/*
		 *
		 *	Debugging helpers
		 *
		 */
		uint32_t lastStatusMessage = 0;
		#if defined(TREACLE_DEBUG)
			void showStatus();							//Show general status information
			Stream *debug_uart_ = nullptr;				//The stream used for any debugging
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
			void debugPrintln()
			{
				if(debug_uart_ != nullptr)
				{
					debug_uart_->println();
				}
			}
			#if defined(ESP8266)
			const char debugString_treacleSpace[9] = "treacle ";
			const char debugString_starting[9] = "starting";
			const char debugString_start[6] = "start";
			const char debugString_ended[6] = "ended";
			const char debugString_enablingSpace[10] = "enabling ";
			const char debugString_checkingSpace[10] = "checking ";
			const char debugString_initialisingSpace[14] = "initialising ";
			const char debugString_notInitialised[16] = "not initialised";
			const char debugString_selectingSpace[11] = "selecting ";
			const char debugString_OK[3] = "OK";
			const char debugString_unknown[8] = "unknown";
			const char debugString_failed[7] = "failed";
			const char debugString_WiFi[5] = "WiFi";
			const char debugString_Client[7]  = "Client";
			const char debugString_AP[3] = "AP";
			const char debugString_ClientAndAP[12] = "Client & AP";
			#if defined(TREACLE_SUPPORT_ESPNOW)
				const char debugString_ESPNow[8] = "ESP-Now";
			#endif
			const char debugString_channel[8] = "channel";
			const char debugString_changedSpaceTo[11] = "changed to";
			const char debugString_WiFiSpacenotSpaceenabled[17] = "WiFi not enabled";
			#if defined(TREACLE_SUPPORT_LORA)
				const char debugString_LoRa[5] = "LoRa";
			#endif
			#if defined(TREACLE_SUPPORT_COBS)
				const char debugString_COBS[5] = "COBS";
			#endif
			const char debugString_newSpaceState[10] = "new state";
			const char debugString_uninitialised[14] = "uninitialised";
			const char debugString_selectingId[12] = "selectingId";
			const char debugString_selectedId[11] = "selectedId";
			const char debugString_online[7] = "online";
			const char debugString_offline[8] = "offline";
			const char debugString_stopped[8] = "stopped";
			const char debugString_nodeId[7] = "nodeId";
			const char debugString_tick[5] = "tick";
			const char debugString_keepalive[10] = "keepalive";
			const char debugString_short_application_data[23] = "short application data";
			const char debugString_sent[5] = "sent";
			const char debugString_received[9] = "received";
			const char debugString_toSpace[4] = "to ";
			const char debugString_fromSpace[6] = "from ";
			const char debugString_ActiveSpaceTransports[18] = "active transports";
			const char debugString_SpaceTransportID[14] = " transport ID";
			const char debugString_packetSpace[8] = "packet ";
			const char debugString_dropped[8] = "dropped";
			const char debugString_addingSpace[8] = "adding ";
			const char debugString_deletingSpace[10] = "deleting ";
			const char debugString_peer[5] = "peer";
			const char debugString_SpacedutySpacecycle[12] = " duty cycle";
			const char debugString_tooShort[10] = "too short";
			const char debugString_inconsistent[13] = "inconsistent";
			const char debugString_bytes[6] = "bytes";
			const char debugString_SpacenewCommaadded[12] = " new, added";
			const char debugString__too_many_nodes[16] = " too many nodes";
			const char debugString_includes[9] = "includes";
			const char debugString_checksum_invalid[17] = "checksum invalid";
			const char debugString_this_node[10] = "this node";
			const char debugString_node_name[10] = "node name";
			const char debugString_idResolutionRequest[17] = "name->ID request";
			const char debugString_nameResolutionRequest[17] = "ID->name request";
			const char debugString_nameResolutionResponse[18] = "ID->name response";
			const char debugString_looking_up[11] = "looking up";
			const char debugString_responding[11] = "responding";
			const char debugString_rxReliability[14] = "rxReliability";
			const char debugString_txReliability[14] = "txReliability";
			const char debugString_message[8] = "message";
			const char debugString_cleared[8] = "cleared";
			const char debugString_padded_by[10] = "padded by";
			const char debugString_encrypted[10] = "encrypted";
			const char debugString_decrypted[10] = "decrypted";
			const char debugString_encryption_key[15] = "encryption key";
			const char debugString_duplicate[10] = "duplicate";
			const char debugString_payload_numberColon[16] = "payload number:";
			const char debugString_after[6] = "after";
			const char debugString_minutes[8] = "minutes";
			const char debugString_expediting_[12] = "expediting ";
			const char debugString_for[4] = "for";
			const char debugString_response[9] = "response";
			const char debugString_all[4] = "all";
			const char debugString_nodes[6] = "nodes";
			const char debugString_reached[8] = "reached";
			const char debugString_with[5] = "with";
			const char debugString_duty_cycle_exceeded[20] = "duty cycle exceeded";
			const char debugString_TXcolon[4] = "TX:";
			const char debugString_TX_drops_colon[10] = "TX drops:";
			const char debugString_RXcolon[4] = "RX:";
			const char debugString_RX_drops_colon[10] = "RX drops:";
			const char debugString_up[3] = "up";
			const char debugString_suggested_message_interval[27] = "suggested message interval";
			#if defined(TREACLE_SUPPORT_MQTT)
				const char debugString_MQTT[5] = "MQTT";
				const char debugString_MQTTspace[6] = "MQTT ";
				const char debugString_connectionSpace[12] = "connection ";
				const char debugString_server[7] = "server";
				const char debugString_topic[6] = "topic";
				const char debugString_username[9] = "username";
				const char debugString_password[9] = "password";
			#endif
			#if defined(TREACLE_SUPPORT_UDP)
				const char debugString_UDP[5] = "UDP";
				const char debugString_UDPspace[5] = "UDP ";
			#endif
			#if defined(TREACLE_SUPPORT_MQTT) || defined(TREACLE_SUPPORT_MQTT)
				const char debugString_port[5] = "port";
			#endif
			#elif defined(ESP32)
			const char debugString_treacleSpace[9] PROGMEM = "treacle ";
			const char debugString_starting[9] PROGMEM = "starting";
			const char debugString_start[6] PROGMEM = "start";
			const char debugString_ended[6] PROGMEM = "ended";
			const char debugString_enablingSpace[10] PROGMEM = "enabling ";
			const char debugString_checkingSpace[10] PROGMEM = "checking ";
			const char debugString_initialisingSpace[14] PROGMEM = "initialising ";
			const char debugString_notInitialised[16] PROGMEM = "not initialised";
			const char debugString_selectingSpace[11] PROGMEM = "selecting ";
			const char debugString_OK[3] PROGMEM = "OK";
			const char debugString_unknown[8] PROGMEM = "unknown";
			const char debugString_failed[7] PROGMEM = "failed";
			const char debugString_WiFi[5] PROGMEM = "WiFi";
			const char debugString_Client[7]  PROGMEM = "Client";
			const char debugString_AP[3] PROGMEM = "AP";
			const char debugString_ClientAndAP[12] PROGMEM = "Client & AP";
			#if defined(TREACLE_SUPPORT_ESPNOW)
				const char debugString_ESPNow[8] PROGMEM = "ESP-Now";
			#endif
			const char debugString_channel[8] PROGMEM = "channel";
			const char debugString_changedSpaceTo[11] PROGMEM = "changed to";
			const char debugString_WiFiSpacenotSpaceenabled[17] PROGMEM = "WiFi not enabled";
			#if defined(TREACLE_SUPPORT_LORA)
				const char debugString_LoRa[5] PROGMEM = "LoRa";
			#endif
			#if defined(TREACLE_SUPPORT_COBS)
				const char debugString_COBS[5] PROGMEM = "COBS";
			#endif
			const char debugString_newSpaceState[10] PROGMEM = "new state";
			const char debugString_uninitialised[14] PROGMEM = "uninitialised";
			const char debugString_selectingId[12] PROGMEM = "selectingId";
			const char debugString_selectedId[11] PROGMEM = "selectedId";
			const char debugString_online[7] PROGMEM = "online";
			const char debugString_offline[8] PROGMEM = "offline";
			const char debugString_stopped[8] PROGMEM = "stopped";
			const char debugString_nodeId[7] PROGMEM = "nodeId";
			const char debugString_tick[5] PROGMEM = "tick";
			const char debugString_keepalive[10] PROGMEM = "keepalive";
			const char debugString_short_application_data[23] PROGMEM = "short application data";
			const char debugString_sent[5] PROGMEM = "sent";
			const char debugString_received[9] PROGMEM = "received";
			const char debugString_toSpace[4] PROGMEM = "to ";
			const char debugString_fromSpace[6] PROGMEM = "from ";
			const char debugString_ActiveSpaceTransports[18] PROGMEM = "active transports";
			const char debugString_SpaceTransportID[14] PROGMEM = " transport ID";
			const char debugString_packetSpace[8] PROGMEM = "packet ";
			const char debugString_dropped[8] PROGMEM = "dropped";
			const char debugString_addingSpace[8] PROGMEM = "adding ";
			const char debugString_deletingSpace[10] PROGMEM = "deleting ";
			const char debugString_peer[5] PROGMEM = "peer";
			const char debugString_SpacedutySpacecycle[12] PROGMEM = " duty cycle";
			const char debugString_tooShort[10] PROGMEM = "too short";
			const char debugString_inconsistent[13] PROGMEM = "inconsistent";
			const char debugString_bytes[6] PROGMEM = "bytes";
			const char debugString_SpacenewCommaadded[12] PROGMEM = " new, added";
			const char debugString__too_many_nodes[16] PROGMEM = " too many nodes";
			const char debugString_includes[9] PROGMEM = "includes";
			const char debugString_checksum_invalid[17] PROGMEM = "checksum invalid";
			const char debugString_this_node[10] PROGMEM = "this node";
			const char debugString_node_name[10] PROGMEM = "node name";
			const char debugString_idResolutionRequest[17] PROGMEM = "name->ID request";
			const char debugString_nameResolutionRequest[17] PROGMEM = "ID->name request";
			const char debugString_nameResolutionResponse[18] PROGMEM = "ID->name response";
			const char debugString_looking_up[11] PROGMEM = "looking up";
			const char debugString_responding[11] PROGMEM = "responding";
			const char debugString_rxReliability[14] PROGMEM = "rxReliability";
			const char debugString_txReliability[14] PROGMEM = "txReliability";
			const char debugString_message[8] PROGMEM = "message";
			const char debugString_cleared[8] PROGMEM = "cleared";
			const char debugString_padded_by[10] PROGMEM = "padded by";
			const char debugString_encrypted[10] PROGMEM = "encrypted";
			const char debugString_decrypted[10] PROGMEM = "decrypted";
			const char debugString_encryption_key[15] PROGMEM = "encryption key";
			const char debugString_duplicate[10] PROGMEM = "duplicate";
			const char debugString_payload_numberColon[16] PROGMEM = "payload number:";
			const char debugString_after[6] PROGMEM = "after";
			const char debugString_minutes[8] PROGMEM = "minutes";
			const char debugString_expediting_[12] PROGMEM = "expediting ";
			const char debugString_for[4] PROGMEM = "for";
			const char debugString_response[9] PROGMEM = "response";
			const char debugString_all[4] PROGMEM = "all";
			const char debugString_nodes[6] PROGMEM = "nodes";
			const char debugString_reached[8] PROGMEM = "reached";
			const char debugString_with[5] PROGMEM = "with";
			const char debugString_duty_cycle_exceeded[20] PROGMEM = "duty cycle exceeded";
			const char debugString_TXcolon[4] PROGMEM = "TX:";
			const char debugString_TX_drops_colon[10] PROGMEM = "TX drops:";
			const char debugString_RXcolon[4] PROGMEM = "RX:";
			const char debugString_RX_drops_colon[10] PROGMEM = "RX drops:";
			const char debugString_up[3] PROGMEM = "up";
			const char debugString_suggested_message_interval[27] PROGMEM = "suggested message interval";
			#if defined(TREACLE_SUPPORT_MQTT)
				const char debugString_MQTT[5] PROGMEM = "MQTT";
				const char debugString_MQTTspace[6] PROGMEM = "MQTT ";
				const char debugString_connectionSpace[12] PROGMEM = "connection ";
				const char debugString_server[7] PROGMEM = "server";
				const char debugString_topic[6] PROGMEM = "topic";
				const char debugString_username[9] PROGMEM = "username";
				const char debugString_password[9] PROGMEM = "password";
			#endif
			#if defined(TREACLE_SUPPORT_UDP)
				const char debugString_UDP[5] PROGMEM = "UDP";
				const char debugString_UDPspace[5] PROGMEM = "UDP ";
			#endif
			#if defined(TREACLE_SUPPORT_MQTT) || defined(TREACLE_SUPPORT_UDP)
				const char debugString_port[5] PROGMEM = "port";
			#endif
			#endif
			
			void debugPrintTransportName(uint8_t transport)
			{
				#if defined(TREACLE_SUPPORT_ESPNOW)
					if(transport == espNowTransportId){debugPrint(debugString_ESPNow);return;}
				#endif
				#if defined(TREACLE_SUPPORT_LORA)
					if(transport == loRaTransportId){debugPrint(debugString_LoRa);return;}
				#endif
				#if defined(TREACLE_SUPPORT_MQTT)
					if(transport == MQTTTransportId){debugPrint(debugString_MQTT);return;}
				#endif
				#if defined(TREACLE_SUPPORT_UDP)
					if(transport == UDPTransportId){debugPrint(debugString_UDP);return;}
				#endif
				#if defined(TREACLE_SUPPORT_COBS)
					if(transport == cobsTransportId){debugPrint(debugString_COBS);return;}
				#endif
			}
			void debugPrintPayloadTypeDescription(uint8_t type)
			{
				if(type & (uint8_t)payloadType::encrypted){debugPrint(debugString_encrypted);debugPrint(' ');}
				type = type & (0xff ^ (uint8_t)payloadType::encrypted);	//Remove the encrypted flag!
				if(type == (uint8_t)payloadType::keepalive){debugPrint(debugString_keepalive);}
				else if(type == (uint8_t)payloadType::idResolutionRequest){debugPrint(debugString_idResolutionRequest);}
				else if(type == (uint8_t)payloadType::nameResolutionRequest){debugPrint(debugString_nameResolutionRequest);}
				else if(type == (uint8_t)payloadType::idAndNameResolutionResponse){debugPrint(debugString_nameResolutionResponse);}
				else if(type == (uint8_t)payloadType::shortApplicationData){debugPrint(debugString_short_application_data);}
			}
			void debugPrintState(state theState)
			{
				if(theState == state::uninitialised){debugPrint(debugString_uninitialised);}
				else if(theState == state::starting){debugPrint(debugString_starting);}
				else if(theState == state::selectingId){debugPrint(debugString_selectingId);}
				else if(theState == state::selectedId){debugPrint(debugString_selectedId);}
				else if(theState == state::online){debugPrint(debugString_online);}
				else if(theState == state::offline){debugPrint(debugString_offline);}
				else if(theState == state::stopped){debugPrint(debugString_stopped);}
				else{debugPrint(debugString_unknown);}
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
