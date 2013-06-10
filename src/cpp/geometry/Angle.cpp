#include "Angle.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <iostream>


Angle::Angle()
{
	this->setPhi( 0.0 );
}

Angle::Angle( float phi, bool degrees )
{
	if ( degrees )
		this->setPhi( phi * ( 180.0 / M_PI ) );
	else
		this->setPhi( phi );
}

void
Angle::setPhi( float phi )
{
	// Reduce to +/- full circle
	float newPhi = fmod( phi, 2 * M_PI );

	// Wrap around if outside bounds ( [-pi,pi] )
	if ( newPhi > M_PI )
		this->_phi = newPhi - ( 2 * M_PI );
	else if ( newPhi < -M_PI )
		this->_phi = newPhi + ( 2 * M_PI );
	else
		this->_phi = newPhi;
}

float
Angle::phi()
{
	return this->_phi;
}

float
Angle::degrees()
{
	return (this->_phi * 180) / M_PI;
}

void
Angle::reverse()
{
	this->setPhi( this->_phi + M_PI );
}

Angle
Angle::deltaAngle( Angle angle )
{
	return Angle( angle.phi() - this->_phi );
}

// Overloaded out stream operator
std::ostream & operator << ( std::ostream & output, const Angle & a )
{
	output << "ø" << a._phi; 
	return output;
}

