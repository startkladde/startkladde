/*
 * ObjectUsedTask.h
 *
 *  Created on: Aug 30, 2009
 *      Author: mherrman
 */

#ifndef OBJECTUSEDTASK_H_
#define OBJECTUSEDTASK_H_

// TODO reduce dependencies
template<class T> class ObjectUsedTask: public Task
{
	public:
		ObjectUsedTask (DataStorage &dataStorage, db_id id):
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
		db_id id;
		bool result;
};

#endif /* OBJECTUSEDTASK_H_ */
