#ifndef SCALAR_H
#define SCALAR_H

#include <iostream>


/**
 * Represents a geometric scalar
 *
 * Holds the values of a scalar and provides ways of manipulation
 */ 
class Scalar 
{
 public:
	/**
	 * Default Constructor, initializes values to 0
	 */
	Scalar();

	/**
	 * Contructs Scalar
	 *
	 * @param	magnitude	The desired magnitude of the Scalar
	 */
	Scalar( float magnitude );
	
	/**
	 * Gets magnitude
	 *
	 * @return	The magnitude value
	 */
	float magnitude();

	/**
	 * Sets magnitude
	 * 
	 * @param	magnitude
	 */
	void setMagnitude( float magnitude );

	/**
	 * Overload of the << stream operator for a Scalar object, allows direct use of a Scalar object in a stream out situation
	 */
	friend std::ostream & operator << ( std::ostream & output, const Scalar & s );

 private:
	float
		_magnitude;
};

#endif
