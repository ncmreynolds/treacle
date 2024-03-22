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

#ifdef ESP32
	#include <WiFi.h>
	#include <esp_wifi.h> //Only needed for esp_wifi_set_channel()
	extern "C"
	{
		#include <esp_now.h>
		//#include <esp_wifi.h> // only for esp_wifi_set_channel()
	}
	#if !defined(TREACLE_OBFUSCATE_ONLY)
		#include <aes/esp_aes.h>
	#endif
#endif

#define NUMBER_OF_PROTOCOLS

#include <SPI.h>
#include <LoRa.h>
#include "CRC16.h" //A CRC16 is used to check the packet is LIKELY to be sent in a known format
#include "CRC.h"

class treacleClass	{

	public:
		treacleClass();								//Constructor function
		~treacleClass();							//Destructor function
		//ESP-Now
		void enableEspNow();						//Enable the ESP-Now radio
		bool espNowEnabled();						//Is ESP-Now radio enabled?
		void enableEspNowEncryption();				//Enable encryption for ESP-Now
		void disableEspNowEncryption();				//Disable encryption for ESP-Now
		bool espNowInitialised();					//Is ESP-Now radio correctly initialised?
		void setEspNowChannel(uint8_t);				//Set the WiFi channel used for ESP-Now
		uint8_t getEspNowChannel();					//Get the WiFi channel used for ESP-Now
		//LoRa
		void setLoRaPins(int8_t cs, int8_t reset,	//Set the GPIO for the LoRa radio
			int8_t irq = -1);
		void enableLoRa();							//Enable the LoRa radio
		bool loRaEnabled();							//Is LoRa radio enabled?
		bool loRaInitialised();						//Is LoRa radio correctly initialised?
		//COBS/Serial
		void enableCobs();
		bool cobsEnabled();
		bool cobsInitialised();
		//Messaging
		bool online();								//Is treacle online? ie. has this node heard back from a peer that has heard it recently
		bool online(uint8_t);						//Is a specific treacle node online? ie. has this node heard from it recently
		void goOffline();							//Actively go offline
		void goOnline();							//Actively go online (if possible and this will take a few seconds)
		uint8_t maxPayloadSize();					//Maximum single packet payload size
		uint32_t messageWaiting();					//Is there a message waiting?
		void clearWaitingMessage();					//Trash an incoming message
		uint8_t messageSender();					//The sender of the waiting message
		bool queueMessage(char*);					//Send a short message
		bool queueMessage(uint8_t*, uint8_t);		//Send a short message
		bool queueMessage(const unsigned char*,		//Send a short message
			uint8_t);
		bool retrieveWaitingMessage(uint8_t*);		//Retrieve a message. The buffer must be large enough for it, no checking can be done
		//Encryption
		void setEncryptionKey(uint8_t* key);		//Set the encryption key
		//General
		bool begin(uint8_t maxNodes = 80);			//Start treacle, optionally specify a max number of nodes
		void end();									//Stop treacle
		void enableDebug(Stream &);					//Start debugging on a stream
		void disableDebug();						//Stop debugging
	protected:
	private:
		//State machine
		enum class state : uint8_t {uninitialised,	//State tracking
			starting, selectingId, selectedId,
			online, offline,
			stopped};
		state currentState = state::uninitialised;	//Current state
		uint32_t lastStateChange = 0;				//Track time of state changes
		void changeCurrentState(state);				//Change state and track the time of change
		
		//Protocol information
		uint8_t numberOfActiveProtocols = 0;		//Used to track protocol IDs
		//bool* protocolInitialised = nullptr;		//Has the radio initialised OK?

		//Transmit packet buffers
		static const uint8_t maximumBufferSize= 250;//Maximum buffer size, which is based off ESP-Now max size
		static const uint8_t maximumPayloadSize=238;//Maximum application payload size, which is based off ESP-Now max size
		//uint8_t transmitBuffer[maximumBufferSize];	//General transmit buffer
		//uint8_t transmitPacketSize = 0;				//Current transmit packet size
		
		struct protocolData
		{
			bool initialised = false;				//Has the protocol initialised OK?
			bool encrypted = false;					//Is the protocol encrypted?
			uint32_t txPackets = 0;					//Simple stats for successfully transmitted packets
			uint32_t txPacketsDropped = 0;			//Simple stats for failed transmit packets
			uint32_t rxPackets = 0;					//Simple stats for successfully received packets
			uint32_t rxPacketsProcessed = 0;		//Simple stats for successfully received packets that were passed on for processing
			uint32_t rxPacketsDropped = 0;			//Simple stats for received packets that were dropped, probably due to a full buffer
			uint32_t txStartTime = 0;				//Used to calculate TX time for each packet using micros()
			uint32_t txTime = 0;					//Total time in micros() spent transmitting
			float calculatedDutyCycle = 0;			//Calculated from txTime and millis()
			float maximumDutyCycle = 1;				//Used as a hard brake on TX if exceeded
			uint32_t lastTick;						//Track this node's ticks
			uint16_t nextTick;						//How long until the next tick for each protocol, which is important
			uint8_t transmitBuffer[maximumBufferSize];	//General transmit buffer
			uint8_t transmitPacketSize = 0;				//Current transmit packet size
			bool bufferSent = true;					//Per protocol marker for when something is sent
		};
		protocolData* protocol = nullptr;			//This will be allocated from heap during begin()
		
		
		//Node information
		//uint8_t maximumNumberOfNodes = 16;			//Max number of nodes
		static const uint8_t maximumNumberOfNodes = 16;			//Max number of nodes
		uint8_t numberOfNodes = 0;
		struct nodeInfo
		{
			uint8_t id = 0;
			uint32_t* lastTick = nullptr; 			//This is per protocol
			uint16_t* nextTick = nullptr; 			//This is per protocol
			uint16_t* txReliability = nullptr;		//This is per protocol
			uint16_t* rxReliability = nullptr;		//This is per protocol
			char* name = nullptr;
		};
		nodeInfo node[maximumNumberOfNodes];		//Chunky struct could overwhelm a small microcontroller, so be careful
		//nodeInfo* node;							//Chunky struct could overwhelm a small microcontroller, so be careful
		//Node management functions
		bool nodeExists(uint8_t id);				//Check if a node ID exists
		uint8_t indexFromId(uint8_t id);			//Get an index into nodeInfo from a node ID
		bool addNode(uint8_t id);					//Create a node
		uint8_t indexFromName(char* name);			//Get an index into nodeInfo from a node name
		//Node ID management
		char* currentNodeName = nullptr;			//Everything has a name, don't use numerical addresses
		uint8_t currentNodeId = 0;					//Current node ID, 0 implies not set
		uint8_t minimumNodeId = 1;					//Lowest a node ID can be
		uint8_t maximumNodeId = 16;					//Highest a node ID can be
		bool selectNodeId();						//Select a node ID for this node
		
		//Duty cycle monitoring
		uint32_t lastDutyCycleCheck = 0;			//Time of last duty cycle check
		uint32_t dutyCycleCheckInterval = 30E3;		//Check duty cycle every 30s
		void checkDutyCycle();						//Calculate the duty cycle based off current txTime
		
		//Receive packet buffers
		uint8_t receiveBuffer[maximumBufferSize];	//General receive buffer
		uint8_t receiveBufferSize = 0;				//Current receive payload size
		uint8_t receiveProtocol = 0;				//Protocol that received the packet
		bool receiveBufferDecrypted = false;		//Has the decryption been done?
		bool receiveBufferCrcChecked = false;		//Has the CRC been checked and removed?
		//Packet receiving functions
		bool packetReceived();						//Check for a packet in the buffer
		bool applicationDataPacketReceived();		//Check for an application data packet in the buffer
		void clearReceiveBuffer();					//Clear the receive buffer

		//Packet encoding/decoding
		enum class payloadType:uint8_t{				//These are all a bit TBC
			keepalive =						0x00,	//Lower four bits are a numeric type, making for 16 types
			idResolutionRequest =			0x01,	//Upper four bits are a bitmask
			nameResolutionRequest =			0x02,
			idAndNameResolutionResponse =	0x03,
			shortApplicationData =			0x04,
			//idAndNameResolutionResponse =	0x05,
			//idAndNameResolutionResponse =	0x06,
			//idAndNameResolutionResponse =	0x07,
			//idAndNameResolutionResponse =	0x08,
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
		enum class headerPosition:uint8_t{			//These are all a bit TBC
			sender =			0,
			recipient =			1,
			payloadNumber =		2,
			packetLength =		3,					//This is raw packet length without CRC, padding or encryption!
			payloadType =		4,
			blockIndex =		5,
			nextTick =			8,
			payload =			10
			};
		uint16_t payloadNumber = 0;					//Sequence number for payloads
		//Encoding/decoding functions
		void buildStandardPacketHeader(uint8_t,		//Put standard packet header in first X bytes
			uint8_t, payloadType);
		void buildKeepalivePacket(uint8_t);			//Keepalive packet
		void buildIdResolutionRequestPacket(		//ID resolution request - which ID has this name?
			uint8_t, char*);
		void buildNameResolutionRequestPacket(		//Name resolution request - which name has this ID?
			uint8_t, uint8_t);
		void buildIdAndNameResolutionResponsePacket(//ID resolution response - ID maps to name
			uint8_t, uint8_t);
		void unpackPacket();						//Unpack the packet in the receive buffer
		void unpackKeepalivePacket(uint8_t);		//Unpack a keepalive packet
		void unpackIdResolutionRequestPacket(		//Unpack an ID resolution request
			uint8_t);
		void unpackNameResolutionRequestPacket(		//Unpack a name resolution request
			uint8_t);
		void unpackIdAndNameResolutionResponsePacket(//Unpack an ID resolution response
			uint8_t);

		//General packet handling
		bool processPacketBeforeTransmission(uint8_t protocol);//Add CRC then encrypt, if necessary and possible
		
		//Encryption
		void enableEncryption(uint8_t protocol);	//Enable encryption for a specific protocol
		void disableEncryption(uint8_t protocol);	//Disable encryption for a specific protocol
		uint8_t* encryptionKey = nullptr;			//Left null until set
		#ifdef ESP32
			esp_aes_context context;				//AES context
		#endif
		uint8_t encryptionBlockSize = 16;			//Have to pad to this
		bool encryptPayload(uint8_t*,				//Pad the buffer if necessary and encrypt the payload
			uint8_t&);
		bool decryptPayload(uint8_t*,				//Decrypt the payload and remove the padding, if necessary
			uint8_t&);
		
		//Checksums
		const uint16_t treaclePolynome = 0xac9a;	//Taken from https://users.ece.cmu.edu/~koopman/crc/ as a 'good' polynome
		//Checksum functions
		bool appendChecksumToPacket(uint8_t*,		//Append a checksum to the packet if possible. Also increases the payload size!
			uint8_t&);
		bool validatePacketChecksum(uint8_t*,		//Check the checksum of a packet. Also decreases the payload size!
			uint8_t&);
			

		//Ticks
		uint16_t maximumTickTime = 15E3;			//Absolute longest time something can be scheduled in the future
		uint16_t tickTimeMargin = 1E3;				//Margin allowed for any scheduled tick time
		//Tick functions
		void setTickTime();							//Set a new tick time whenever something happens
		bool sendPacketOnTick();					//Send a single packet if it is due, returns true if this happens
		void timeOutTicks();						//Potentially time out ticks from other nodes if they stop responding
		
		//Protocol abstraction helpers
		bool sendBuffer(uint8_t, uint8_t*,			//Picks the appropriate sendBuffer function based on protocol
			uint8_t payloadSize);
		bool noPacketInQueue();						//Check queue for every protocol
		
		//ESP-Now specific settings
		uint8_t espNowProtocolId = 255;				//ID assigned to this protocol if enabled, 255 implies it is not
		uint8_t broadcastMacAddress[6] = {			//Most ESP-Now communications is broadcast
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
		uint8_t preferredespNowChannel = 1;			//It may not be possible to switch to the preferred channel if it is a WiFi client
		uint8_t currentEspNowChannel = 0;			//Track this, as it's not fixed if the device is a WiFi client
		//ESP-Now specific functions
		bool initialiseWiFi();						//Initialise WiFi and return result. Only changes things if WiFi is not already set up when treacle begins
		bool changeWiFiChannel(uint8_t channel);	//Change the WiFi channel
		bool initialiseEspNow();					//Initialise ESP-Now and return result
		bool addEspNowPeer(uint8_t*);				//Add a peer, including relevant channel/interface for the time of addition
		bool deleteEspNowPeer(uint8_t*);			//Delete a peer
		bool sendBufferByEspNow(uint8_t*,			//Send a buffer using ESP-Now
			uint8_t);
		
		//LoRa specific settings
		uint8_t loRaProtocolId = 255;				//ID assigned to this protocol if enabled, 255 implies it is not
		int8_t loRaCSpin = 7;						//LoRa radio chip select pin
		int8_t loRaResetPin = 2;					//LoRa radio reset pin
		int8_t loRaIrqPin = 10;						//LoRa radio interrupt pin
		uint32_t loRaFrequency = 868E6;				//LoRa frequency, broadly 868 in the EU, US is 915E6, Asia 433E6
		uint8_t defaultLoRaTxPower = 17;			//LoRa TX power
		uint8_t defaultLoRaSpreadingFactor = 7;		//LoRa spreading factor
		uint32_t defaultLoRaSignalBandwidth = 250E3;//Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3(default), 250E3, and 500E3.
		uint8_t loRaSyncWord = 0x12;				//Valid options are 0x12, 0x56, 0x78, don't use 0x34 as that is LoRaWAN
		float lastLoRaRssi = 0.0;					//Track RSSI as an extra indication of reachability
		//LoRa specific functions
		bool initialiseLoRa();						//Initialise LoRa and return result
		bool sendBufferByLoRa(uint8_t*,				//Send a buffer using ESP-Now
			uint8_t);
		
		//COBS/Serial specific setting
		uint8_t cobsProtocolId = 255;				//ID assigned to this protocol if enabled, 255 implies it is not
		Stream *cobsUart_ = nullptr;				//COBS happens over a UART
		uint32_t cobsBaudRate = 115200;				//COBS needs a baud rate
		//COBS/Serial specific functions
		bool initialiseCobs();						//Initialise Cobs and return result
		bool sendBufferByCobs(uint8_t*,				//Send a buffer using COBS
			uint8_t);
		
		/*
		 *
		 *	Debugging helpers
		 *
		 */
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
		const char debugString_treacleSpace[9] PROGMEM = "treacle ";
		const char debugString_starting[9] PROGMEM = "starting";
		const char debugString_start[6] PROGMEM = "start";
		const char debugString_ended[6] PROGMEM = "ended";
		const char debugString_enablingSpace[10] PROGMEM = "enabling ";
		const char debugString_checkingSpace[10] PROGMEM = "checking ";
		const char debugString_initialisingSpace[14] PROGMEM = "initialising ";
		const char debugString_notInitialised[16] PROGMEM = "not initialised";
		const char debugString_selectingSpace[11] PROGMEM = "selecting ";
		const char debugString_colonSpace[3] PROGMEM = ": ";
		const char debugString_OK[3] PROGMEM = "OK";
		const char debugString_unknown[8] PROGMEM = "unknown";
		const char debugString_failed[7] PROGMEM = "failed";
		const char debugString_WiFi[5] PROGMEM = "WiFi";
		const char debugString_Client[7]  PROGMEM = "Client";
		const char debugString_AP[3] PROGMEM = "AP";
		const char debugString_ClientAndAP[12] PROGMEM = "Client & AP";
		const char debugString_ESPNow[8] PROGMEM = "ESP-Now";
		const char debugString_channel[8] PROGMEM = "channel";
		const char debugString_changedSpaceTo[11] PROGMEM = "changed to";
		const char debugString_WiFiSpacenotSpaceenabled[17] PROGMEM = "WiFi not enabled";
		const char debugString_LoRa[5] PROGMEM = "LoRa";
		const char debugString_COBS[5] PROGMEM = "COBS";
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
		const char debugString_ActiveSpaceProtocols[17] PROGMEM = "active protocols";
		const char debugString_SpaceProtocolID[13] PROGMEM = " protocol ID";
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
		
		void debugPrintProtocolName(uint8_t protocol)
		{
			if(protocol == espNowProtocolId){debugPrint(debugString_ESPNow);}
			else if(protocol == loRaProtocolId){debugPrint(debugString_LoRa);}
			else if(protocol == cobsProtocolId){debugPrint(debugString_COBS);}
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
};
extern treacleClass treacle;	//Create an instance of the class, as only one is practically usable at a time
#endif
