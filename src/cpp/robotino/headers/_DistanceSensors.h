/**
 * @file	_DistanceSensors.h
 * @brief	Header file for the _DistanceSensors class
 */
#ifndef _DISTANCESENSORS_H
#define _DISTANCESENSORS_H

#include "Axon.h"

#include "../../geometry/Angle.h"

#include <rec/robotino/api2/DistanceSensorArray.h>


///	The number of distancesensors available
#define DISTANCESENSORS_COUNT 9


/**
 * Reimplementation of the DistanceSensorArray class from RobotinoAPI2
 *
 * In the current state, this class is just a basic implementation. It has no
 * advanced functionality like the other "Brainified" Robotino classes.
 * It does however provide access to the sensors data, and has a function to
 * calculate the Angle a sensor is pointing in.
 *
 * @todo Project suggestion: Use the distanceSensorArray to map detected
 * obstacles as Robotino drives. Use this map as basis for route planning
 * in cooperation with _OmniDrive.
 *
 * See @link _DistanceSensors.h @endlink for documentation of @c \#define
 * parameters
 */
class _DistanceSensors : public Axon, public rec::robotino::api2::DistanceSensorArray
{
 public:
	/**
	 * Constructs _DistanceSensors
	 *
	 * @param	pBrain	A pointer to the owner Brain object
	 */
	_DistanceSensors( Brain * pBrain );

	void analyze();

	void apply();

	/**
	 * Gets the current distance value of the requested sensor
	 *
	 * @param	sensorNo	The number of the desired sensor. The sensors are
	 * numbered from 0 clockwise, with the first sensor (0) pointing straight
	 * ahead.
	 *
	 * @return	The distance value of the queried sensor
	 */
	float sensorDistance( unsigned int sensorNo );

	/**
	 * Gets the angle of the requested sensor
	 *
	 * @param	sensorNo	The number of the desired sensor. The sensors are
	 * numbered from 0 clockwise, with the first sensor (0) pointing straight
	 * ahead.
	 * 
	 * @return	The Angle of the queried sensor
	 */
	Angle sensorAngle( unsigned int sensorNo );

 private:
	float
	/// Array storing the latest values
		readDistances[ DISTANCESENSORS_COUNT ];

	bool
	/// If the distances were updated in the last cycle
		distancesUpdated;

	unsigned int
	/// The time of the latest uptdate
		updateTime;
	
	/**
	 * Implementation of virtual function from
	 * rec::robotino::api2::DistanceSensorArray.
	 * Called by Brain::processEvents() when the any of the DistanceSensor
	 * values have changed.
	 * Stores the latest values and update time.
	 * See RobotinoAPI2 documentation for details.
	 */
	void distancesChangedEvent( const float * distances, unsigned int size );
};

#endif
