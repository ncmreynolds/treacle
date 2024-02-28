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
		bool espNowInitialised();					//Is ESP-Now radio correctly initialised?
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
		uint32_t messageWaiting();					//Is there a message waiting?
		uint8_t messageSender();					//The sender of the waiting message
		bool sendMessage();
		//General
		bool begin();								//Start treacle
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
		
		//ID management
		uint8_t currentNodeId = 0;					//Current node ID, 0 implies not set
		uint8_t minimumNodeId = 1;					//Lowest a node ID can be
		uint8_t maximumNodeId = 16;					//Highest a node ID can be
		bool selectNodeId();						//Select a node ID for this node
		bool checkNodeIdIsUnique(uint8_t);			//Check a proposed node ID is unique

		//Protocol data
		uint8_t numberOfActiveProtocols = 0;		//Used to track protocol IDs
		bool* protocolEnabled;						//Should the radio be initialised?
		bool* protocolInitialised;					//Has the radio initialised OK?
		uint32_t* txPackets;						//Simple packet TX stats
		uint32_t* txPacketsDropped;					//Simple packet TX stats
		uint32_t* rxPackets;						//Simple packet RX stats
		uint32_t* rxPacketsDropped;					//Simple packet RX stats
		uint32_t* txStartTime;						//Used to calculate TX time for each packet using micros()
		uint32_t* TxTime;							//Total time in ms spent transmitting
		float* calculatedDutyCycle;					//Calculated from txTime and millis()
		float* maximumDutyCycle;					//Used as a hard brake on TX if exceeded
		
		//Packet buffers
		const uint8_t maximumBufferSize = 255;		//Maximum buffer size
		uint8_t* transmitBuffer = nullptr;			//General transmit buffer
		uint8_t transmitPayloadSize = 0;			//Current transmit payload size
		bool* bufferSent = nullptr;					//Per protocol marker for when something is sent
		uint8_t* receiveBuffer = nullptr;			//General receive buffer
		uint8_t receivePayloadSize = 0;				//Current receive payload size
		uint16_t packetNumber = 0;					//Sequence number
		void buildEmptyTickPacket(uint8_t);			//Keepalive packet
		bool packetReceived();						//Check for a packet in the buffer
		void clearReceiveBuffer();					//Clear the receive buffer
		
		//Encryption
		uint8_t* encryptionKey = nullptr;			//Left null until set
		
		//Checksums
		const uint16_t treaclePolynome = 0xac9a;	//Taken from https://users.ece.cmu.edu/~koopman/crc/ as a 'good' polynome
		bool appendChecksumToPacket(uint8_t*,		//Append a checksum to the packet if possible. Also increses the payload size!
			uint8_t&);
		bool validatePacketChecksum(uint8_t*,		//Check the checksum of a packet. Also decreses the payload size!
			uint8_t&);

		//Ticks
		uint32_t* lastTick;							//Track this node's ticks
		uint16_t* nextTick;							//How long until the next tick for each protocol, which is important
		uint16_t maximumTickTime = 60E3;			//Absolute longest time something can be scheduled in the future
		uint16_t tickTimeMargin = 5E3;				//Margin allowed for any scheduled tick time
		void setTickTime();							//Set a new tick time whenever something happens
		bool sendPacketOnTick();					//Send a single packet if it is due, returns true if this happens
		
		//ESP-Now specific settings
		uint8_t espNowProtocolId = 255;				//ID assigned to this protocol if enabled, 255 implies it is not
		uint8_t broadcastMacAddress[6] = {			//Most ESP-Now communications is broadcast
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
		uint8_t preferredespNowChannel = 11;		//It may not be possible to switch to the preferred channel if it is a WiFi client
		uint8_t currentEspNowChannel = 0;			//Track this, as it's not fixed if the device is a WiFi client
		//ESP-Now specific functions
		bool initialiseWiFi();						//Initialise WiFi and return result. Only changes things if WiFi is not already set up when treacle begins
		bool changeWiFiChannel(uint8_t channel);	//Change the WiFi channel
		bool initialiseEspNow();					//Initialise ESP-Now and return result
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
		
		//COBS/Serial specific setting
		uint8_t cobsProtocolId = 255;				//ID assigned to this protocol if enabled, 255 implies it is not
		Stream *cobsUart_ = nullptr;				//COBS happens over a UART
		uint32_t cobsBaudRate = 115200;				//COBS needs a baud rate
		//COBS/Serial specific functions
		bool initialiseCobs();						//Initialise Cobs and return result
		
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
		const char debugString_data[5] PROGMEM = "data";
		const char debugString_sent[5] PROGMEM = "sent";
		const char debugString_ActiveSpaceProtocols[17] PROGMEM = "active protocols";
		const char debugString_SpaceProtocolID[13] PROGMEM = " protocol ID";
		const char debugString_packetSpace[8] PROGMEM = "packet ";
		const char debugString_received[9] PROGMEM = "received";
		const char debugString_dropped[8] PROGMEM = "dropped";
};
extern treacleClass treacle;	//Create an instance of the class, as only one is practically usable at a time
#endif
