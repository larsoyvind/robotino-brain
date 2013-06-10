#ifndef ANGLE_H
#define ANGLE_H

#include <iostream>

/**
 * Represents a geometric angle
 *
 * Holds the geometric property of an angle in radians, defined as e=[-pi, pi], and provides ways of manipulation
 */
class Angle 
{
 public:
	/**
	 * Default constructor, initializes values to 0
	 */
	Angle();

	/**
	 * Constructs angle
	 *
	 * @param	phi Angle, is normalized to [-pi, pi]
	 * @param	degrees	If true, input is interpreted as degrees. Standard is radians.
	 */
	Angle( float phi, bool degrees = false );

	/**
	 * Gets the value of phi
	 *
	 * @return	The angle value in phi
	 */
	float phi();

	/**
	 * Sets phi
	 *
	 * @param	phi Angle in rad, is normalized to [-pi, pi]
	 */
	void setPhi( float phi );

	/**
	 * Returns the angle value in degrees, the value will be e=[-180, 180]
	 *
	 * @return	The angle value in degrees
	 */
	float degrees();

	/**
	 * Turns the angle pi ( 180 degrees )
	 */
	void reverse();

	/**
	 * Create the smallest possible angle to turn to match input angle
	 *
	 * @param	phi	The angle to match
	 *
	 * @return	Angle representing the angle to be turned 
	 */
	Angle deltaAngle( Angle phi );

	/**
	 * Overload of the << stream operator for an Angle object, allows direct use of an Angle object in a stream out situation
	 */
	friend std::ostream & operator << ( std::ostream & output, const Angle & a );

 private:
	float
		_phi;
};

#endif
