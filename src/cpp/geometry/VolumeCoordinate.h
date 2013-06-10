#ifndef VOLUMECOORDINATE_H
#define VOLUMECOORDINATE_H

#include "Coordinate.h"

#include <iostream>


/**
 * Represents a three-dimentional coordinate
 *
 * Adds another value to a Coordinate to make up a 3d-coordinate and provides ways of manipulation
 */ 
class VolumeCoordinate : public Coordinate
{
 public:
	/**
	 * Default Constructor, initializes values to 0
	 */
	VolumeCoordinate();

	/**
	 * Contructs VolumeCoordinate
	 *
	 * @param	x	X value of the volumecoordinate
	 * @param	y	Y value of the volumecoordinate
	 * @param	z	Z value of the volumecoordinate
	 */
	VolumeCoordinate(float x, float y, float z);

	/**
	 * Gets the value of phi
	 *
	 * @return	The angle value in phi
	 */
	float  z();

	/**
	 * Overload of the << stream operator for a VolumeCoordinate object, allows direct use of a VolumeCoordinate object in a stream out situation
	 */
	friend std::ostream & operator << ( std::ostream & output, const VolumeCoordinate & vc );

 private:
	float
		_z;
};

#endif
