#include "headers/_Bumper.h"

#include "headers/Brain.h"


_Bumper::_Bumper( Brain * pBrain )
	: rec::robotino::api2::Bumper::Bumper()
	  , Axon( pBrain )
{
	this->hasContact = false;
	this->updateTime = 0;
	this->lastContactTime = 0;
}

void
_Bumper::analyze()
{}

void
_Bumper::apply()
{}

bool
_Bumper::contact()
{
	if ( ( this->updateTime + BRAIN_DATA_MAX_AGE ) > this->brain()->msecsElapsed() )
		this->update();

	return this->hasContact;
}

unsigned int
_Bumper::lastContact()
{
	return this->brain()->msecsElapsed() - this->lastContactTime;
}

void
_Bumper::update()
{
	this->hasContact = this->value();
	this->updateTime = this->brain()->msecsElapsed();
	if ( this->hasContact )
	{
		this->lastContactTime = this->updateTime;
	}
}

void
_Bumper::bumperEvent( bool hasContact )
{
	this->hasContact = hasContact;
	
	this->updateTime = this->brain()->msecsElapsed();

	if ( this->hasContact )
		this->lastContactTime = this->updateTime;
}

