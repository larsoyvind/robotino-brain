#include "KinectReader.h"

#include "../geometry/VolumeCoordinate.h"
#include "../tcp/TcpSocket.h"

#include <rec/robotino/api2/Com.h>

#include <stdlib.h>
#include <math.h>       // for fabs()
#include <unistd.h>		// for usleep()
#include <string>
#include <cstring>
#include <iostream>


int KinectReader::readPosition( unsigned int average ) 
{
	std::string input;

	/* Kinect has the following coordinate map:
	 * x -> -left/+right
	 * y -> +up/-down
	 * z -> -closer/+farther
	 * ..with the kinect positioned in 0,0,0
	 * Values given in millimeters
	 *
	 * Mapping to Robotino:
	 * robotinoX = kinectZ
	 * robotinoY = kinectX
	 * robotinoZ = kinectY + kinectHeight
	 * Brain/API2 expected values in meters
	 */

	float
		xCurVal = 0.0,
		yCurVal = 0.0,
		zCurVal = 0.0;

	unsigned int
		i = 0,
		endx = 0,
		endy = 0;

/*	TcpSocket
		tcpClient;

	// Connect to the server
	try 
	{
		std::cerr << "KinectReader: connecting..." << std::endl;
*/		TcpSocket tcpClient( this->port, this->server );
/*	}
	catch ( std::exception * e )
	{
		std::cout << "Kinect - TcpSocket - " << e->what() << std::endl;
		return KINECTREADER_COULD_NOT_CONNECT;
	}
 	
	std::cerr << "KinectReader: connected" << std::endl;
*/
	this->runLoop = true;

	while ( this->runLoop )
	{
		xCurVal = 0.0;
		yCurVal = 0.0;
		zCurVal = 0.0;

		i = 0;

		// Read data from socket
		while ( tcpClient.read( input ) )
		{
	//		std::cout << "Kinect read: " << input << std::endl;
			
			if ( input == "Click" )
			{
				this->clickTime = pCom->msecsElapsed();
				continue;
			}

			endx = input.find( ',', 0 );
			if ( endx == std::string::npos ) continue;
			endy = input.find( ',', endx + 1 );
			if ( endy == std::string::npos ) continue;

			xCurVal += (float) std::atof( input.substr( 0, endx ).c_str() );
			yCurVal += (float) std::atof( input.substr( endx + 1, endy ).c_str() );
			zCurVal += (float) std::atof( input.substr( endy + 1 ).c_str() );

	//		std::cout << "Kinect extracted: " << VolumeCoordinate( xCurVal, yCurVal, zCurVal ) << std::endl;

			// If data is erranous, read again
			if ( ( fabs( xCurVal ) + fabs( yCurVal ) + fabs( zCurVal ) ) < 0.1f ) continue;

			// When the requested number of coordinates has been recieved,
			// exit reading loop to store data
			if ( ++i >= average ) break;
		}

		if ( i < average )
		{
			this->runLoop = false;
			std::cerr << "KinectReader: Connection lost" << std::endl;
			return KINECTREADER_LOST_CONNECTION;
		}

		// Convert to Robotino/Brain standard and store in class variables
		this->x = ( ( zCurVal / average ) / 1000.0 ) + KINECTREADER_DEPTH_ADJUSTMENT;
		this->y = ( ( xCurVal / average ) / 1000.0 );
		this->z = ( ( yCurVal / average ) / 1000.0 ) + this->height;

		this->updateTime = this->pCom->msecsElapsed();
		this->updated = true;
	}

	return KINECTREADER_NORMAL_EXIT;
}

KinectReader::KinectReader( std::string server, std::string port, rec::robotino::api2::Com * pCom ) 
{
	this->server = server;
	this->port = port;
	this->pCom = pCom;

	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;

	this->height = KINECTREADER_MIN_HEIGHT;

	this->updateTime = 0;
	this->clickTime = 0;

	this->runLoop = false;
	this->updated = false;
}

VolumeCoordinate KinectReader::getCoordinate()
{
	this->updated = false;
	return VolumeCoordinate( this->x, this->y, this->z );
}

bool KinectReader::isUpdated()
{
	return this->updated;
}

unsigned int KinectReader::dataAge()
{
	return ( this->pCom->msecsElapsed() - this->updateTime );
}

unsigned int KinectReader::clickAge()
{
	return this->pCom->msecsElapsed() - this->clickTime;
}

bool
KinectReader::isRunning()
{
	return this->runLoop;
}

void
KinectReader::stopLoop()
{
	this->runLoop = false;
}

void
KinectReader::setHeight( float height )
{
	if ( height > KINECTREADER_MIN_HEIGHT )
	{
		this->height = height;
		std::cout << "Kinect height set to " << height << " meters" << std::endl;
	}
	else
		std::cout << "Could not set kinect height to " << height << " meters, too low" << std::endl;
}
