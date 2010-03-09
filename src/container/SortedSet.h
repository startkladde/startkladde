/*
 * SortedSet.h
 *
 *  Created on: 09.03.2010
 *      Author: Martin Herrmann
 */

#ifndef SORTEDSET_H_
#define SORTEDSET_H_

#include <QMap>
#include <QList>

/**
 * A generic sorted set, based on QMap
 *
 * Entries are unique.
 *
 * Complexity:
 *   - contains: O(log n)
 *   - insert: O(log n)
 *   - remove: unknown (O(n)?)
 *   - toQList: O(1) (O(n) after change)
 *
 * This class is not thread safe
 */
template<typename T> class SortedSet
{
	public:
		SortedSet (): listValid (false) {}
		virtual ~SortedSet () {}

		// *** Data access
		bool clear ();
		bool contains (const T &value) const;
		bool insert (const T &value);
		bool isEmpty () const;
		bool remove (const T &value);
		int size () const;

		// *** QList
		QList<T> toQList () const;
		SortedSet<T> &operator= (const QList<T> &list);

	protected:
		// *** List
		void invalidateList ();
		QList<T> &generateList () const;

	private:
		QMap<T, int> data;

		mutable QList<T> generatedList;
		mutable bool listValid;
};


// *****************
// ** Data access **
// *****************

/**
 * Removes all items from the set
 *
 * @return true if anything was changed
 */
template<typename T> bool SortedSet<T>::clear ()
{
	if (data.isEmpty ()) return false;

	data.clear ();
	invalidateList ();
	return true;
}

/**
 * Determines whether the item contains the given value
 *
 * @param value a value
 * @return true if the set contains value, false if not
 */
template<typename T> bool SortedSet<T>::contains (const T &value) const
{
	return data.contains (value);
}

/**
 * Inserts the given value into the set, unless it is already present
 *
 * @param value the value to insert
 * @return true if the value was inserted, false if it was already present
 */
template<typename T> bool SortedSet<T>::insert (const T &value)
{
	if (data.contains (value)) return false;

	data.insert (value, 0);
	invalidateList ();
	return true;
}

/**
 * Determines whether the set is empty
 *
 * @return true if the set is empty, false if not
 */
template<typename T> bool SortedSet<T>::isEmpty () const
{
	return data.isEmpty ();
}

/**
 * Removes an entry from the set if it is present
 *
 * @param value the value to remove
 * @return true if the value was present, false if not
 */
template<typename T> bool SortedSet<T>::remove (const T &value)
{
	if (data.remove (value)>1)
	{
		invalidateList ();
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Determines the number of elements in the set
 *
 * @return the number of elements in the set
 */
template<typename T> int SortedSet<T>::size () const
{
	return data.count ();
}


// **********
// ** List **
// **********

/**
 * Returns a QList containing all values from the set, in order
 *
 * The QList is cached, so due to Qt's implicit sharing, it is very fast as
 * long as the set is not changed. If the set is changed, the list will be
 * regnerated on the next access.
 */
template<typename T> QList<T> SortedSet<T>::toQList () const
{
	return generateList ();
}

template<typename T> SortedSet<T> &SortedSet<T>::operator= (const QList<T> &list)
{
	data.clear ();

	foreach (const T &value, list)
		data.insert (value, 0);

	invalidateList ();

	return *this;
}

/**
 * Marks the list as invalid, so it will be regenerated on the next access
 */
template<typename T> void SortedSet<T>::invalidateList ()
{
	listValid=false;
}

/**
 * Regenerates a list if necessary
 *
 * @return a reference to the list
 */
template<typename T> QList<T> &SortedSet<T>::generateList () const
{
	if (!listValid)
	{
		generatedList=data.keys ();
		listValid=true;
	}

	return generatedList;
}

#endif
