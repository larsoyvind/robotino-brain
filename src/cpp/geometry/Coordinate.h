#ifndef COORDINATE_H
#define COORDINATE_H

#include <iostream>

class Vector;


/**
 * Represents a two-dimentional coordinate
 *
 * Holds the two values that make up a coordinate and provides ways of manipulation
 */ 
class Coordinate
{
 public:
	/**
	 * Default constructor, initializes values to 0
	 */
	Coordinate();

	/**
	 * Contructs Coordinate
	 *
	 * @param	x	X value of the coordinate
	 * @param	y	Y value of the coordinate
	 */
	Coordinate(float x, float y);

	/**
	 * Gets the value of z
	 *
	 * @return	The value of z
	 */
	float x();

	/**
	 * Gets the value of y
	 *
	 * @return	The value of y
	 */
	float y();

	/**
	 * Calculates a vector to the given target
	 *
	 * @param	target	A target Coordinate
	 *
	 * @return	Vector pointing to target
	 */
	Vector getVector( Coordinate target ); 

	/**
	 * Overload of the << stream operator for a Coordinate object, allows direct use of a Coordinate object in a stream out situation
	 */
	friend std::ostream & operator << ( std::ostream & output, const Coordinate & c );

 private:
	float
		_x,
		_y;
};

#endif
