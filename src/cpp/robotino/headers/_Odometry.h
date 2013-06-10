/**
 * @file	_Odometry.h
 * @brief	Header file for the _Odometry class
 */
#ifndef _ODOMETRY_H
#define _ODOMETRY_H

#include "Axon.h"

#include "../../geometry/AngularCoordinate.h"

#include <rec/robotino/api2/Odometry.h>

class Brain;


/// Factor to adjust Robotinos internal Odometry by.
///
/// To calibrate this value:
/// 	-# Set the value of this parameter to 1
///		-# Recompile and run
///		-# Mark Robtinos position
///		-# Have Robotino drive 1 meter straight forward
///			- For examply by calling _OmniDrive::setDestination() with a
/// Coordinate of [1, 0]
///		-# Measure the distance travelled, taking into account the value of
/// getPosition()
///		-# Set the measured value, in meters, as the new value.
///
/// @warning	This is only a partial fix. As the error of the recorded
/// odometry values differ by the at any time given speed of the omnidrive,
/// adjusting by this value can never be assumed to be entirely correct.
/// With prolonged continuos use the deviation will only continue to increase,
/// requiring periodical recalibration.
#define ODOMETRY_ADJUSTMENT_FACTOR	1


/**
 * Reimplementation of the Odometry class from RobotinoAPI2
 *
 * The original Odometry class handles the odometry system which calculates the
 * current position from the wheel movements of the OmniDrive.
 * This implementation features correction by value of deviations from the
 * odometry. It also packs position in an AngularCoordinate object for
 * easier handling and computation.
 * 
 * See @link _Odometry.h @endlink for documentation of @c \#define parameters
 */
class _Odometry : public rec::robotino::api2::Odometry, public Axon
{
 public:
	/**
	 * Constructs _CompactBha
	 *
	 * @param	pBrain	A pointer to the owner Brain object
	 */
	_Odometry( Brain * pBrain );

	/**
	 * Set the values of the internal odometry
	 *
	 * This function reimplements the set function of the original Odometry
	 * class from RobotinoAPI2, to ensure the ODOMETRY_ADJUSTMENT_FACTOR is
	 * taken into accord.
	 *
	 * @param	x	The new X-value of the position
	 * @param	y	The new Y-value of the position
	 * @param	phi	The new phi value of the direction
	 * @param	blocking	Tells RobotinoAPI2 to wait and verify if the
	 * call was completed successfully. If false, RobotinoAPI2 will return
	 * true immediately.
	 */
	bool set( double x, double y, double phi, bool blocking = true );

	void analyze();

	void apply();

	/**
	 * Gets the current position and course.
	 *
	 * @return	The current position and course as an AngularCoordinate
	 */
	AngularCoordinate getPosition();

	/**
	 * Gets the current absolute speed.
	 *
	 * This value is the magnitude of the current Robotino speed Vector.
	 *
	 * @return	The current absolute speed
	 */
	float currentAbsSpeed();

	/**
	 * Gets the current absolute omega
	 *
	 * @return	The current absolute omega
	 */
	float currentAbsOmega();

 private:
	double
	/// The current x value of the coordinate
		x,
	/// The current y value of the coordinate
		y,
	/// The current heading
		phi;

	float
	/// The current speed in the x direction
		vx,
	/// The current speed in the y direction
		vy,
	/// The current rotation speed
		omega;

	unsigned int
	/// The time the values were last updated
		updateTime,
	/// The sequence number of the last update
		sequence;

	/**
	 * This function reimplements the readings function of the original Odometry
	 * class from RobotinoAPI2, to ensure the ODOMETRY_ADJUSTMENT_FACTOR is
	 * not overridden. The function is made private as getPosition() is to be
	 * used for this functionality.
	 */
	void readings( double * x, double * y, double * phi, unsigned int * sequence = 0 );

	/**
	 * Implementation of virtual function from rec::robotino::api2::Odometry.
	 * Called by Brain::processEvents() when the Odometry values has changed.
	 * Stores the latest values and update time.
	 * See RobotinoAPI2 documentation for details.
	 */
	void readingsEvent( double x, double y, double phi, float vx, float vy, float omega, unsigned int sequence );

	/**
	 * Requests and saves updated sensor values from Robotino.
	 * Called by getter if a the updateTime value is too long.
	 */
	void update();
};

#endif
