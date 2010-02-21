/*
 * ObjectUsedTask.h
 *
 *  Created on: Aug 30, 2009
 *      Author: Martin Herrmann
 */

#ifndef OBJECTUSEDTASK_H_
#define OBJECTUSEDTASK_H_

template<class T> class ObjectUsedTask: public Task
{
	public:
		ObjectUsedTask (DataStorage &dataStorage, dbId id):
			dataStorage (dataStorage), id (id), result (true)
		{
		}

		virtual QString toString () const
		{
			return QString::fromUtf8 ("Prüfen, ob %1 benutzt ist").arg (T::objectTypeDescription ());
		}

		virtual bool run ()
		{
			return dataStorage.objectUsed<T> (this, id, &result);
		}

		virtual bool getResult () const { return result; }

	private:
		DataStorage &dataStorage;
		dbId id;
		bool result;
};

#endif
