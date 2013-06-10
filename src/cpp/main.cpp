#include <stdlib.h>
#include <iostream>
#include <thread>

#include "robotino/headers/Brain.h"

#include "Control.cpp"

#include "kinect/KinectReader.h"

// #include "tcp/TcpSocket.h"

using namespace std;


/**
 * main function for robotinoXT-project
 */
int main( int argc, char *argv[] )
{

	// parse and apply arguments
	
	string name = "RobotinoBrain"; 
	string robotinoIP = "10.10.1.57";

	if ( argc > 1 )
	{
		robotinoIP = argv[1];
	}

	// initialize console display if applicable

	// new brain
	Brain brain( name, robotinoIP );

	// initialize brain
	int returnValue = brain.initialize();
	// handle any errors
	if ( returnValue )
	{
		cout << "Error during brain initialization" << endl;
		return EXIT_FAILURE;
	}
	
	/// @todo Kinect address and port as parameters
	brain.enableKinect( "10.10.1.66", "50000", 0.68 );
	
	// start brain function (loop)
	brain.start();

	cerr << "Current position: " << brain.odom()->getPosition() << endl;
	

	// Start control program and prompt for user input
	Control c( & brain );
	c.prompt();


	cout << "End of program" << endl;

	return EXIT_SUCCESS;
}
