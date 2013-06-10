#ifndef VECTOR_H
#define VECTOR_H

#include "Angle.h"
#include "Scalar.h"
#include "Coordinate.h"


/**
 * Represents a geometric vector
 *
 * Combines and Angle and a Scalar to create a Vector
 */ 
class Vector : public Angle, public Scalar
{
 public: 
	/**
	 * Default Constructor, initializes values to 0
	 */
	Vector();

	/**
	 * Contructs vector
	 *
	 * @param	magnitude	Desired magnitude
	 * @param	phi Angle, is normalized to [-pi, pi]
	 * @param	degrees	If true, input is interpreted as degrees. Standard is radians.
	 */
	Vector( float magnitude, float phi, bool degrees = false );

	/**
	 * Calculates the cartesian values of the vector
	 *
	 * @return	Cartesian values stored as a Coordinate
	 */
	Coordinate cartesian();

	/**
	 * Overload of the << stream operator for a Vector  object, allows direct use of a Vector object in a stream out situation
	 */
	friend std::ostream & operator << ( std::ostream & output, const Vector & v );
};

#endif
