#include <stdlib.h>
#include <iostream>
#include <thread>

#include "robotino/headers/Brain.h"

#include "Control.cpp"

#include "kinect/KinectReader.h"

#define ROBOTINO_DEFAULT_IP "10.10.1.57"
#define ROBOTINO_CONNECTION_NAME "RobotinoBrain"

#define KINECT_IP "10.10.1.66"
#define	KINECT_PORT "5000"
#define KINECT_HEIGHT_METERS 0.68

using namespace std;


/**
 * main function for robotinoXT-project
 */
int main( int argc, char *argv[] )
{
	// parse and apply arguments
	
	string name = ROBOTINO_CONNECTION_NAME;
	string robotinoIP = ROBOTINO_DEFAULT_IP;

	if ( argc > 1 )
		robotinoIP = argv[1];

	// initialize console display if applicable

	// new brain
	Brain brain( name, robotinoIP );

	// initialize brain
	int returnValue = brain.initialize();
	// handle any errors
	if ( returnValue )
	{
		cout << "Error during brain initialization" << endl;
		cout << "hello" << endl; 
		return EXIT_FAILURE;
	}
	
	// Enable and connect to Kinect Server
	//brain.enableKinect( KINECT_IP, KINECT_PORT, KINECT_HEIGHT_METERS );
	
	// start brain function (loop)
	brain.start();

	cerr << "Current position: " << brain.odom()->getPosition() << endl;
	

	// Start control program and prompt for user input
	Control c( & brain );
	c.prompt();


	cout << "End of program" << endl;

	return EXIT_SUCCESS;
}
