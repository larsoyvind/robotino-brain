/**
 * @file	_CompactBha.h
 * @brief	Header file for the _CompactBha class
 */
#ifndef _COMPACTBHA_H
#define _COMPACTBHA_H

#include "Axon.h"

#include "../../geometry/Coordinate.h"
#include "../../geometry/VolumeCoordinate.h"

#include <rec/robotino/api2/CompactBHA.h>

#include <list>


	// Bellows and stringpots mapping (facing Robotino)

#define CBHA_INNER_OVER	0
#define CBHA_INNER_RIGHT	1
#define CBHA_INNER_LEFT	2
#define CBHA_OUTER_OVER	3
#define CBHA_OUTER_RIGHT	4
#define CBHA_OUTER_LEFT	5
#define CBHA_ROTATE_HORISONTAL	6
#define CBHA_ROTATE_VERTICAL	7


	// Array lengths

/// The number of string potentiometers available
#define CBHA_STRINGPOTS_COUNT	6
/// The number of bellows available (for setting desired pressure directly)
#define CBHA_BELLOWS_COUNT	8


	// Pressures

/// Max pressure the bellows will be set to
#define CBHA_MAX_PRESSURE	1.5
/// Maximum pressure adjustment per cycle
#define CBHA_PRESSURE_MAX_ADJUST	0.5
/// Minimum pressure adjustment per cycle
#define CBHA_PRESSURE_MIN_ADJUST	0.05
/// Minimum sum of set bellows pressure for engaging the compressor
#define CBHA_PRESSURE_REQUIRED_THRESHOLD	0.1


	// Timing

/// Milliseconds required to fill gripper sufficiently to grip and hold an
/// object, and grip to be considered completed
#define CBHA_GRIP_TIME_MSECS	4000
/// Milliseconds after release triggered before release is considered completed
#define CBHA_RELEASE_TIME_MSECS	2000


	// Data

/// The number of delta values to keep and consider
#define CBHA_DELTA_DEPTH	4


	// Sensitivity

/// Pressure difference threshold for detecting if arm is actively
/// moving/repositioning
#define CBHA_ARM_ACTIVITY_THRESHOLD		0.1
#define CBHA_ARM_ACTIVITY_DELTAS_THRESHOLD 0.005
/// Necessary arm movement to trigger gripping action, average over deltas
#define CBHA_GRIP_THRESHOLD		0.005
/// Necessary arm movement to trigger release action, average over deltas
#define CBHA_RELEASE_THRESHOLD	-0.005
/// Necessary arm movement to register as touched 
#define CBHA_TOUCHED_THRESHOLD	0.01


	// Metrics

/// The distance from the center of Robotino to the tip of gripper claw
/// when the cBHA arm is in an "relaxed" position. In meters.
#define CBHA_ARM_RELAXED_DISTANCE_FROM_CENTER	0.47


	// Calibration

/// Milliseconds before Kinect coordinate is considered outdated
#define CBHA_CALIBRATE_COORD_MAX_AGE	300
/// Minimum height from floor for kinect coordinate in meters
#define CBHA_CALIBRATE_MIN_HEIGHT	0.1
/// Maximum height from floor for kinect coorinate in meters
#define CBHA_CALIBRATE_MAX_HEIGHT	0.4
/// Maximum difference in old/new coordinates to allow dynamic calibration, in
/// meters, considered separately for x and y.
#define CBHA_CALIBRATE_MAX_XY_DEVIATION	2.2
/// Maximum driving speed for which to allow calibration, in m/s
#define CBHA_CALIBRATE_MAX_DRIVE_VELOCITY	0.01
/// Maximum turning speed (omega) for which to allow calibration, in rad/s
#define CBHA_CALIBRATE_MAX_ROTATION_VELOCITY	0.01


/**
 * Reimplementation of the CompactBHA class from RobotinoAPI2
 *
 * The original CompactBHA class handles the compact Bionic Handling Assistant
 * accessory of the Robotino.
 * This reimplementation enhances the original by adding autonomus reactions
 * to forces applied to the cBHA arm. By continously monitoring the cBHA,
 * certain events are detected by analyzing sensor data. When these events
 * are detected, appropriate actions are taken. For example is the gripper
 * closed when it is pushed down, presumably by the weight of an object
 * handed to it. The compressor is enabled and disabled automatically based
 * on the neccessity of air pressure for the given target state.
 *
 * See @link _CompactBha.h @endlink for documentation of @c \#define parameters
 */
class _CompactBha : public rec::robotino::api2::CompactBHA, public Axon
{
 public:
	/**
	 * Constructs _CompactBha
	 *
	 * @param	pBrain	A pointer to the owner Brain object
	 */
	_CompactBha( Brain * pBrain );

	void analyze();

	void apply();

	/**
	 * Check if arm is currently holding an object.
	 *
	 * @attention	As the cBHA gripper does not have any sensor to detect if
	 * it is actually gripping an object, the variable @c _isHolding is set
	 * @c true if a gripping action (grip()) has completed, and @c false if
	 * a release action (release()) has completed.
	 *
	 * @return	Boolean state of the @c _isHolding parameter, indicating if
	 * an object is currently being held.
	 */
	bool isHolding();

	/**
	 * Initiates a gripping action, the gripper is closed.
	 */
	void grip();

	/**
	 * Initiates a release action, the gripper is opened.
	 */
	void release();

	/**
	 * Rotate the gripper to a horisontal position.
	 *
	 * Sets the bellows to a pressure status which will rotate the gripper to a
	 * horisonal position.
	 */
	void rotateHorisontal();

	/**
	 * Rotate the gripper to a vetical position.
	 *
	 * Sets the bellows to a pressure status which will rotate the gripper to a 
	 * vertical position.
	 */
	void rotateVertical();

	/**
	 * Return the gripper rotation to the initial position.
	 *
	 * Sets the bellows controlling gripper rotation to 0, causing the rotation
	 * to return to the initial position.
	 */
	void rotateRelax();

	/**
	 * Turns the inner part of the cBHA arm to the relative position indicated
	 * by the given coordinate.
	 *
	 * Any coordinate is accepted, but the effecive range of each of the
	 * coordinates values is only [-1, 1].
	 *
	 * @param	x X-value of the coordinate for the desired relative position.
	 * @param	y Y-value of the coordinate for the desired relative position.
	 */
	void innerToCoordinate( float x, float y);

	/// @overload
	void innerToCoordinate( Coordinate c );

	/**
	 * Turns the outer part of the cBHA arm to the relative position indicated
	 * by the given coordinate.
	 *
	 * Any coordinate is accepted, but the effecive range of each of the
	 * coordinates values is only [-1, 1].
	 *
	 * @param	x X-value of the coordinate for the desired relative position.
	 * @param	y Y-value of the coordinate for the desired relative position.
	 */
	void outerToCoordinate( float x, float y);

	/// @overload
	void outerToCoordinate( Coordinate c );

	/**
	 * Return the cBHA arm to the initial position.
	 *
	 * Sets the target pressures of the bellows to 0.0, the pressures will be
	 * gradually reduced according to the parameter set by setMaxArmSpeed().
	 * The compressor wil automatically be turned off when pressure is no longer
	 * needed.
	 */
	void armRelax();

	/**
	 * Control the speed of the cBHA arm by setting the max change in pressure
	 * allowed per cycle.
	 *
	 * @param	maxAdjustPerCycle	The maximum pressure change allowed per
	 * cycle, in bar.
	 */
	void setMaxArmSpeed( float maxAdjustPerCycle );

	/**
	 * Waits for a touch event, then returns the corresponding Kinect
	 * coordinate.
	 *
	 * @return	VolumeCoordinate from KinectReader
	 */
	VolumeCoordinate getTouchCoordinate();

	/**
	 * Calculates the sum of the absolutes of the differences between current
	 * and target pressures of the bellows responsible for controlling the
	 * cbha arm movement (not rotation or gripping).
	 *
	 * @return	The sum of the pressure diffs
	 */
	float armTotalPressureDiff();

//	This function is a part of a functionality to observe deltas, it has done
//	it's job, but is kept commented out as a convenience for future developers
//	void resetDeltas();

 private:
	float
	/// Array holding the currently applied (or to be applied) pressures
		appliedPressures[ CBHA_BELLOWS_COUNT ],
	/// Array holding the desired pressures
		targetPressures[ CBHA_BELLOWS_COUNT ],
	/// Array holding the last read pressures
		readPressures[ CBHA_BELLOWS_COUNT ],

	/// Array holding the last read string potentiometer values
		readPots[ CBHA_STRINGPOTS_COUNT ],
	/// The last read foil potentiometer value
		readFoilPot,

//		largestPressureDelta,
//		largestPotDelta,

	/// The maximum change in pressure allowed, potentially reducing the speed
	/// of motion for the cBHA arm		
		maxArmSpeed;

	std::list<float>
	/// Array holding a number of lists containing delta values for pressures
		pressureDeltas[ CBHA_BELLOWS_COUNT ],
	/// Array holding a number of lists containing delta values for the
	/// potentiometers
		potDeltas[ CBHA_STRINGPOTS_COUNT ],
	/// List containing delta values for the foil potentiometer
		foilPotDeltas;

	bool
	/// The (to be) applied state of the compressor
		compressorsEnabled,
	/// The (to b) applied state of the water drain valve
		waterDrainValve,
	/// The last observed status of the pressure sensor
		pressureSensorStatus,

	/// State of the gripper
		_isHolding,
	/// If the gripper is currently gripping
		isGripping,
	/// If the gripper is currently releasing
		isReleasing,

	/// If pressure is required to reach the desired cBHA arm position
		armPressureRequired,
	/// If pressure is required to perform rotation
		rotatePressureRequired,

	/// If pressures have been updated during the last cycle
		pressuresUpdated,
	/// If string potentiometers have been updated during the last cycle
		potsUpdated,
	/// If the foil potentiometer has been updated during the last cycle
		foilPotUpdated,

	/// If a recieve event has been detected
		recieveDetected,
	/// If a deliver event has been detected
		deliverDetected,
	/// If a touch event has been detected
		touchDetected,
	
	/// A function is waiting for a touch event
		waitForTouch;

	unsigned int
	/// The last time the pressures were updated
		pressuresUpdateTime,
	/// The last time the string potentiometers were updated
		potsUpdateTime,
	/// The last time the foil potentiometer was updated
		foilPotUpdateTime,
	/// The time when a gripping action will be completed
		gripDoneTime,
	/// The time when a relase action will be completed
		releaseDoneTime;

	/**
	 * Implementation of virtual function from rec::robotino::api2::CompactBHA.
	 * Called by Brain::processEvents() when pressures have changed.
	 * Stores delta values and new values.
	 * See RobotinoAPI2 documentation for details.
	 */
	void pressuresChangedEvent( const float * pressures, unsigned int size );
 
	/**
	 * Implementation of virtual function from rec::robotino::api2::CompactBHA.
	 * Called by Brain::processEvents() when the pressureSensor state has
	 * changed. Stores the new value.
	 * See RobotinoAPI2 documentation for details.
	 */
	void pressureSensorChangedEvent( bool pressureSensor );
 
	/**
	 * Implementation of virtual function from rec::robotino::api2::CompactBHA.
	 * Called by Brain::processEvents() when string potentiometer values have
	 * changed.
	 * Stores delta values and new values.
	 * See RobotinoAPI2 documentation for details.
	 */
	void stringPotsChangedEvent( const float * readings, unsigned int size );

	/**
	 * Implementation of virtual function from rec::robotino::api2::CompactBHA.
	 * Called by Brain::processEvents() when the foil potentiometer has changed.
	 * Stores the last delta value and the new value.
	 * See RobotinoAPI2 documentation for details.
	 */
	void foilPotChangedEvent( float value );

	/**
	 * Function for calibrating odometry. This is triggered when a "touch" of
	 * the cBHA is detected. The function checks a number of states to see if
	 * performing calibration is safe, then gets the latest coordinate from
	 * the KinectReader and uses this to determine an accurate position.
	 * If the calculated position is within reasonable limits, the new position
	 * is applied to the odometry.
	 */
	void calibrateOdometry();

	/**
	 * Prints the latest set of delta values from the given array of lists.
	 *
	 * @param	array	The array containing the lists containing the values
	 * to be printed
	 * @param	size	The size of the array
	 * @param	precision	The desired precision of the output numbers
	 * @param	width	The desired witdth of the output number fields
	 */
	void printLatestDeltas(
			std::list<float> array[],
			unsigned int size,
			unsigned int precision = 4,
			unsigned int width = 6 ) const;

	/**
	 * Sums up the floats in a list.
	 * This function is used when calculating the average of a set ov deltas.
	 *
	 * @param	list	The list to be considered
	 */
	double floatListSum( std::list<float> * list ) const;

//	This function is a part of a functionality to observe deltas, it has done
//	it's job, but is kept commented out as a convenience for future developers
//	void delta( float pressure, float pot );
};

#endif
