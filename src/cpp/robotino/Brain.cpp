#include "headers/Brain.h"

#include "headers/_Bumper.h"
#include "headers/_OmniDrive.h"
#include "headers/_CompactBha.h"
#include "headers/_Odometry.h"
#include "headers/_DistanceSensors.h"

#include "../geometry/All.h"

#include "../kinect/KinectReader.h"

#include <rec/robotino/api2/Com.h>

#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h> // Needed by usleep()
#include <thread>


Brain::Brain( std::string name, std::string robotinoIP )
	: rec::robotino::api2::Com( name.c_str(), true, true )
{
	std::cerr << "Brain for Robotino at " << robotinoIP << std::endl;
	this->name = name;
	this->robotinoIP = robotinoIP;


	// Initialize variables
	this->initializationDone = false;
	this->runMainLoop = false;
	this->runComEventsLoop = false;

	// Start ComEvents reader thread
	this->tComEvents = std::thread( & Brain::processComEventsLoop, this );
}

Brain::~Brain()
{
	std::cerr << "Destructing Brain" << std::endl;

	this->stop();

	if ( this->kinectRunning && this->tKinectReader.joinable() )
	{
		std::cerr << "Stopping Kinect reader" << std::endl;
		this->pKinect->stopLoop();
		this->tKinectReader.join();
		std::cerr << "Kinect reader exited" << std::endl;
	}

	std::cerr << "Disconnecting" << std::endl;
	this->disconnectFromServer();

	std::cerr << "Stopping Com event reader" << std::endl;
	this->runComEventsLoop = false;
	this->tComEvents.join();

	std::cerr << "Brain destructed, have a nice day!" << std::endl;
}

_OmniDrive *
Brain::drive()
{
	return this->pDrive;
}

_Odometry *
Brain::odom()
{
	return this->pOdom;
}

_CompactBha *
Brain::cbha()
{
	return this->pCbha;
}

KinectReader *
Brain::kinect()
{
	return this->pKinect;
}

int
Brain::initialize()
{
	std::cerr << "--Initializing Brain" << std::endl;

	// Connect to Robotino
	std::cerr << "- Com" << std::endl;
	std::cerr << "Connecting to Robotino..." << std::endl;
	try
	{
		this->setAddress( this->robotinoIP.c_str() );
		this->connectToServer( true );
		std::cerr << "Connected to Robotino at " << this->robotinoIP << std::endl;
	}
	catch ( const rec::robotino::api2::RobotinoException ex)
	{
		std::cerr << "RobotinoException while connecting:\n\t" << ex.what() << std::endl;
		return 1;
	}
	catch ( const std::exception ex)
	{
		std::cerr << "std::exception while connecting:\n\t" << ex.what() << std::endl;
		return 1;
	}


	// Inspect robotino configuration
	
	std::cerr << "- Bumper" << std::endl;
	this->pBumper = new _Bumper( this );

	std::cerr << "- DistanceSensorArray" << std::endl;
	this->pDistSensors = new _DistanceSensors( this );

	std::cerr << "- Odometry " << std::endl;
	this->pOdom = new _Odometry( this );
	this->pOdom->set( 0.0, 0.0, 0.0 );

	std::cerr << "- OmniDrive " << std::endl;
	this->pDrive = new _OmniDrive( this );

	std::cerr << "- CompactBHA" << std::endl;
	this->pCbha = new _CompactBha( this );


	this->initializationDone = true;
	std::cerr << "--Initialization complete" << std::endl;
	
	return 0;
}

void
Brain::enableKinect( std::string server, std::string port, float height )
{
	if ( this->kinectRunning )
	{
		std::cerr << "Brain: Kinect is already up and running" << std::endl;
		return;
	}

	std::cerr << "Connecting to kinect at " << server << ":" << port << std::endl;
	this->pKinect = new KinectReader( server, port, this );
	this->pKinect->setHeight( height );

	this->tKinectReader = std::thread( & Brain::kinectReader, this );
	
	sleep( 1 );

	if ( this->kinectRunning )
	{
		std::cerr << "Kinect reader thread started" << std::endl;
	}
	else
	{
		std::cerr << "Could not connect to Kinect" << std::endl;
		this->tKinectReader.join();
		this->pKinect = NULL;
	}
}

bool
Brain::kinectIsAvailable()
{
	if ( this->kinectRunning )
		return this->pKinect->isRunning();
	else
		return false;
}

void
Brain::start()
{
	if ( ! this->initializationDone )
	{
		std::cerr << "Brain: Cannot start main loop until initialization is  completed" << std::endl;
		return;
	}
	if ( this->runMainLoop )
	{
		std::cerr << "Brain: Main loop is already running!" << std::endl;
		return;
	}

	this->tBrainMain = std::thread( & Brain::mainLoop, this );
}

void
Brain::stop()
{
	if ( this->runMainLoop )
	{
		std::cerr << "Stopping main loop" << std::endl;
		this->runMainLoop = false;
		this->tBrainMain.join();
		std::cerr << "Main loop exited" << std::endl;
	}
	else
		std::cerr << "Main loop is not running" << std::endl;
}

bool
Brain::isRunning()
{
	return this->runMainLoop;
}


// - Private functions -

void
Brain::processComEventsLoop()
{
	std::cerr << "ComEvents reader thread started" << std::endl;
	this->runComEventsLoop = true;
	int sleepTime = 10;

	while ( this->runComEventsLoop )
	{
		this->processComEvents();
		usleep( sleepTime );
	}
	std::cerr << "ComEvents reader thread exited" << std::endl;
}

void
Brain::kinectReader()
{
	this->kinectRunning = true;
	unsigned int tries = 0;
	while ( this->kinectRunning )
	{
		switch ( this->pKinect->readPosition() )
		{
			case KINECTREADER_NORMAL_EXIT :
				this->kinectRunning = false;
				std::cerr << "Kinect disconnected" << std::endl;
				return;
				break;

			case KINECTREADER_COULD_NOT_CONNECT :
				this->kinectRunning = false;
				std::cerr << "Could not connect to Kinect" << std::endl;
				if ( tries++ < BRAIN_EXTERNAL_CONNECTION_RETRIES ) continue;
				return;
				break;

			case KINECTREADER_LOST_CONNECTION :
				std::cerr << "Lost kinect, reconnecting" << std::endl;
				tries = 0;
				break;

			default:
				this->kinectRunning = false;
				std::cerr << "Kinect: unknown error" << std::endl;
				return;
				break;
		}
	}
}

void
Brain::mainLoop()
{
	std::cerr << "Brain: Main loop starting" << std::endl;
	if ( ! this->initializationDone ) return;
	if ( this->runMainLoop ) return;	// Already running

	this->runMainLoop = true;

	// Flush any erranous readings from startup
	for ( unsigned int i = 0; i < BRAIN_FLUSH_COUNT; i++ )
	{
		this->processEvents();
		usleep( 100000 );
	}

	unsigned int
		loopStartTime,
		loopContinueTime;

	// Read current time (for comparison at the end of the loop)
	loopStartTime = this->msecsElapsed();
	while ( this->runMainLoop )
	{
		// Update all Robotino sensor data
		this->processEvents();

		// Check critical data to see if any action needs to be taken ASAP (_Bumper::contact)
		if ( this->pBumper->contact() )
		{
			this->pDrive->fullStop();
		}
		
		// Call analyzers for all Robotino sensors
		this->pOdom->analyze();
//		this->pDistSensors->analyze();	// Not yet implemented
		this->pCbha->analyze();

		// Call appliers for all Robotino actuators
		this->pDrive->apply();
		this->pCbha->apply();

		// Wait until a minimum of time has passed before looping
		// (to avoid commands queuing up in Robotino)
		loopContinueTime = loopStartTime + BRAIN_LOOP_TIME;

		if ( this->msecsElapsed() > loopContinueTime )
		{
			std::cout << "Brain: exceeded LOOP_TIME, loop computation time used: "
				<< ( this->msecsElapsed() - loopStartTime )
				<< " msecs" << std::endl;
		}
		else
		{
			usleep( ( loopContinueTime - this->msecsElapsed() ) * 1000 );
		}

		loopStartTime = this->msecsElapsed();
	}

	std::cerr << "Brain main loop ended" << std::endl;
}

void
Brain::errorEvent( const char * errorString )
{
	std::cerr << "Brain errorEvent:\n\t" << errorString << std::endl;
}

void
Brain::connectedEvent()
{
	std::cout << "Brain connectedEvent()" << std::endl;
}

void
Brain::connectionClosedEvent()
{
	/// @todo handle (unintentional) loss of connection
	std::cerr << "Brain connectionClosedEvent()" << std::endl;
}

void
Brain::logEvent( const char * message, int level )
{
	std::cout << "Brain logEvent():\t" << level << ": " << message << std::endl; 
}

