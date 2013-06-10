#include "headers/_DistanceSensors.h"

#include "headers/Axon.h"
#include "headers/Brain.h"

#include "../geometry/Angle.h"

#include <rec/robotino/api2/DistanceSensorArray.h>

#include <stdexcept>
#include <string>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>


_DistanceSensors::_DistanceSensors( Brain * pBrain )
	: Axon::Axon( pBrain )
	  , rec::robotino::api2::DistanceSensorArray::DistanceSensorArray()
{
		for ( unsigned int i = 0; i < DISTANCESENSORS_COUNT; i++ )
		{
			this->readDistances[ i ] = 0.0f;
		}

		this->distancesUpdated = false;
}

void
_DistanceSensors::analyze()
{
	/// @todo Not yet implemented
}

void
_DistanceSensors::apply()
{}

float
_DistanceSensors::sensorDistance( unsigned int sensorNo )
{
	if ( sensorNo >= DISTANCESENSORS_COUNT )
	   throw new std::out_of_range( std::string( "Specified sensorNo does not exist" ) );

	return this->readDistances[ sensorNo ];
}

Angle
_DistanceSensors::sensorAngle( unsigned int sensorNo )
{
	if ( sensorNo >= DISTANCESENSORS_COUNT )
	   throw new std::out_of_range( std::string( "Specified sensorNo does not exist" ) );

	return Angle( ( ( 2 * M_PI ) / 9 ) * sensorNo );
}


// Private functions

void
_DistanceSensors::distancesChangedEvent( const float * distances, unsigned int size )
{
		for ( unsigned int i = 0; i < size; i++ )
			this->readDistances[ i ] = distances[ i ];

		this->distancesUpdated = true;
		this->updateTime = this->brain()->msecsElapsed();
}

