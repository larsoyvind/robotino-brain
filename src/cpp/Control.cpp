#include "robotino/headers/Brain.h"
#include "robotino/headers/_OmniDrive.h"
#include "robotino/headers/_Odometry.h"
#include "robotino/headers/_CompactBha.h"

#include "geometry/All.h"

#include "kinect/KinectReader.h"

#include <stdlib.h>
#include <iostream>
#include <string>
#include <iomanip>	// Manipulation of output
#include <unistd.h> // Needed by usleep()
#include <math.h>	// Needed by fabs()
#include <vector>
#include <stdexcept>
#include <thread>


/// The maximum height of a coordinate from Kinect that will be considered for fetching
#define CONTROL_FETCH_HEIGHT_LIMIT	0.4

/// The distance between the tip of the gripper and Robotinos center, relative to the floor, when in the calibration position
#define CONTROL_CALIBRATE_ARM_DISPLACEMENT 0.42

/// Microseconds to wait before checking Kinect for a new coordinate
#define CONTROL_KINECT_WAIT	50000


/**
 * Class for controlling Brain and providing a simple user interface for user
 * interaction. Also contains some examples demonstrating a bit of the
 * Cababilites of Brain
 */
class Control
{
 public:
	/**
	 * Constructs the Control object
	 *
	 * @param	A Brain pointer is required to access Brain and subclasses
	 */
	Control( Brain * pBrain )
	{
		this->pBrain = pBrain;
	}

	/**
	 * Provides a simple command interpreter
	 */
	bool prompt()
	{
		std::string
			input = "",
			command = "";

		unsigned int
			separator = 0;

		sleep( 1 );
		std::cerr << std::endl;

		while ( true )
		{
			input = "";
			std::cerr << "Brain $ ";
			std::getline( std::cin, input );

			separator = input.find_first_of( " " );

			this->_stop = true;
			if ( this->tWorker.joinable() )
				this->tWorker.join();
			this->_stop = false;

			command = input.substr( 0, separator );

			separator += 1;

			if ( command == "goto" )
			{
				std::cerr << "Going to " << input.substr( separator ) << std::endl;
				this->goTo( input.substr( separator ) );
			}
			else if ( command == "stop" )
			{
				std::cerr << "Stopping" << std::endl;
				this->pBrain->drive()->niceStop();
			}
			else if ( command == "go" )
			{
				std::cerr << "Continuing (unless I'm already there :) )" << std::endl;
				this->pBrain->drive()->go();
			}
			else if ( command == "pointat" )
			{
				std::cerr << "Pointing at " << input.substr( separator ) << std::endl;
				this->pointAt( input.substr( separator ) );
			}
			else if ( command == "stoppointing" )
			{
				this->pBrain->drive()->stopPointing();
				std::cerr << "Pointing stopped" << std::endl;
			}

			else if ( command == "resetodometry" )
			{
				this->pBrain->drive()->fullStop();
				usleep( 200000 );
				this->pBrain->odom()->set( 0.0, 0.0, 0.0 );
				this->pBrain->drive()->setDestination( Coordinate( 0.0, 0.0 ) );
				this->pBrain->drive()->stopPointing();
				std::cerr << "Odometry set to 0, 0 ø0" << std::endl;
			}
			else if ( command == "printposition" )
			{
				std::cerr << "Current position: " << this->pBrain->odom()->getPosition() << std::endl;
			}

/*			else if ( command == "inner" )
			{
			//	[coordinate]\n"
			}
			else if ( command == "outer" )
			{   
			//	[coordinate]\n"
			}
*/			else if ( command == "relaxarm" )
			{   
				std::cerr << "Relaxing arm" << std::endl;
				this->pBrain->cbha()->armRelax();
			}
			else if ( command == "horisontal" )
			{   
				std::cerr << "Rotate horisontal" << std::endl;
				this->pBrain->cbha()->rotateHorisontal();
			}
			else if ( command == "vertical" )
			{   
				std::cerr << "Rotate vertical" << std::endl;
				this->pBrain->cbha()->rotateVertical();
			}
			else if ( command == "norotate" )
			{   
				std::cerr << "Relaxing rotation" << std::endl;
				this->pBrain->cbha()->rotateRelax();
			}
			else if ( command == "grip" )
			{   
				std::cerr << "Gripping" << std::endl;
				this->pBrain->cbha()->grip();
			}
			else if ( command == "release" )
			{
				std::cerr << "Releasing" << std::endl;
				this->pBrain->cbha()->release();
			}
			else if ( command == "cbhatest" )
			{
				std::cerr << "cBHA test procedure, please wait..." << std::endl;
				this->cbhaTest();
				std::cerr << "Test completed" << std::endl;
			}

			else if ( command == "calibrate" )
			{
				std::cerr << "Calibrate to Kinect coordinate system" << std::endl;
				this->calibratePositionToKinect();
			}
			else if ( command == "fetch" )
			{
				std::cerr << "Fetching" << std::endl;
				this->tWorker = std::thread( & Control::fetch, this, false );
			}
			else if ( command == "deliver" )
			{
				std::cerr << "Delivering" << std::endl;
				this->tWorker = std::thread( & Control::fetch, this, true );
			}
			else if ( command == "serialfetch" )
			{
				std::cerr << "Looping fetch and deliver" << std::endl;
				this->tWorker = std::thread( & Control::serialfetch, this );
			}
			else if ( command == "mimic" )
			{
				std::cerr << "cBHA mimicking" << std::endl;
				this->tWorker = std::thread( & Control::cbhaMimic, this, true );
			}
			else if ( command == "gotokinect" )
			{
				std::cerr << "Following Kinect position" << std::endl;
				this->tWorker = std::thread( & Control::driveToKinectPos, this );
			}
			else if ( command == "pointatkinect" )
			{
				std::cerr << "Pointing to Kinect position" << std::endl;
				this->tWorker = std::thread( & Control::turnToKinectPos, this );
			}

			else if ( command == "brainstop" )
			{
				this->pBrain->stop();
			}
			else if ( command == "brainstart" )
			{
				this->pBrain->start();
			}

			else if ( command == "nobrain" )
			{
				if ( this->pBrain->isRunning() )
				{
					std::cerr << "Brain loop must be stopped first!" << std::endl;
					continue;
				}
				this->aheadAndBack( 1.5 );
			}

			else if ( command == "help" )
			{
				this->printInstructions();
			}
			else if ( command == "exit" )
			{
				std::cerr << "Ending program" << std::endl;
				break;
			}
			else
			{
				std::cerr << "I am sorry, I am not familiar with the command \"" << command << "\"."
					<< "\nPlease try again. For help, use the command \"help\"." << std::endl;
			}
		}
		
		return true;
	}


 private:
	Brain *
	/// The pointer to the Brain object
		pBrain;

	bool
	/// A parameter used to stop function loops
		_stop;

	std::thread
	/// A thread for executing looping function without losing control through the promt
		tWorker;

	/**
	 * Prints usage instructions
	 */
	void printInstructions()
	{
		std::cerr
			<< "\nWhat I can do for you:\n"

			<< "\nDriving:\n"
			<< "goto [coordinate]\tI will drive myself to the given coordinate\n"
			<< "stop\tI will come to a halt, no more, no less.\n"
			<< "go\tI will continue, if I was previously stopped\n"
			<< "pointat [coordinate]\tI will turn myself to point at the given coordinate. I will hovever not do this until I am close enough to my destination.\n"
			<< "stoppointing\tI will not point any more\n"
			<< "resetodometry\tSets all odometry values to 0. Also resets destination and stops any pointing.\n"

			<< "\nArm:\n"
//			<< "\tinner [coordinate]\n"
//			<< "\touter [coordinate]\n"
			<< "relaxarm\n"
			<< "horisontal\n"
			<< "vertical\n"
			<< "norotate\n"
			<< "grip\n"
			<< "release\n"
			<< "cbhatest\tA cbha test routine\n";

		if ( this->pBrain->kinectIsAvailable() )
		{
			std::cerr
			<< "\nWith kinect:\n"
			<< "calibrate\tWill perform a calibration routine, mapping the Kinect coordinate system to Robotino. The prompt will be disabled during calibration.\n"
			<< "fetch\tI will fetch item from your hand (requires Kinect)\n"
			<< "deliver\tThe same as fetch, only I will deliver any item I am currently holding (requires Kinect)\n"
			<< "mimic\tMy arm will mimic you arm (requires Kinect)\n"
			<< "gotokinect\n"
			<< "pointatkinect\n";
		}
		else
		{
			std::cerr
			<< "\n(Kinect unavailable, commands hidden)\n";
		}

		std::cerr
			<< "Brain controls:\n"
			<< "brainstop\tStops the brain loop\n"
			<< "brainstart\tStarts the brain loop\n"

			<< "Meta functions:\n"
			<< "help\tDisplay this help text\n"
			<< "exit\tExit the program\n"

			<< "\nA new command will cancel any running command\n"
			<< std::endl;	
	}

	/**
	 * The fetch function makes Robotino go to fetch an object from a persons
	 * hand. The persons hand must be tracked by Kinect.
	 * 
	 * The functionality demonstrated here was the initial benchmark Brain was
	 * built to solve.
	 *
	 * @param	deliver	Boolean to make Robotino deliver an object instead of
	 * fetching it.
	 */
	void fetch( bool deliver = false )
	{
		if ( ! this->checkKinect( ( deliver ) ? "Deliver" : "Fetch" ) ) return;

		AngularCoordinate initialPosition = this->pBrain->odom()->getPosition();

		this->pBrain->drive()->setStopWithin( CBHA_ARM_RELAXED_DISTANCE_FROM_CENTER );

		bool stopped = false;
		bool high = false;

		while ( ! this->_stop
			   && (	( ! deliver && ! this->pBrain->cbha()->isHolding() )
					|| ( deliver && this->pBrain->cbha()->isHolding() ) ) )
		{
			if ( this->pBrain->kinect()->isUpdated() && this->pBrain->kinect()->dataAge() < 200 )
			{
				stopped = false;
				VolumeCoordinate vc = this->pBrain->kinect()->getCoordinate();

				if ( vc.z() < CONTROL_FETCH_HEIGHT_LIMIT ) 
				{
					high = false;
					std::cout << "Pickup at Coordinate: " << (Coordinate) vc << std::endl;
					this->pBrain->drive()->setDestination( vc );
					this->pBrain->drive()->setPointAt( vc );
					this->pBrain->drive()->go();

					// Hold arm up a bit (mostly for show :) )
					this->pBrain->cbha()->setMaxArmSpeed( 0.1 );
					this->pBrain->cbha()->innerToCoordinate( Coordinate( 0.0, 0.3 ) );
					this->pBrain->cbha()->outerToCoordinate( Coordinate( 0.0, 0.2 ) );
				}
				else
				{
					if ( ! high )
					{
						std::cout << "Stopping, above pickup level" << std::endl;
						high = true;
					}
					this->pBrain->cbha()->setMaxArmSpeed( 0.01 );
					this->pBrain->cbha()->armRelax();
					this->pBrain->drive()->niceStop();
					usleep( 50000 );
				}
			}
			else
			{
				if ( this->pBrain->kinect()->dataAge() > 200 )
				{
					if ( ! stopped )
					{
						std::cout << "Stopping, no coordinate" << std::endl;
						stopped = true;
					}

					this->pBrain->drive()->niceStop();
				}
				usleep( CONTROL_KINECT_WAIT );
			}
		}
		
		if ( this->_stop )
		{
			// Recieved stop signal before completing, do cleanup.
			this->pBrain->cbha()->setMaxArmSpeed( 0.01 );
			this->pBrain->cbha()->armRelax();
			this->pBrain->drive()->stopPointing();
			this->pBrain->drive()->niceStop();
			return;
		}

		std::cout << ( ( deliver ) ? "Delivery" : "Pickup" ) << " done! Returning..." << std::endl;

		this->pBrain->cbha()->setMaxArmSpeed( 0.01 );
		this->pBrain->cbha()->armRelax();
		this->pBrain->drive()->setPointAt( Coordinate( 1.0, 0.0 ) );
		this->pBrain->drive()->setStopWithin( 0 );
		this->pBrain->drive()->setDestination( (Coordinate) initialPosition );
		this->pBrain->drive()->go();
	}

	void serialfetch()
	{
		while ( ! this->_stop )
		{
			// Wait til Robotino has stopped (assuming it is currently running fetch/deliver)
			while ( this->pBrain->odom()->currentAbsSpeed() > 0.01 || this->pBrain->odom()->currentAbsOmega() > 0.01 )
				usleep( 100000 );

			this->fetch( this->pBrain->cbha()->isHolding() );
			sleep( 1 ); // Let Robotino get up to speed
		}
	}

	bool checkKinect( std::string requesterName )
	{
		if ( ! this->pBrain->kinectIsAvailable() )
		{
			std::cerr << requesterName  << " requires Kinect!" << std::endl;
			return false;
		}
		return true;
	}

	/**
	 * Sets the destination coordinate of the OmniDrive object, which makes
	 * Robotino drive to this coordinate.
	 *
	 * @param	input	A string parsable to a coordinate
	 */
	bool goTo( std::string input )
	{
		this->pBrain->drive()->stopPointing();
		Coordinate * destination = this->parseCoordinate( input );
		if ( destination == NULL )
		{
			std::cerr << "Unable to parse coordinate, try again" << std::endl;
			return false;
		}

		std::cerr << "Driving to coordinate " << * destination << std::endl;
		this->pBrain->drive()->setDestination( * destination );
		this->pBrain->drive()->go();
		delete destination;
		return true;
	}

	/**
	 * Sets the point at coordinate of the OmniDrive object, which makes
	 * Robotino turn to point at the coordinate.
	 *
	 * @param	input	A string parsable to a coordinate
	 */
	bool pointAt( std::string input )
	{
		Coordinate * target = this->parseCoordinate( input );
		if ( target == NULL )
		{
			std::cerr << "Unable to parse coordinate, try again" << std::endl;
			return false;
		}

		std::cerr << "Pointing to coordinate " << * target << std::endl;
		this->pBrain->drive()->setPointAt( * target );
		delete target;
		return true;
	}

	/**
	 * Helper function to parse a coordinate from a string
	 *
	 * @param	input	A string parsable to a coordinate
	 */
	Coordinate * parseCoordinate( std::string input )
	{
		size_t separator = input.find_first_of( ": " );

		if ( separator == std::string::npos ) return NULL;

		float
			x,
			y;

		try
		{
			x = std::stof( input.substr( 0, separator )  );
			y = std::stof( input.substr( separator ) );
		}
		catch ( const std::invalid_argument & ex )
		{
			std::cerr << "Could not aquire floats from \"" << input << "\"" << std::endl;
			return NULL;
		}

		return new Coordinate( x, y );
	}

	/**
	 * Make Robotino continuosly drive to the coordinate of a hand tracked
	 * by a Kinect.
	 */
	void driveToKinectPos()
	{
		if ( ! this->checkKinect( "DriveToKinectPos" ) ) return;

		while ( ! this->_stop )
		{
			if ( this->pBrain->kinect()->isUpdated() )
			{
				VolumeCoordinate vc = this->pBrain->kinect()->getCoordinate();
				this->pBrain->drive()->setDestination( vc );
				usleep ( CONTROL_KINECT_WAIT );
			}
			else
			{
				usleep( CONTROL_KINECT_WAIT );
			}
		}
	}

	/**
	 * Make Robotino continuosly turn to the coordinate of a hand tracked
	 * by a Kinect.
	 */
	void turnToKinectPos()
	{
		if ( ! this->checkKinect( "TurnToKinectPos" ) ) return;

		while ( ! this->_stop )
		{
			if ( this->pBrain->kinect()->isUpdated() )
			{
				VolumeCoordinate vc = this->pBrain->kinect()->getCoordinate();
				this->pBrain->drive()->setPointAt( vc );
				usleep ( CONTROL_KINECT_WAIT );
			}
			else
			{
				usleep( CONTROL_KINECT_WAIT );
			}
		}
	}

	/**
	 * A test routine to verify that the cBHA is operational
	 */
	void cbhaTest()
	{
		this->pBrain->cbha()->rotateHorisontal();
		sleep( 4 );
		this->pBrain->cbha()->rotateVertical();
		sleep( 4 );
		this->pBrain->cbha()->rotateRelax();
		sleep( 5 );
		this->pBrain->cbha()->innerToCoordinate( Coordinate( 0.0, 1.0 ) );
		this->pBrain->cbha()->outerToCoordinate( Coordinate( 0.0, 1.0 ) );
		sleep( 10 );
		this->pBrain->cbha()->armRelax();
	}
	
	/**
	 * Make Robotinos cBHA mimic the motions of a hand tracket by a Kinect.
	 *
	 * @param	mirror	(default) Robotino will mirror the hans motions. If you
	 * have Robotino facing away from you, set this to false.
	 */
	void cbhaMimic( bool mirror = true )
	{
		if ( ! this->checkKinect( "Mimic" ) ) return;

		VolumeCoordinate
			zero( 1.5, 0, 1.0 ); // Provide a relatively central point if Click-calibration does not work

		unsigned int
			earliestNewCalibration = 0;

		while ( ! this->_stop )
		{
			if ( this->pBrain->kinect()->isUpdated()  )
			{
				if ( this->pBrain->kinect()->clickAge() < 500
						&& this->pBrain->msecsElapsed() > earliestNewCalibration )
				{
					zero = this->pBrain->kinect()->getCoordinate();
					earliestNewCalibration = this->pBrain->msecsElapsed() + 1000;
					continue;
				}

				VolumeCoordinate vc = this->pBrain->kinect()->getCoordinate();

				float x = vc.x() - zero.x();
				float y = vc.y() - zero.y();
				float z = vc.z() - zero.z();
				
				if ( ! mirror ) y *= -1;

				this->pBrain->cbha()->outerToCoordinate( Coordinate( y * 2.0, z * 2.0 ) );
				this->pBrain->cbha()->innerToCoordinate( Coordinate( y * 1.5, z * 1.5 ) );

				if ( x > 0.1 )
					this->pBrain->cbha()->rotateHorisontal();
				else if (x < -0.1 )
					this->pBrain->cbha()->rotateVertical();
				else
					this->pBrain->cbha()->rotateRelax();

				if ( fabs( x ) > 0.2 )
					this->pBrain->cbha()->grip();
				else
					this->pBrain->cbha()->release();
			}
			else
				usleep( CONTROL_KINECT_WAIT );
		}

		this->pBrain->cbha()->rotateRelax();
		this->pBrain->cbha()->armRelax();
		this->pBrain->cbha()->release();
	}


	void calibratePositionToKinect()
	{
		if ( ! this->checkKinect( "CalibratePositionToKinect" ) ) return;

		this->pBrain->drive()->setStopWithin( 0 );

		std::cerr << "Performing odometry calibration, please wait..." << std::endl;

		this->pBrain->cbha()->grip();
		this->pBrain->drive()->niceStop();
		sleep( 4 );

		// Make sure Robotino is not driving
		while ( this->pBrain->odom()->currentAbsSpeed() > 0.01 || this->pBrain->odom()->currentAbsOmega() > 0.01 )
			usleep( 100000 );

		/// @todo Improvement: Use current coordinate system, so this is not lost if calibration is aborted (not currently applicable).
		this->pBrain->odom()->set( 0.0, 0.0, 0.0 );
	
		this->pBrain->cbha()->innerToCoordinate( 0.0, 0.4 );
		this->pBrain->cbha()->outerToCoordinate( 0.0, 0.4 );
		
		std::cerr << "Wait for arm to reach position..." << std::endl;
		while ( this->pBrain->cbha()->armTotalPressureDiff() > 0.1 )
			usleep( 100000 );

		std::cerr
			<< "Stand beside Robotino and make sure Kinect is reading your hand.\n"
			<< "Then, using your wrist, slightly push down on the tip of Robotinos gripper.\n"
			<< "Robotino will, after a slight pause, drive 1 meter forward"
			<< std::endl;

		VolumeCoordinate kinectCoordinate0 = this->pBrain->cbha()->getTouchCoordinate();

		std::cerr << "First coordinate stored : " << kinectCoordinate0 << "\nGet out of my way!" << std::endl;

		this->pBrain->cbha()->innerToCoordinate( 0.0, 0.2 );
		this->pBrain->cbha()->outerToCoordinate( 0.0, 0.2 );
		usleep ( 500000 );

		this->pBrain->drive()->setDestination( Coordinate( 1.0, 0.0 ) );
		this->pBrain->drive()->go();
		this->pBrain->drive()->setPointAt( Coordinate( 1000.0, 0.0 ) );
		sleep ( 2 );

		this->pBrain->cbha()->innerToCoordinate( 0.0, 0.4 );
		this->pBrain->cbha()->outerToCoordinate( 0.0, 0.4 );

		// Wait until Robotino is in position
		while ( this->pBrain->odom()->currentAbsSpeed() > 0.01 || this->pBrain->odom()->currentAbsOmega() > 0.01 )
			usleep( 100000 );

		this->pBrain->drive()->niceStop();

		std::cerr << "Wait for arm to reach position..." << std::endl;
		while ( this->pBrain->cbha()->armTotalPressureDiff() > 0.1 )
			usleep( 100000 );

		std::cerr
			<< "Again, using your wrist tracked by Kinect, slightly push down on the tip of Robotinos gripper."
			<< std::endl;

		VolumeCoordinate kinectCoordinate1 = this->pBrain->cbha()->getTouchCoordinate();
		std::cerr << "Second coordinate stored: " << kinectCoordinate1 << std::endl;

		// Get current odom position
		AngularCoordinate odomPos1 = this->pBrain->odom()->getPosition();

			// Calculate actual heading and position:
		// This is done using the now known travel direction of Robotino, and
		// the approximate distance between the touched arm and Robotinos
		// center.
		Vector odomDeviation = Coordinate( 0.0, 1.0 ).getVector( odomPos1 );
		Angle phi;
		Coordinate kinectCoordinate0Adjusted = kinectCoordinate0;
		float kinectAngleDiff = 99.0;

		while ( true )
		{
			Vector kinectVector = kinectCoordinate0Adjusted.getVector( kinectCoordinate1 );
			kinectAngleDiff -= fabs( kinectVector.phi() );

			kinectAngleDiff = fabs( kinectAngleDiff );

			phi = Angle( kinectVector.phi() + odomPos1.phi() );

			Coordinate convertedOdomDeviation = Vector( odomDeviation.magnitude(), phi.phi() ).cartesian();

			kinectCoordinate0Adjusted = Coordinate(
					kinectCoordinate0.x() + convertedOdomDeviation.x(),
					kinectCoordinate0.y() + convertedOdomDeviation.y() );

			if ( kinectAngleDiff > 0.01 ) break;
			kinectAngleDiff = fabs( kinectVector.phi() );
		}

		// Calculate the arm offset to apply to the second kinect position
		Coordinate armVector = Vector( CONTROL_CALIBRATE_ARM_DISPLACEMENT, phi.phi() ).cartesian();

		// Calculate and apply coordinates and vector
		float x = kinectCoordinate1.x() - armVector.x();
		float y = kinectCoordinate1.y() - armVector.y();
		if ( this->pBrain->odom()->set( x, y, phi.phi() ) )
		{
			usleep( 200000 );	// Give set a moment to take effect
			std::cerr << "Calibration completed, new position set: " << this->pBrain->odom()->getPosition() << std::endl;
		}

		this->pBrain->cbha()->armRelax();
		this->pBrain->cbha()->release();
		this->pBrain->drive()->setDestination( this->pBrain->odom()->getPosition() );
		this->pBrain->drive()->stopPointing();
	}

	void aheadAndBack( float distance )
	{
		if ( this->pBrain->isRunning() )
		{
			std::cerr << "This function cannot be used when the Brain loop is running" << std::endl;
		}
		AngularCoordinate
			position;

		this->pBrain->odom()->set( position.x(), position.y(), position.phi() );

		while ( position.x() < distance )
		{
			this->pBrain->drive()->setVelocity( 0.4, 0.0, 0.0 );
			usleep( 50000 );
			position = this->pBrain->odom()->getPosition();
		}

		this->pBrain->drive()->setVelocity( 0.0, 0.0, 0.0 );
		sleep( 2 );

		while ( position.x() > 0 )
		{
			this->pBrain->drive()->setVelocity( -0.4, 0.0, 0.0 );
			usleep( 50000 );
			position = this->pBrain->odom()->getPosition();
		}
	}
};
