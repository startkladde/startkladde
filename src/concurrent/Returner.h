/*
 * Returner.h
 *
 *  Created on: 28.02.2010
 *      Author: Martin Herrmann
 */

#ifndef RETURNER_H_
#define RETURNER_H_

#include "src/StorableException.h"
#include "src/concurrent/Waiter.h"

// http://stackoverflow.com/questions/667077/c-cross-thread-exception-handling-problem-with-boostexception
template<typename T> class Returner
{
	public:
		Returner ();
		Returner (const T &initialValue);
		virtual ~Returner ();

		void returnValue (const T &value);
		void exception (const StorableException &thrownException);

		T returnedValue ();

	private:
		Waiter waiter;
		T value;
		StorableException *thrownException;
};

template<typename T> Returner<T>::Returner ():
	thrownException (NULL)
{
}

template<typename T> Returner<T>::Returner (const T &initialValue):
	value (initialValue), thrownException (NULL)
{
}

template<typename T> Returner<T>::~Returner ()
{
	delete thrownException;
}

template<typename T> void Returner<T>::returnValue (const T &value)
{
	this->value=value;
	waiter.notify ();
}

template<typename T> void Returner<T>::exception (const StorableException &thrownException)
{
	this->thrownException=thrownException.clone ();
	waiter.notify ();
}

template<typename T> T Returner<T>::returnedValue ()
{
	waiter.wait ();
	if (thrownException)
	{
		StorableException *copy=thrownException;
		thrownException=NULL;
		copy->deleteAndThrow ();
	}

	// Not reached if there is an exception
	return value;
}

#endif
