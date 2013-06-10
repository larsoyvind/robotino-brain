#include "Vector.h"
#include "Scalar.h"
#include "Coordinate.h"

#include <math.h>


Vector::Vector()
	: Angle::Angle(),
	Scalar::Scalar()
{}

Vector::Vector( float magnitude, float phi, bool degrees ) 
	: Angle::Angle( phi, degrees ),
	Scalar::Scalar( magnitude )
{} 

Coordinate
Vector::cartesian()
{
	return Coordinate(
			(float) this->magnitude() * cos( this->phi() ),
			(float) this->magnitude() * sin( this->phi() ) );
}

// Overloaded out stream operator
std::ostream & operator << ( std::ostream & output, const Vector & v )
{
	output << (Scalar) v << " " << (Angle) v; 
	return output;
}
