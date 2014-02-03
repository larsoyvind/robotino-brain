/**
 * @file	_LaserRangeFinder.h
 * @brief	Header file for the _LaserRangeFinder class
 */
#ifndef _LASERRANGEFINDER_H
#define _LASERRANGEFINDER_H

#include "Axon.h"

#include <rec/robotino/api2/LaserRangeFinder.h>
#include <rec/robotino/api2/LaserRangeFinderReadings.h>


/**
 * Reimplementation of the LaserRangeFinder class from RobotinoAPI2
 *
 * See @link _LaserRangeFinder.h @endlink for documentation of @c \#define
 * parameters
 */
class _LaserRangeFinder :
	public Axon,
	public rec::robotino::api2::LaserRangeFinder
{
 public:
	/**
	 * Constructs _LaserRangeFinder
	 *
	 * @param	pBrain	A pointer to the owner Brain object
	 */
	_LaserRangeFinder( Brain * pBrain );

	void analyze();

	void apply();

	/**
	 * Performs a test to verify the existece of the LaserRangeFinder
	 *
	 * @return  true if LaserRangeFinder exists
	 */
	bool test();
	
	/**
	 * Prints the latest readings
	 *
	 * @todo Ideally, this should return the string instead
	 */
	void readingsToString();

 private:
	rec::robotino::api2::LaserRangeFinderReadings
	/// The last laserRangeFinderReadings object
		latestReadings;

	bool
	/// If the readings were updated in the last cycle
		readingsUpdated;

	unsigned int
	/// The time of the latest uptdate
		updateTime;
	
	/**
	 * Implementation of virtual function from
	 * rec::robotino::api2::LaserRangeFinder.
	 * Called by Brain::processEvents() when the LaserRangeFinder has changed
	 * readings.
	 * Stores the latest values and update time.
	 * See RobotinoAPI2 documentation for details.
	 */
	void scanEvent( const rec::robotino::api2::LaserRangeFinderReadings & scan );
};

#endif
