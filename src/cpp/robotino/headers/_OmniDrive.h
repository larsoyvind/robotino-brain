/**
 * @file	_OmniDrive.h
 * @brief	Header file for the _OmniDrive class
 */
#ifndef _OMNIDRIVE_H
#define _OMNIDRIVE_H

#include "Axon.h"

#include "../../geometry/Coordinate.h"

#include <rec/robotino/api2/OmniDrive.h>

class Angle;
class Vector;
class AngularCoordinate;


	// Speed
/// The maximum top speed when setting speed manually
#define OMNIDRIVE_MAX_SPEED	0.7

	// Travel

/// The maximum speed Robotino will travel at, in m/s.
/// Travel means going only in x direction (straight forward) and turning to
/// point at the destination.
#define OMNIDRIVE_TRAVEL_MAX_SPEED	0.5
/// The minimum speed Robotino will travel at, in m/s.
/// Above a quarter of this value, this value will be used for speed. Below a
/// quarter, the speed will be set to 0.
#define OMNIDRIVE_TRAVEL_MIN_SPEED	0.05
/// The maximum angle to target position for which Robotino will start going
/// forward, in rad.
/// With an angle below this value, speed will gradually be increased as the
/// angle approaches 0.
#define OMNIDRIVE_TRAVEL_MAX_ANGLE	1.0	// 1.0f ~= 60 degrees
/// The minimum distance to destination allowing for travel rather than manouver,
/// in meters.
#define OMNIDRIVE_TRAVEL_MIN_DISTANCE	0.1
/// The minimum distance to destination that will be assumed as arrived, in
/// meters.
#define OMNIDRIVE_MIN_ACCEPTABLE_DISTANCE	0.02


	// Manouvering

/// The maximum speed Robotino will manouver at.
/// Manouvering means using the drive in both x and y directions, manouvering is
/// automatically engaged when Robotino is close to the destination.
#define OMNIDRIVE_MANOUVER_MAX_SPEED	0.3
/// The minimum speed Robotino will manouver at
/// Above a quarter of this value, this value will be used. Below a quarter,
/// the speed will be set to 0.
#define OMNIDRIVE_MANOUVER_MIN_SPEED	0.05


	// Accelleration

/// The maximum adjustment that will be done to the speed, x or y, each cycle.
/// This is valid for both accelleration and decelleration
/// @todo needs cycle duration factor to stay constant with changes in cycle
/// duration. Should preferrably be given as m/s^2.
#define OMNIDRIVE_VELOCITY_MAX_ADJUST	0.02
/// The maximum adjustment that will be done to the rotation, omega, each cycle.
/// This is valid for both accelleration and decelleration
/// @todo needs cycle duration factor to stay constant with changes in cycle
/// duration. Should preferrably be given as m/s^2.
#define OMNIDRIVE_ROTATE_MAX_ADJUST	0.4


	// Rotation

/// The maximum speed Robotino will rotate at, in rads/s.
#define OMNIDRIVE_ROTATE_MAX_SPEED	1.8
/// The minumum speed Robotino will rotate at, in rads/s.
/// Above a quarter of this value, this value will be used for speed. Below a
/// quarter, the speed will be set to 0.
#define OMNIDRIVE_ROTATE_MIN_SPEED	0.2
/// The maximum deviation in angle that will be assumed as pointing at a target.
/// In rads.
#define OMNIDRIVE_ROTATE_ACCEPTABLE_DELTA_ANGLE	0.01
/// The maximum distance to destination before performing pointing to target.
/// The current stop distance (set by _OmniDrive::setStopWithin() ) is also
/// considered when this is checked.
#define OMNIDRIVE_POINTING_DESTINATION_MAX_DISTANCE	0.05
/// The minimum distance to pointing target for which pointing will be
/// performed.
#define OMNIDRIVE_POINTING_TARGET_MIN_DISTANCE	0.0


/**
 * Reimplementation of the OmniDrive class from RobotinoAPI2
 *
 * The original OmniDrive class handles the omnidirectional drive system of
 * Robotino. This reimplementation enhances the original by automating all
 * drive instructions. By cooperating with the Odometry class _OmniDrive can
 * easily drive Robotino to a destination just by providing the desired
 * coordinate. Other features include pointing at a coordinate, stopping at
 * a desired distance, smooth accelleration and both smooth and emergency
 * stopping.
 *
 * See @link _OmniDrive.h @endlink for documentation of @c \#define parameters
 */
class _OmniDrive : public Axon, public rec::robotino::api2::OmniDrive
{
 public:
	/**
	 * Constructs _OmniDrive
	 *
	 * @param	pBrain	A pointer to the owner Brain object
	 */
	_OmniDrive( Brain * pBrain );

	/**
	 * Gets the current destination
	 *
	 * @return	Current destination as a Coordinate
	 */
	Coordinate destination();

	/**
	 * Sets desired destination
	 *
	 * The destination will be taken into accord at the next execution of
	 * analyze()
	 *
	 * @param	destination	Coordinate of the desired destination
	 */
	void setDestination( Coordinate destination );

	/**
	 * Gets the current pointing target
	 *
	 * @return	The current pointing target as a Coordinate
	 *
	 * @warning	This will return the last set target even if pointing is set
	 * to disabled by @c stopPointing()
	 */
	Coordinate pointAt();

	/**
	 * Sets desired target for pointing.
	 *
	 * Also reenables pointing if this was disabled by stopPointing()
	 *
	 * @attention	Pointing will only be performed when sufficiently close to
	 * the current destination and sufficiently far from the pointing target
	 * see \#defines in @link _OmniDrive.h @endlink
	 */
	void setPointAt( Coordinate target );

	/**
	 * Check if pointing is activated
	 *
	 * @return 	Boolean indicating if pointing is active
	 */
	bool pointingActive();

	/**
	 * Sets @c _doPoint to false, making Robotino stop pointing.
	 *
	 * Pointing can only be resumed by submitting a new coordinate to
	 * setPointAt()
	 */
	void stopPointing();

	/**
	 * Get the currnet stopWithin value
	 *
	 * @return	The current stopWithin value
	 */
	float stopWithin();

	/**
	 * Set the distance from a destination for witch Robotino will consider
	 * sufficently close
	 *
	 * @param	distance	Acceptable distance to destination
	 */
	void setStopWithin( float distance );

	void analyze();

	void apply();

	/**
	 * Sets the @c stop variable to true, making Robotino perform a nice stop.
	 * Does not change destination or pointAt, requries a call to go() to
	 * resume driving, setDestination() is not sufficient.
	 */
	void niceStop();

	/**
	 * Brings Robotino to a full and immediate halt, no questions asked.
	 * Requires call to go() to enable drive, a call to setDestination()
	 * is not sufficient.
	 */
	void fullStop();

	/**
	 * Check if the @c stop variable is set
	 *
	 * @return	Boolean indicating if @c stop is set
	 */
	bool stopIsSet();

	/**
	 * Releases a stop-command by setting the @c stop parameter to false.
	 */
	void go();

	/**
	 * Overrides rec::robotino::api2::OmniDrive::setVelocity()
	 * Suspends the automatic driving system and sets given speeds for
	 * x, y and omega:
	 *
	 * @param xSpeed	Desired speed in x direction
	 * @param ySpeed	Desired speed in y direction
	 * @param omega		Desired rotation speed
	 */
	void setVelocity( float xSpeed, float ySpeed, float omega );
	

 private:
	float
	/// Speed in x direction to be set
		xSpeed,
	/// Speed in y direction to be set
		ySpeed,
	/// Turning speed to be set
		omega,
	/// Previous speed in x direction
		xOld,
	/// Previous speed in y direction
		yOld,
	/// Previous turning speed
		omegaOld,
	/// Distance to target do considere as arrived
		_stopWithin,
	/// Target manual speeds
		targetXSpeed,
		targetYSpeed,
		targetOmega;

	bool
	/// Travel backwards, protecting the cBHA
		travelReversed,
	/// Do not travel, only perform manouvering
		onlyManouver,
	/// Yup, do that
		_doPointAt,
	/// If set, Robotino will stop and will not move.
		stop,
	/// Automatically moving to destination
		autoDrive;

	Coordinate
	/// Coordinate of the current destination
		_destination,
	/// Coordinate of the current pointing target
		_pointAt;


	/**
	 * Calculates speed in the X axis and turning speed to drive towards the
	 * desired destination.
	 *
	 * @param	destinationVector	Vector pointing to destination.
	 */
	void travelTowards( Vector destinationVector );

	/**
	 * Calculates speeds to manouver towards a destination.
	 *
	 * @param	heading	The current heading
	 * @param	destinationVector	A vector pointing to the destination.
	 */
	void manouverTowards( Angle heading, Vector destinationVector );

	/**
	 * Calculates speeds to turn Robotino towards the given target
	 *
	 * @param	position	Current position
	 * @param	target	Pointing target
	 */
	void turnTowards( AngularCoordinate position, Coordinate target );

	/**
	 * Calculates speeds for manouvering. Used for both X and Y directions.
	 *
	 * @param	length	The X or Y deviation from desired destination
	 *
	 * @return	An appropriate speed for approachin the destination
	 */
	float findManouverVelocity( float length );

	/**
	 * Calculates the appropriate travel speed based on the distance and
	 * angle to the destination. Speed will be 0 until the the angle is smaller
	 * than set in OMNIDRIVE_TRAVEL_MAX_ANGLE.
	 *
	 * @param	lenght	Distance to destination
	 * @param	omega	Angle to destination
	 *
	 * @return	An appropriate speed for approaching the destination.
	 */
	float findTravelVelocity( float length, float omega );

	/**
	 * Calculates the neccesary speed to turn to deminish the parameter.
	 * 
	 * @param	deltaAngle	An angle in rad describing the difference between
	 * the current and the desired heading.
	 *
	 * @return	An appropriate speed of turning to approach the desired angle.
	 */
	float findAngularVelocity( float deltaAngle );

	/**
	 * Calculates new speed values based on current and desired speeds.
	 *
	 * @param	newSpeed	The desired speed
	 * @param	currentSpeed	The current speed
	 * @param	rotation	If this is a rotation velocity (omega)
	 *
	 * @return	A speed complying with the set options
	 */
	float softAccellerate( float newSpeed, float currentSpeed, bool rotation = false );
};

#endif
