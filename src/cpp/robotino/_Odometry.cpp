#include "headers/_Odometry.h"

#include "headers/Brain.h"

#include "../geometry/Vector.h"

#include <math.h>
#include <iostream>

_Odometry::_Odometry( Brain * pBrain )
	: rec::robotino::api2::Odometry(),
	Axon::Axon( pBrain )
{
	this->updateTime = 0;
}

bool
_Odometry::set( double x, double y, double phi, bool blocking )
{
	if ( rec::robotino::api2::Odometry::set( x / ODOMETRY_ADJUSTMENT_FACTOR, y / ODOMETRY_ADJUSTMENT_FACTOR, phi, blocking ) ) 
	{
		if ( ! blocking ) return true;
		std::cout
			<< "Odometry successfully set to "
			<< x << " , " << y << " , " << phi
			<< "  (adjusted by " << 1 / ODOMETRY_ADJUSTMENT_FACTOR << ")"
			<< std::endl;
		return true;
	}
	else
	{
		std::cerr << "Error: Odometry position could not be set" << std::endl;
		return false;
	}
}

void
_Odometry::analyze()
{}

void
_Odometry::apply()
{}

AngularCoordinate
_Odometry::getPosition()
{
	if ( ( this->brain()->msecsElapsed() - this->updateTime ) > BRAIN_DATA_MAX_AGE )
	{
		this->update();
	}
	return AngularCoordinate( this->x, this->y, this->phi );
}

float
_Odometry::currentAbsSpeed()
{
	return Coordinate( 0.0, 0.0 ).getVector( Coordinate( vx, vy ) ).magnitude();
}

float
_Odometry::currentAbsOmega()
{
	return fabs( this->omega );
}

// Private functions

void
_Odometry::readings( double * x, double * y, double * phi, unsigned int * sequence )
{}

void
_Odometry::readingsEvent( double x, double y, double phi, float vx, float vy, float omega, unsigned int sequence )
{
	this->x = x * ODOMETRY_ADJUSTMENT_FACTOR;
	this->y = y * ODOMETRY_ADJUSTMENT_FACTOR;
	this->phi = phi;
	this->vx = vx;
	this->vy = vy;
	this->omega = omega;
	this->sequence = sequence;
	this->updateTime = this->brain()->msecsElapsed();
}

void
_Odometry::update()
{
	double
		* x = & this->x,
		* y = & this->y,
		* phi = & this->phi;
	
	unsigned int
		* sequence = & this->sequence;

	rec::robotino::api2::Odometry::readings( x, y, phi, sequence );
//	std::cout
//		<< "Odom read:  " << this->x << " " << this->y << " " << this->phi
//		<< std::endl;
	this->x *= ODOMETRY_ADJUSTMENT_FACTOR;
	this->y *= ODOMETRY_ADJUSTMENT_FACTOR;
	this->updateTime = this->brain()->msecsElapsed();
//	std::cout
//		<< "Odom wrote: " << this->x << " " << this->y << " " << this->phi
//		<< std::endl;
}

