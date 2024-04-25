/*
 *	An Arduino library for treacle support
 *
 *	https://github.com/ncmreynolds/treacle
 *
 *	Released under LGPL-2.1 see https://github.com/ncmreynolds/treacle/LICENSE for full license
 *
 */
#ifndef treacleCOBS_cpp
#define treacleCOBS_cpp
#include "treacle.h"
/*
 *
 *	COBS functions
 *
 */
#if defined(TREACLE_SUPPORT_COBS)
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
#endif
#endif