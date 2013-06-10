#ifndef _BUMPER_H 
#define _BUMPER_H

#include "Axon.h"

#include <rec/robotino/api2/Bumper.h>


/**
 * Reimplementation of the Bumper class from RobotinoAPI2
 *
 * The original Bumper class handles only the bumper of the Robotino.
 * The bumper is a pressure sensitive tube running all the way around the
 * Robotino chassis, to be able to detect contact in case of a collision.
 *
 * This class implements the virtual function bumperEvent, that is trigerred
 * after running Brain::processEvents() if contact has ben registered.
 */
class _Bumper : public rec::robotino::api2::Bumper, public Axon
{
 public:
	/**
	 * Constructs _Bumper.
	 *
	 * $param	pBrain	Pointer to the owner Brain object
	 */
	_Bumper( Brain * pBrain	);

	void analyze();

	void apply();

	/**
	 * Gets the current contact status.
	 *
	 * @return	Boolean indicating contact status.
	 */
	bool contact();

	/**
	 * Gets the time since the last registered Contact.
	 *
	 * @return	The time passed since contact was last registered, in
	 * milliseconds
	 */
	unsigned int lastContact();

 private:
	unsigned int
		updateTime,
		lastContactTime;

	bool
		hasContact;

	void update();

	void bumperEvent( bool hasContact );
};

#endif
