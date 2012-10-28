#ifndef MAYBE_H_
#define MAYBE_H_

#include <cassert>

#include <QList>

/**
 * A container that may or may not contain a value of a given type
 *
 * A Maybe is considered valid if it contains a value, or invalid else.
 *
 * A Maybe essentially behaves like a QList, except that it is limited to one
 * value. Modifying a Maybe will not affect any copies of that Maybe.
 *
 * A Maybe stores a value, but the container is implicitly shared, so copying
 * is very fast as long as the value is not modified.
 *
 * Note that an invalid Maybe of any type can be created by Maybe<>::invalid.
 */
template<typename T=void> class Maybe
{
	public:
		// Construction
		Maybe (const T &value);
		Maybe ();
		Maybe (const Maybe<void> &voidValue);
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

/**
 * Not a useful class on its own, but it contains the static invalid value used
 * when writing Maybe<>::invalid.
 */
template<> class Maybe<void>
{
	public:
		Maybe ();
		static const Maybe<void> invalid;
};


// ******************
// ** Construction **
// ******************

/**
 * Constructs a valid Maybe with the specified value
 */
template<typename T> Maybe<T>::Maybe (const T &value)
{
	list.append (value);
}

/**
 * Constructs an invalid Maybe
 *
 * Consider using Maybe<T>::invalid () instead for more clarity.
 */
template<typename T> Maybe<T>::Maybe ()
{
}

/**
 * Constructs an invalid Maybe
 *
 * This constructor exists so you can omit the type when creating an invalid
 * Maybe of any type and write "Maybe<>::invalid".
 *
 * @param voidValue must be Maybe<>::invalid
 */
template<typename T> Maybe<T>::Maybe (const Maybe<void> &voidValue)
{
	assert (&voidValue==&Maybe<void>::invalid);
}


/**
 * Creates an invalid Maybe
 */
template<typename T> Maybe<T> Maybe<T>::invalid ()
{
	return Maybe<T> ();
}

template<typename T> Maybe<T>::~Maybe ()
{
}


// *****************
// ** Read access **
// *****************

/**
 * Returns true if this Maybe is valid, false if not
 */
template<typename T> bool Maybe<T>::isValid () const
{
	return (!list.isEmpty ());
}

/**
 * Returns a reference to the value
 *
 * This method may only be called if this Maybe is valid. The returned reference
 * gets invalid if the Maybe is changed.
 */
template<typename T> T &Maybe<T>::getValue ()
{
	// This will assert if the value is not valid
	return list[0];
}

/**
 * Returns a pointer to the value so the -> operator can be used directly on the
 * Maybe.
 */
template<typename T> T *Maybe<T>::operator-> ()
{
	// This will assert if the value is not valid
	return &(list[0]);
}

/**
 * Same as getValue, but returns a constant reference
 */
template<typename T> const T &Maybe<T>::getValue () const
{
	// This will assert if the value is not valid
	return list.at (0);
}

/**
 * Same as operator*, but returns a constant pointer
 */
template<typename T> const T *Maybe<T>::operator-> () const
{
	// This will assert if the value is not valid
	return &(list.at (0));
}


// ******************
// ** Write access **
// ******************

template<typename T> void Maybe<T>::setValue (const T &value)
{
	list.clear ();
	list.append (value);
}

template<typename T> void Maybe<T>::clearValue ()
{
	list.clear ();
}


#endif
