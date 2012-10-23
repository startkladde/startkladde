#ifndef MAYBE_H_
#define MAYBE_H_

#include <QList>

/**
 * A container that may or may not contain a value of a given type
 *
 * A Maybe is considered valid if it contains a value, or invalid else.
 *
 * A Maybe essentially behaves like a QList, except that it is limited to one
 * value. Modifying a Maybe will not affect any copies of that Maybe.
 *
 * This container is implicitly shared, so copying is very fast.
 */
template<class T> class Maybe
{
	public:
		// Construction
		Maybe (const T &value);
		Maybe ();
		static Maybe<T> invalid ();
		virtual ~Maybe ();

		// Read access
		bool isValid () const;
		T &getValue ();
		T *operator-> ();
		const T &getValue () const;
		const T *operator-> () const;

		// Write access
		void setValue (const T &value);
		void clearValue ();

	private:
		// This is implemented using QList to get free copy constructor,
		// assignment operator and implicit sharing
		QList<T> list;
};


// ******************
// ** Construction **
// ******************

/**
 * Constructs a valid Maybe with the specified value
 */
template<class T> Maybe<T>::Maybe (const T &value)
{
	list.append (value);
}

/**
 * Constructs an invalid Maybe
 *
 * Consider using Maybe<T>::invalid () instead for more clarity.
 */
template<class T> Maybe<T>::Maybe ()
{
}

/**
 * Creates an invalid Maybe
 */
template<class T> Maybe<T> Maybe<T>::invalid ()
{
		return Maybe<T> ();
}

template<class T> Maybe<T>::~Maybe ()
{
}


// *****************
// ** Read access **
// *****************

/**
 * Returns true if this Maybe is valid, false if not
 */
template<class T> bool Maybe<T>::isValid () const
{
	return (!list.isEmpty ());
}

/**
 * Returns a reference to the value
 *
 * This method may only be called if this Maybe is valid. The returned reference
 * gets invalid if the Maybe is changed.
 */
template<class T> T &Maybe<T>::getValue ()
{
	// This will assert if the value is not valid
	return list[0];
}

/**
 * Returns a pointer to the value so the -> operator can be used directly on the
 * Maybe.
 */
template<class T> T *Maybe<T>::operator-> ()
{
	// This will assert if the value is not valid
	return &(list[0]);
}

/**
 * Same as getValue, but returns a constant reference
 */
template<class T> const T &Maybe<T>::getValue () const
{
	// This will assert if the value is not valid
	return list.at (0);
}

/**
 * Same as operator*, but returns a constant pointer
 */
template<class T> const T *Maybe<T>::operator-> () const
{
	// This will assert if the value is not valid
	return &(list.at (0));
}


// ******************
// ** Write access **
// ******************

template<class T> void Maybe<T>::setValue (const T &value)
{
	list.clear ();
	list.append (value);
}

template<class T> void Maybe<T>::clearValue ()
{
	list.clear ();
}


#endif
