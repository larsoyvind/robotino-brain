#include "AngularCoordinate.h"

#include "Coordinate.h"

#include <iostream>


AngularCoordinate::AngularCoordinate()
	: Coordinate::Coordinate(),
	Angle::Angle()
{}

AngularCoordinate::AngularCoordinate( float x, float y, float phi )
	: Coordinate::Coordinate( x, y ),
	Angle::Angle( phi )
{}

// Overloaded out stream operator
std::ostream & operator << ( std::ostream & output, const AngularCoordinate & ac )
{
	output << (Coordinate) ac << " " << (Angle) ac;
	return output;
}

