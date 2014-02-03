#include "headers/_LaserRangeFinder.h"

#include "headers/Axon.h"
#include "headers/Brain.h"

#include <stdlib.h>
#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>


_LaserRangeFinder::_LaserRangeFinder( Brain * pBrain ) :
	Axon::Axon( pBrain ),
	rec::robotino::api2::LaserRangeFinder::LaserRangeFinder()
{
	this->readingsUpdated = false;
	this->updateTime = 0;
}

void
_LaserRangeFinder::analyze()
{
	/// @todo Not yet implemented
	
	this->readingsUpdated = false;
}

void
_LaserRangeFinder::apply()
{}  /// Should be empty as LaserRangeFinder is not an actuator

bool
_LaserRangeFinder::test()
{
	/// @todo As the API does not (yet) have this kind of functionality, this
	/// requires analysing the data sent by the API, and may not be entirely
	/// possible.

	return true;
}

void
_LaserRangeFinder::readingsToString()
{
	std::cerr << "Seq = " << latestReadings.seq
		<< "  Stamp = " << latestReadings.stamp
		<< "\nAngles; min = " << latestReadings.angle_min
		<< "  max = " << latestReadings.angle_max
		<< "  increment = " << latestReadings.angle_increment
		<< "\nTime increment = " << latestReadings.time_increment
		<< "  Scan time = " << latestReadings.scan_time
		<< "\nRange; min = " << latestReadings.range_min
		<< "  max = " << latestReadings.range_max
		<< std::cerr;

	const float *rangev;		// Holder for rangevector
	unsigned int rangec = 0;	// Holder for rangecount

	this->latestReadings.ranges( &rangev, &rangec );

	for ( unsigned int i = 0; i < rangec; i++ )
	{
		if ( i % 5 == 0 )
		{
			if ( i % 50 == 0 )
			{
				std::cerr << std::endl;
			}

			std::cerr << std::setw(5) << std::setprecision( 2 ) << rangev[ i ] << "   ";
		}
	}
	std::cerr << std::endl;
}

// Private functions

void
_LaserRangeFinder::scanEvent(
		const rec::robotino::api2::LaserRangeFinderReadings & scan )
{
	/// @todo Not yet fully implemented, see header file for intended functions
	this->latestReadings = scan;

	this->readingsUpdated = true;
	this->updateTime = this->brain()->msecsElapsed();
}

