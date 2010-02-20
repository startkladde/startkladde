/*
 * DeleteObjectTask.h
 *
 *  Created on: Aug 15, 2009
 *      Author: Martin Herrmann
 */

#ifndef _DeleteObjectTask_h
#define _DeleteObjectTask_h

template<class T> class DeleteObjectTask: public Task
{
	public:
		DeleteObjectTask (DataStorage &dataStorage, db_id id):
			dataStorage (dataStorage), id (id)
		{
		}

		virtual QString toString () const
		{
			return QString::fromUtf8 ("%1 aus der Datenbank löschen").arg (T::objectTypeDescription ());
		}

		virtual bool run ()
		{
			bool success;
			QString message;
			bool completed=dataStorage.deleteObject<T> (this, id, &success, &message);
			setSuccess (success);
			setMessage (message);
			return completed;
		}

	private:
		DataStorage &dataStorage;
		db_id id;
};

#endif /* DELETEOBJECTTASK_H_ */
