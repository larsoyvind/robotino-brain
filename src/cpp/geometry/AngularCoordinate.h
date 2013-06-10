#ifndef ANGULARCOORDINATE_H
#define ANGULARCOORDINATE_H

#include "Coordinate.h"
#include "Angle.h"

#include <iostream>


/**
 * Represents a position and heading
 *
 * Combines a Coordinate and an Angle to create the equivalent of a position and heading
 */ 
class AngularCoordinate : public Coordinate, public Angle
{
 public:
	/**
	 * Default constructor, initializes values to 0
	 */
	AngularCoordinate();

	/**
	 * Constructor, uses inherited constructors from Coordinate and Angle
	 *
	 * @param	x	x-value of coordinate
	 * @param	y	y-value of coordinate
	 * @param	phi	angle in radians
	 */
	AngularCoordinate( float x, float y, float phi );

	/**
	 * Overload of the << stream operator for an AngularCoordinate object, allows direct use of an AngularCoordinate object in a stream out situation
	 */
	friend std::ostream & operator << ( std::ostream & output, const AngularCoordinate & ac );
};

#endif
