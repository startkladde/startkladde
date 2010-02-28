/*
 * StorableException.h
 *
 *  Created on: 28.02.2010
 *      Author: Martin Herrmann
 */

#ifndef STORABLEEXCEPTION_H_
#define STORABLEEXCEPTION_H_

/**
 * An exception which can be caught, stored, and later rethrown without knowing
 * the precise type of the exception
 *
 * This is achieved by providing two methods, #clone and #deleteAndThrow, which
 * every subclass must reimplement (even if derived from a class implementing
 * the methods).
 *
 * The exception is caught by reference. A copy ist made using #clone and the
 * pointer is stored. Later, the stored exception can be rethrown by calling
 * #deleteAndThrow.
 *
 * An example:
 *   StorableException *storedException;
 *
 *   try
 *   {
 *     // ...
 *   }
 *   catch (StorableException &ex)
 *   {
 *     storedException=ex.clone ();
 *   }
 *
 *   // ...
 *
 *   if (storedException)
 *     storedException->deleteAndThrow;
 *
 * The Returner class uses StorableExceptions to throw exceptions across thread
 * borders.
 */
class StorableException
{
	public:
		virtual ~StorableException ();

		/**
		 * Creates a new instance of the same class
		 *
		 * This method must be reimplemented by every subclass, even if
		 * inherited from a class implementing it.
		 *
		 * @return a new instance of the same actual class; the caller takes
		 *         ownership of the result
		 */
		virtual StorableException *clone () const=0;

		/**
		 * Deletes this instance and throws a copy
		 *
		 * This method must be reimplemented by every subclass, even if
		 * inherited from a class implementing it.
		 *
		 * @throw itself
		 */
		virtual void deleteAndThrow () const=0;
};

#endif /* STORABLEEXCEPTION_H_ */
