#include "headers/_CompactBha.h"

#include "headers/Axon.h"
#include "headers/Brain.h"
#include "headers/_Odometry.h"

#include "../kinect/KinectReader.h"

#include "../geometry/Vector.h"

#include <rec/robotino/api2/CompactBHA.h>
#include <rec/robotino/api2/CompactBHASimple.h>

#include <stdlib.h>
#include <unistd.h>	// usleep
#include <iostream>
#include <iomanip>
#include <list>
#include <math.h>	// fabs


_CompactBha::_CompactBha( Brain * pBrain )
	: rec::robotino::api2::CompactBHA::CompactBHA()
	  , Axon::Axon( pBrain )
{
	float bellowReadings[ CBHA_BELLOWS_COUNT ];
	this->pressures( & bellowReadings[ 0 ] );
	for ( unsigned int i = 0; i < CBHA_BELLOWS_COUNT; i++ )
	{
		this->appliedPressures[ i ] = 0.0;
		this->targetPressures[ i ] = 0.0;
		this->readPressures[ i ] = bellowReadings[ i ];
		this->pressureDeltas[ i ].assign( CBHA_DELTA_DEPTH, 0.0 );
	}

	float potReadings[ CBHA_STRINGPOTS_COUNT ];
	this->stringPots( & potReadings[ 0 ] );
	for ( unsigned int i = 0; i < CBHA_STRINGPOTS_COUNT; i++ )
	{
		this->readPots[ i ] = potReadings[ i ];
		this->potDeltas[ i ].assign( CBHA_DELTA_DEPTH, 0.0 );
	}

	this->readFoilPot = this->foilPot();
	this->foilPotDeltas.assign( CBHA_DELTA_DEPTH, 0.0 );

//	this->largestPressureDelta = 0.0;
//	this->largestPotDelta = 0.0;

	this->compressorsEnabled = false;
	this->waterDrainValve = false;
	this->pressureSensorStatus = false;

	this->_isHolding = false;
	this->isGripping = false;
	this->isReleasing = false;

	this->pressuresUpdated = false;
	this->potsUpdated = false;
	this->foilPotUpdated = false;
	
	this->armPressureRequired = false;
	
	this->touchDetected = false;
	this->recieveDetected = false;
	this->deliverDetected = false;

	this->pressuresUpdateTime = 0;
	this->potsUpdateTime = 0;
	this->foilPotUpdateTime = 0;
	this->gripDoneTime = 0;
	this->releaseDoneTime = 0;

	this->maxArmSpeed = CBHA_PRESSURE_MAX_ADJUST;
}

void
_CompactBha::analyze()
{
		// Insert 0.0 delta values if no new data was recieved
	if ( ! pressuresUpdated )
		for ( unsigned int i = 0; i < CBHA_BELLOWS_COUNT; i++ )
		{
			pressureDeltas[ i ].pop_back();
			pressureDeltas[ i ].push_front( 0.0 );
		}
//	else
//		this->printLatestDeltas( this->pressureDeltas, CBHA_BELLOWS_COUNT );

	if ( ! potsUpdated )
		for ( unsigned int i = 0; i < CBHA_STRINGPOTS_COUNT; i++ )
		{
			potDeltas[ i ].pop_back();
			potDeltas[ i ].push_front( 0.0 );
		}
//	else
//		this->printLatestDeltas( this->potDeltas, CBHA_STRINGPOTS_COUNT );

	if ( ! foilPotUpdated )
	{
		foilPotDeltas.pop_back();
		foilPotDeltas.push_front( 0.0 );
	}

	// Reset updated-flags for next iteration
	this->pressuresUpdated = false;
	this->potsUpdated = false;
	this->foilPotUpdated = false;

	// Reset detection flags for this iteration
	this->touchDetected = false;
	this->recieveDetected = false;
	this->deliverDetected = false;
	this->armPressureRequired = false;


	// Calculate new pressures from targetPressures
	for ( unsigned int i = 0; i < CBHA_BELLOWS_COUNT - 2; i++ )
	{
		float diff = this->targetPressures[ i ] - this->readPressures[ i ];
		if ( fabs( diff ) < maxArmSpeed )
			this->appliedPressures[ i ] = this->targetPressures[ i ];
		else if ( diff > 0 )
			this->appliedPressures[ i ] = this->readPressures[ i ] + maxArmSpeed;
		else // diff < 0
			this->appliedPressures[ i ] = this->readPressures[ i ] - maxArmSpeed;
	}
	
	// Pressures for rotation are applied directly (for now anyways)
	this->appliedPressures[ 6 ] = this->targetPressures[ 6 ];
	this->appliedPressures[ 7 ] = this->targetPressures[ 7 ];

	// Check if arm or rotation is active (pressure required)
	for ( unsigned int i = 0; i < CBHA_BELLOWS_COUNT - 2; i++ ) // Don't care about rotation
	{
		if ( appliedPressures[ i ] > CBHA_PRESSURE_REQUIRED_THRESHOLD )
		{
			this->armPressureRequired = true;
//			std::cout
//				<< "AppliedPressures[ " << i << " ] = "
//				<< appliedPressures[ i ] << " > "
//				<< CBHA_PRESSURE_REQUIRED_THRESHOLD
//				<< std::endl;
			break;
		}
	}


		// Start actual analysis, detect arm "events"

	// Detect if arm is moving because of pressure difference (between read and set)
	bool armActivity = false;
	for ( unsigned int i = 0; i < CBHA_BELLOWS_COUNT - 2; i++ ) // Don't care about rotation
	{
		double diff = fabs( readPressures[ i ] - targetPressures[ i ] );
		if ( diff > CBHA_ARM_ACTIVITY_THRESHOLD )
		{
			armActivity = true;
//			std::cout
//				<< "CompactBha: Arm activity threshold! - Diffs"
//			    << " ("	<< i << ": "
//				<< readPressures[ i ] << " - "
//				<< targetPressures[ i ] << " = "
//				<< diff << " )"
//				<< std::endl;
			break;
		}
		else
		{
			double pressureDeltaSum = this->floatListSum( & this->pressureDeltas[ i ] );
			pressureDeltaSum /= CBHA_DELTA_DEPTH;
			if ( fabs( pressureDeltaSum ) > CBHA_ARM_ACTIVITY_DELTAS_THRESHOLD )
			{
				armActivity = true;
//				std::cout
//					<< "CompactBha: Arm activity threshold! - Deltas"
//					<< " ("	<< i << ": "
//					<< pressureDeltaSum
//					<< " > " << CBHA_ARM_ACTIVITY_DELTAS_THRESHOLD
//					<< " )"
//					<< std::endl;
				break;
			}
		}
	}

	// If arm is not moving, look for interaction triggers
	if ( ! armActivity )
	{
		// Calculate arm up/down motion
		double down = this->floatListSum( & this->potDeltas[ CBHA_INNER_OVER ] );
		down += this->floatListSum( & this->potDeltas[ CBHA_OUTER_OVER ] );
		down /= CBHA_DELTA_DEPTH;

		// Detect if arm is pushed down (given object) and if gripping action is allowed
		if ( ! this->_isHolding
				&& ! this->isGripping
				&& ( this->brain()->msecsElapsed() > this->releaseDoneTime )
				&& down > ( CBHA_GRIP_THRESHOLD ) )
		{
			std::cout
				<< "CompactBha: Recieve!"
				<< " ( " << down << " > " << CBHA_GRIP_THRESHOLD << " )"
				<< std::endl;

			this->recieveDetected = true;
			this->touchDetected = true;
		}

		// Detect if arm is lifted (object is taken)
		if ( this->_isHolding
				&& ! this->isReleasing
				&& ( down < CBHA_RELEASE_THRESHOLD ) )
		{
			std::cout
				<< "CompactBha: Release!"
				<< " ( " << down << " < " << CBHA_RELEASE_THRESHOLD << " )"
				<< std::endl;

			this->deliverDetected = true;
			this->touchDetected = true;
		}

		// If not already detected, check if touched (for calibration)
		if ( ! this->touchDetected )
		{
			for ( unsigned int i = 0; i < CBHA_STRINGPOTS_COUNT / 2 ; i++ )
			{
				double motionSum = fabs( this->potDeltas[ i ].front() + this->potDeltas[ i + 3 ].front() );
				if ( motionSum > CBHA_TOUCHED_THRESHOLD )
				{
					std::cout
						<< "CompactBha: Touched!"
						<< " ( pots " << i << '+' << i+3 << " = " << motionSum << " )"
						<< std::endl;

					this->touchDetected = true;
				}
			}
		}

		/// @todo Detect more cBHA patterns?
	}
}

void
_CompactBha::apply()
{
	if ( this->recieveDetected ) this->grip();
	if ( this->deliverDetected ) this->release();
	if ( this->touchDetected )
	{
		if ( this->waitForTouch )
		{
			int i = 0;
			while ( i++ < 5 && this->waitForTouch )
				usleep( 5000 );
		}
		else
			this->calibrateOdometry();
	}
	
	// Check if gripping is in progress, and if so if it is done
	if ( this->isGripping && ( this->brain()->msecsElapsed() > this->gripDoneTime ) )
	{
		this->setGripperValve1( false ); // Close intake-valve
		this->isGripping = false;
		this->_isHolding = true;
	}

	// Check if releasing is in progress, and if so if it is done
	if ( this->isReleasing && ( this->brain()->msecsElapsed() > this->releaseDoneTime ) )
	{
		this->isReleasing = false;
		this->_isHolding = false;
	}


	// Check if compressor is needed
	if ( this->armPressureRequired || this->rotatePressureRequired || this->isGripping )
		this->compressorsEnabled = true;
	else
		this->compressorsEnabled = false;

	// Apply new states
	this->setCompressorsEnabled( this->compressorsEnabled );
	this->setWaterDrainValve( this->waterDrainValve );
	this->setPressures( this->appliedPressures );
}

bool
_CompactBha::isHolding()
{
	return this->_isHolding;
}

void
_CompactBha::grip()
{
	if ( this->isGripping || this->_isHolding ) return;
	std::cout << "Gripping" << std::endl;
	this->isGripping = true;
	this->isReleasing = false;
	this->setGripperValve1( true );	// Open intake-valve
	this->setGripperValve2( true );	// Close outlet-valve
	this->releaseDoneTime = 0;
	this->gripDoneTime = this->brain()->msecsElapsed() + CBHA_GRIP_TIME_MSECS;
}

void
_CompactBha::release()
{
	if ( this->isReleasing || ! this->_isHolding ) return;
	std::cout << "Releasing" << std::endl;
	this->isReleasing = true;
	this->isGripping = false;
	this->setGripperValve1( false );	// Close intake-valve
	this->setGripperValve2( false );	// Open outlet-valve
	this->gripDoneTime = 0;
	this->releaseDoneTime = this->brain()->msecsElapsed() + CBHA_RELEASE_TIME_MSECS;
}

void
_CompactBha::rotateHorisontal()
{
	this->targetPressures[ CBHA_ROTATE_VERTICAL ] = 0.0;
	this->targetPressures[ CBHA_ROTATE_HORISONTAL ] = CBHA_MAX_PRESSURE;
	this->rotatePressureRequired = true;
}

void
_CompactBha::rotateVertical()
{
	this->targetPressures[ CBHA_ROTATE_VERTICAL ] = CBHA_MAX_PRESSURE;
	this->targetPressures[ CBHA_ROTATE_HORISONTAL ] = 0.0;
	this->rotatePressureRequired = true;
}

void
_CompactBha::rotateRelax()
{
	this->targetPressures[ CBHA_ROTATE_VERTICAL ] = 0.0;
	this->targetPressures[ CBHA_ROTATE_HORISONTAL ] = 0.0;
	this->rotatePressureRequired = false;
}

void
_CompactBha::innerToCoordinate( float x, float y )
{
	rec::robotino::api2::CompactBHASimple::xy2pressure(
			x,
			y,
			& this->targetPressures[ CBHA_INNER_OVER ],
			& this->targetPressures[ CBHA_INNER_LEFT ],
			& this->targetPressures[ CBHA_INNER_RIGHT ]
			);
}

void
_CompactBha::innerToCoordinate( Coordinate c )
{
	this->innerToCoordinate( c.x(), c.y() );
}

void
_CompactBha::outerToCoordinate( float x, float y )
{
	rec::robotino::api2::CompactBHASimple::xy2pressure(
			x,
			y,
			& this->targetPressures[ CBHA_OUTER_OVER ],
			& this->targetPressures[ CBHA_OUTER_LEFT ],
			& this->targetPressures[ CBHA_OUTER_RIGHT ]
			);
}

void
_CompactBha::outerToCoordinate( Coordinate c )
{
	this->outerToCoordinate( c.x(), c.y() );
}

void
_CompactBha::armRelax()
{
	this->targetPressures[ CBHA_INNER_OVER ] = 0.0;
	this->targetPressures[ CBHA_INNER_RIGHT ] = 0.0;
	this->targetPressures[ CBHA_INNER_LEFT ] = 0.0;
	this->targetPressures[ CBHA_OUTER_OVER ] = 0.0;
	this->targetPressures[ CBHA_OUTER_RIGHT ] = 0.0;
	this->targetPressures[ CBHA_OUTER_LEFT ] = 0.0;
}

void
_CompactBha::setMaxArmSpeed( float maxAdjustPerCycle )
{
	if ( maxAdjustPerCycle > CBHA_PRESSURE_MAX_ADJUST )
		this->maxArmSpeed = CBHA_PRESSURE_MAX_ADJUST;
	else if ( maxAdjustPerCycle < CBHA_PRESSURE_MIN_ADJUST )
		this->maxArmSpeed = CBHA_PRESSURE_MIN_ADJUST;
	else
		this->maxArmSpeed = maxAdjustPerCycle;
}

VolumeCoordinate
_CompactBha::getTouchCoordinate()
{
	this->waitForTouch = true;

	VolumeCoordinate touchPos;

	while ( true )
	{
		while ( ! this->touchDetected ) usleep( 5000 );

		if ( this->brain()->kinect()->isUpdated()
			&& this->brain()->kinect()->dataAge() < 100 )
		{
			touchPos = this->brain()->kinect()->getCoordinate();
			break;
		}
	}

	this->waitForTouch = false;
	return touchPos;
}

float
_CompactBha::armTotalPressureDiff()
{
	double sum = 0.0;
	for ( unsigned int i = 0; i > CBHA_BELLOWS_COUNT - 2; i++ )
		sum += fabs( this->targetPressures[ i ] - this->readPressures[ i ] );

	return (float) sum;
}

/*
//	This function is a part of a functionality to observe deltas, it has done
//	it's job, but is kept commented out as a convenience for future developers
void _CompactBha::resetDeltas()
{
	this->largestPressureDelta = 0.0;
	this->largestPotDelta = 0.0;
}
*/


// Private functions

void
_CompactBha::pressuresChangedEvent( const float * pressures, unsigned int size )
{
	for ( unsigned int i = 0; i < size; i++ )
	{
		this->pressureDeltas[ i ].pop_back();
		this->pressureDeltas[ i ].push_front( pressures[ i ] - this->readPressures[ i ] );
		this->readPressures[ i ] = pressures[ i ];
	}
	this->pressuresUpdated = true;
	this->pressuresUpdateTime = this->brain()->msecsElapsed();
}
 
void
_CompactBha::pressureSensorChangedEvent( bool pressureSensor )
{
	this->pressureSensorStatus = pressureSensor;
}

void
_CompactBha::stringPotsChangedEvent( const float * readings, unsigned int size )
{
	for ( unsigned int i = 0; i < size; i++ )
	{
		this->potDeltas[ i ].pop_back();
		this->potDeltas[ i ].push_front( readings[ i ] - this->readPots[ i ] );
		this->readPots[ i ] = readings[ i ];
	}
	this->potsUpdated = true;
	this->potsUpdateTime = this->brain()->msecsElapsed();
}

void
_CompactBha::foilPotChangedEvent( float value )
{
	this->foilPotDeltas.pop_back();
	this->foilPotDeltas.push_front( value - this->readFoilPot );
	this->readFoilPot = value;
	this->foilPotUpdated = true;
	this->foilPotUpdateTime = this->brain()->msecsElapsed();
}

void
_CompactBha::calibrateOdometry()
{
	// Verify precence of kinect
	if ( ! this->brain()->kinectIsAvailable() )
	{
		std::cout
			<< "CompactBha: Calibration aborted, no Kinect present"
			<< std::endl;
	}

	// Check if arm is in use
	if ( this->armPressureRequired )
	{
		std::cout
			<< "CompactBha: Calibration aborted, arm is not in relaxed state"
			<< std::endl;
		return;
	}

	// Check age of kinect coordinate
	unsigned int dataAge = this->brain()->kinect()->dataAge();
	if ( dataAge > CBHA_CALIBRATE_COORD_MAX_AGE )
	{
		std::cout
			<< "CompactBha: Calibration aborted, kinect coordinate too old: "
			<< ( dataAge / 1000.0 ) << " seconds old (max: "
			<< CBHA_CALIBRATE_COORD_MAX_AGE << ')'
			<< std::endl;
		return;
	}

	// Check if driving
	float driveSpeed = this->brain()->odom()->currentAbsSpeed();
	if ( driveSpeed > CBHA_CALIBRATE_MAX_DRIVE_VELOCITY )
	{
		std::cout
			<< "CompactBha: Calibration aborted, drive speed too high: "
			<< driveSpeed << " m/s (max: "
			<< CBHA_CALIBRATE_MAX_DRIVE_VELOCITY << ')'
			<< std::endl;
		return;
	}

	// Check if turning
	float turnSpeed = this->brain()->odom()->currentAbsOmega();
	if ( turnSpeed > CBHA_CALIBRATE_MAX_ROTATION_VELOCITY )
	{
		std::cout
			<< "CompactBha: Calibration aborted, drive turning too fast: "
			<< turnSpeed << " rad/s (max: "
			<< CBHA_CALIBRATE_MAX_ROTATION_VELOCITY << ')'
			<< std::endl;
		return;
	}
	
	// Read volumeCoordinate from kinect
	VolumeCoordinate kinectCoordinate = this->brain()->kinect()->getCoordinate();
	
	// Check if height is reasonable
	if (
		( kinectCoordinate.z() < CBHA_CALIBRATE_MIN_HEIGHT )
		|| ( kinectCoordinate.z() > CBHA_CALIBRATE_MAX_HEIGHT )
		)
	{
		std::cout
			<< "CompactBha: Calibration aborted, coordinate outside height boundries: " 
			<< kinectCoordinate.z() << " meters (min/max: "
			<< CBHA_CALIBRATE_MIN_HEIGHT << '/' << CBHA_CALIBRATE_MAX_HEIGHT << ')'
			<< std::endl;
		return;
	}

	// calculate new center position based on kinect coordinate and arm position
	AngularCoordinate odomPosition = this->brain()->odom()->getPosition();

	/// @todo Project suggestion: Precice position of cBHA gripper derived from potmeters
	Vector cbhaVector( CBHA_ARM_RELAXED_DISTANCE_FROM_CENTER, odomPosition.phi() );
	Coordinate cbhaVectorCartesian = cbhaVector.cartesian();
	
	Coordinate newPosition =
		Coordinate(
				( kinectCoordinate.x() - cbhaVectorCartesian.x() ),
				( kinectCoordinate.y() - cbhaVectorCartesian.y() )
				);

	// Check if new Coordinate is withing reasonable limits
	float deltaX = fabs( newPosition.x() - odomPosition.x() );
	float deltaY = fabs( newPosition.y() - odomPosition.y() );
	if (
		( deltaX > CBHA_CALIBRATE_MAX_XY_DEVIATION )
		|| ( deltaY > CBHA_CALIBRATE_MAX_XY_DEVIATION )
	   )
	{
		std::cout
			<< "CompactBha: Calibration aborted, coordinate deviation too large: ["
			<< deltaX << ',' << deltaY << "] (max: "
			<< CBHA_CALIBRATE_MAX_XY_DEVIATION << ')'
			<< "\n\tcBha correction: " << cbhaVector.cartesian()
			<< std::endl;
		return;
	}

	std::cout
		<< "CompactBha: OdometryCalibration, adjust current position by: ["
		<< deltaX << ',' << deltaY << ']'
		<< std::endl;

	// Update Odometry
	this->brain()->odom()->set( newPosition.x(), newPosition.y(), odomPosition.phi() );
}

void
_CompactBha::printLatestDeltas(
		std::list<float> array[],
		unsigned int size,
		unsigned int precision,
		unsigned int width	) const
{
	std::cout.setf(std::ios::fixed, std::ios::floatfield);
	std::cout.setf(std::ios::showpoint);
	std::cout << std::setprecision( precision );

	for ( unsigned int i = 0; i < size; i++ )
	{
		std::cout
			<< "  [" << i << "]"
			<< std::setw( width ) << array[ i ].front()
			<< " ";
	}
	std::cout << std::endl;
}

double
_CompactBha::floatListSum( std::list<float> * list ) const
{
	double result = 0.0;

	for ( std::list<float>::iterator it = list->begin(); it != list->end(); ++it )
		result += *it;

	return result;
}

/*
//	This function is a part of a functionality to observe deltas, it has done
//	it's job, but is kept commented out as a convenience for future developers
void
_CompactBha::delta( float pressure, float pot )
{
	bool updated = false;
	if ( fabs( pressure ) > this->largestPressureDelta )
	{
		this->largestPressureDelta = fabs( pressure );
		updated = true;
	}
	else if ( fabs( pot ) > this->largestPotDelta )
	{
		this->largestPotDelta = fabs ( pot );
		updated = true;
	}

	if ( updated )
	{
		std::cout << "Largest pressureDelta:  " << this->largestPressureDelta
			<< "   Largest potDelta:  " << this->largestPotDelta << std::endl;
	}
}
*/

