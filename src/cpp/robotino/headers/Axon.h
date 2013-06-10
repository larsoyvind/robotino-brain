#ifndef AXON_H 
#define AXON_H 

class Brain;


/**
 * Abstract class for "extremities" with connection to Brain
 * Should be inherited by every Robotino actuator and sensor class
 */
class Axon
{
 public:
	/**
	 * Constructor, saves the pointer to Brain
	 *
	 * @param	pBrain	Pointer to Brain object
	 */
	Axon( Brain * pBrain )
	{
		this->pBrain = pBrain;
	}

	/**
	 * Returns the Brain pointer
	 *
	 * @return	Pointer to Brain
	 */
	Brain * brain()
	{
		return this->pBrain;
	}

	/**
	 * A virtual function that must be implemented by inheriting classes.
	 * The implementation should perform analysis of any and all sensor values
	 * gathered by the inhereting class.
	 */
	virtual void analyze() = 0;
	
	/**
	 * A virtual function that must be implemented by inheriting classes.
	 * The implementation should consider the result of analyze() and apply
	 * these results to all applicable actuators of the inhereting class.
	 */
	virtual void apply() = 0;

 private:
	Brain
		* pBrain;
};

#endif
