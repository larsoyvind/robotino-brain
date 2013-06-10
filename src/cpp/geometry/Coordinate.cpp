#include "Coordinate.h"

#include "Vector.h"

#include <math.h>


Coordinate::Coordinate()
{
	this->_x = 0.0;
	this->_y = 0.0;
}

Coordinate::Coordinate(float x, float y) 
{
	this->_x = x; 
	this->_y = y; 
}

float
Coordinate::x()
{
	return this->_x;
}

float
Coordinate::y()
{
	return this->_y;
}

Vector
Coordinate::getVector( Coordinate target ) 
{
	float dx = ( target.x() - this->_x ); 
	float dy = ( target.y() - this->_y ); 

	float length = sqrt(( dx * dx ) + ( dy * dy )); 
	float direction = atan2( dy, dx );

	return Vector( length, direction );
}

// Overloaded out stream operator
std::ostream & operator << ( std::ostream & output, const Coordinate & c )
{
	output << c._x << "," << c._y;
	return output;
}
