/**
 * @file	_Brain.h
 * @brief	Header file for the _Brain class
 */
#ifndef _BRAIN_H
#define _BRAIN_H

#include "../../geometry/AngularCoordinate.h"
#include "../../geometry/VolumeCoordinate.h"

#include <rec/robotino/api2/Com.h>

#include <string>
#include <thread>

class _Bumper;
class _CompactBha;
class _Odometry;
class _OmniDrive;
class _DistanceSensors;

class KinectReader;


/// Desired loop time of the main loop in milliseconds, to avoid overloading
/// the Robotino command bridge
#define BRAIN_LOOP_TIME	50

/// Age of data in milliseconds before update is forced (on read)
/// Used by subclasses to trigger read instead of using stored data
#define BRAIN_DATA_MAX_AGE	200

/// Number of time to run processEvents to flush data before starting main loop
/// To avoid erranous values registered during startup to cause unwanted
/// reactions to invalid sensor data
#define BRAIN_FLUSH_COUNT	6

/// The number of times an external connection will be tried before failing
#define BRAIN_EXTERNAL_CONNECTION_RETRIES	5

/** 
 * The Brain class is the central hub of the Brain framework.
 * Brain acts as a hub for accessing sensor data and triggers analysis of data
 * and commitment of actions in subclasses.
 */
class Brain : public rec::robotino::api2::Com
{
 public:
	/**
	 * Constructs the Brain object
	 *
	 * @param	name		Name of the program
	 * @param	robotinoIP	Robotino's IP address
	 */
	Brain( std::string name, std::string robotinoIP );

	/**
	 * Destructor, ensures a smooth shutdown by nicely shutting down threads.
	 */
	~Brain();

	/**
	 * Gets a pointer to the _OmniDrive object
	 *
	 * @return	Pointer to the _OmniDrive object
	 */
	_OmniDrive * drive();

	/**
	 * Gets a pointer to the _Odometry object
	 *
	 * @return	Pointer to the _Odometry object
	 */
	_Odometry * odom();

	/**
	 * Gets a pointer to the _CompactBha object
	 *
	 * @return	Pointer to the _CompactBha object
	 */
	_CompactBha * cbha();

	/**
	 * Gets a pointer to the KinectReader object
	 *
	 * @return	Pointer to the KinectReader object
	 */
	KinectReader * kinect();
	
	/**
	 * Initializes Brain by connecing to obotino and creating objects in
	 * accordance with available sensors and actuators.
	 *
	 * @return	Integer indicating success or error
	 */
	int initialize();

	/**
	 * Creates a KinectReader object and sarts the thread reading from the
	 * Kinect Server
	 *
	 * @param	server	The IP or URL of domain name of the host of the Kinect
	 * server
	 * @param	port	The port the Kinect server is hosted on
	 * @param	height	The height of the Kinects position, from the floor, in
	 * meters
	 */
	void enableKinect( std::string server, std::string port, float height );
	
	/**
	 * Checks if a Kinect is available
	 *
	 * This function returns true if a KinectReader thread is currently
	 * running.
	 *
	 * @param	Boolean indicating the availability of a Kinect sensor
	 */
	bool kinectIsAvailable();

	/**
	 * Starts the brain loop
	 */
	void start();

	/**
	 * Stops the brain loop
	 */
	void stop();

	/**
	 * Returns the status of the main loop
	 *
	 * @return bool indicating status
	 */
	bool isRunning();

 private:
	std::string
	/// Holds the name of the application, displayed in Robotinos status screen
		name,
	/// The IP of the Robtino
		robotinoIP;

	bool
	/// Status of initialisation
		initializationDone,
	/// Stop variable for main loop
		runMainLoop,
	/// Stop variable for comEvents loop
		runComEventsLoop,
	/// Indiator and stop variable for KinectReader loop
		kinectRunning;

	_Bumper
	/// Holds a pointer to the _Bumper object
	   	* pBumper;

	_DistanceSensors
	/// Holds a pointer to the _DistanceSensors object
	   	* pDistSensors;

	_Odometry
	/// Holds a pointer to the _Odometry object
	   	* pOdom;

	_OmniDrive
	/// Holds a pointer to the _OmniDrive object
	   	* pDrive;

	_CompactBha
	/// Holds a pointer to the _CompactBha object
	   	* pCbha;

	KinectReader
	/// Holds a pointer to the _KinectReader object
		* pKinect;

	std::thread
	/// Thread for running the main loop of Brain
		tBrainMain,
	/// Thread for running the KinectReader loop
		tKinectReader,
	/// Thread for running the processComEvents loop
		tComEvents;


	/**
	 * A looping function who's only job is to periodically trigger
	 * rec::robotino::api2::Com::processComEvents(), to ensure messages from
	 * Com are handled in due time.
	 * This function runs in it's own thread, @c tComEvents, and is started by
	 * the constructor.
	 */
	void processComEventsLoop();

	/**
	 * A looping function which handles reading data from the Kinect server.
	 * If unable to connect, or if the connection is lost, it tries to
	 * connect again according to the BRAIN_EXTERNAL_CONNECTION_RETRIES
	 * parameter.
	 * This function runs in it's own thread, @c tKinectReader, and is started
	 * by a call to enableKinect().
	 */
	void kinectReader();

	/**
	 * A looping function performing the main task of Brain, making subclasses
	 * analyze sensor data and apply actions.
	 * This function runs in it's own thread, @c tBrainMain, and is started by
	 * a call to start();
	 */
	void mainLoop();

	/**
	 * Implementation of virtual function from rec::robotino::api2::Com, called
	 * by processComEvents() when an errorEvent has occured. Prints any error
	 * messages to std::cerr.
	 */
	virtual void errorEvent( const char * errorString );

	/**
	 * Implementation of virtual function from rec::robotino::api2::Com, called
	 * by processComEvents() when a connectedEvent has occured. Prints any event
	 * information to std::cout.
	 */
	virtual void connectedEvent();

	/**
	 * Implementation of virtual function from rec::robotino::api2::Com, called
	 * by processComEvents() when a connectionClosedEvent has occured. Prints a
	 * message to std::cout.
	 */
	virtual void connectionClosedEvent();

	/**
	 * Implementation of virtual function from rec::robotino::api2::Com, called
	 * by processComEvents() when an logEvent has occured. Prints any event
	 * information to std::cout.
	 */
	virtual void logEvent( const char * message, int level );
};

#endif
