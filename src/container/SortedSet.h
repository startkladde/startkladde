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

#endif
