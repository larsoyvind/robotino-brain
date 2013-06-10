#ifndef KINECTREADER_H
#define KINECTREADER_H

#include <string>

class TcpSocket;
class VolumeCoordinate;

namespace rec {
	namespace robotino {
		namespace api2 {
			class Com;
		}
	}
}

#define KINECTREADER_NORMAL_EXIT		0
#define KINECTREADER_LOST_CONNECTION	1
#define KINECTREADER_COULD_NOT_CONNECT	2
#define KINECTREADER_UNKNOWN_ERROR		99

#define KINECTREADER_MIN_HEIGHT		0.05

/// Parameter to adjust for deviation in Kinects depth (z) coordinate
#define KINECTREADER_DEPTH_ADJUSTMENT	-0.1


class KinectReader
{
	public: 
		/**
		 * Construct the Kinect reader object
		 *
		 * @param	server	IP or domain name of serving hosting the kinect
		 * @param	port	Port number to connect to
		 * @param	pCom	Pointer to the active Robotino Com object, used to calculate age of the current stored coordinate
		 */
		KinectReader( std::string server, std::string port, rec::robotino::api2::Com * pCom ); 

		/**
		 * Connects to the server and starts the loop reading coordinates
		 *
		 * @param	average	The number of values over which to calculate an average coordinate
		 * @return	Integer indicating exit status, statuses are defined in this header file
		 */
		int readPosition( unsigned int average = 1 );

		/**
		 * Get the current coordinate
		 *
		 * @return	The current x,y,z coordinate as a VolumeCoordinate
		 */
		VolumeCoordinate getCoordinate();

		/**
		 * Check if values have been updated since they were last read
		 *
		 * @return	Boolean indicating status
		 */
		bool isUpdated();

		/**
		 * Check the age of the current values
		 *
		 * @return The data age in milliseconds
		 */
		unsigned int dataAge();

		/**
		 * Check the age of the last "Click!" from kinect
		 *
		 * @return The "Click!" age in milliseconds
		 */
		unsigned int clickAge();

		/**
		 * Check if the reader is currently running
		 *
		 * @return	Boolean indicating status
		 */
		bool isRunning();

		/**
		 * Signal the reading loop to exit
		 */
		void stopLoop();

		/**
		 * Set the height of the Kinects physical position relative to the floor
		 *
		 * @param	height	Distance from floor to the center of Kinects camera(s) in meters
		 */
		void setHeight( float height );

	private: 
		std::string
		/// The port on which to connect
			port,
		/// The address of the server
			server;

		rec::robotino::api2::Com
		/// Pointer to the Com object (Brain)
			* pCom;

		float
		/// Stored value of X
			x,
		/// Stored value of Y
			y,
		/// Stored value of Z
			z,
		/// The height of Kinects position, used to correct the z coordinate
			height;

		unsigned int
		/// Time of last update
			updateTime,
		/// Time of last registered click
			clickTime;

		bool
		/// Stop flag for the loop
			runLoop,
		/// If the coordinate has been updated since it was last read
			updated;
};

#endif
