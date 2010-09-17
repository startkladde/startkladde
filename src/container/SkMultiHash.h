/*
 * SkMultiHash.h
 *
 *  Created on: 17.09.2010
 *      Author: Martin Herrmann
 */

#ifndef SKMULTIHASH_H_
#define SKMULTIHASH_H_

#include <QMultiHash>

template<class K, class V> class SkMultiHash: public QMultiHash<K, V>
{
	public:
		bool insertUnique (const K &key, const V &value)
		{
			if (contains (key, value))
				return false;

			insert (key, value);
			return true;
		}
};


#endif
