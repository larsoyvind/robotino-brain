#include "Scalar.h"

#include <math.h>


Scalar::Scalar()
{
	this->setMagnitude( 0.0 );
}

Scalar::Scalar( float magnitude  )
{
	this->setMagnitude( magnitude );
}

void
Scalar::setMagnitude( float magnitude )
{
	this->_magnitude = fabs( magnitude );
}

float
Scalar::magnitude()
{
	return this->_magnitude;
}

// Overloaded out stream operator
std::ostream & operator << ( std::ostream & output, const Scalar & s )
{
	output << s._magnitude; 
	return output;
}
