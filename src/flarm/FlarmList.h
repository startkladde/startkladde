#ifndef FLARMLIST_H_
#define FLARMLIST_H_

#include "src/model/objectList/AbstractObjectList.h"
#include "src/flarm/FlarmRecord.h"

class FlarmRecord;

class FlarmList: public AbstractObjectList<FlarmRecord>
{
	public:
		FlarmList (QObject *parent=NULL);
		virtual ~FlarmList ();

		// AbstractObjectList methods
		virtual int size () const;
		virtual const FlarmRecord &at (int index) const;
		virtual QList<FlarmRecord> getList () const;

		void processPflaaSentence (const PflaaSentence &sentence);

//	protected:
//		int findOrCreateFlarmRecord (const QString &flarmId);
//		int findFlarmRecordByFlarmId (const QString &flarmId);



	private:
		int findFlarmRecordByFlarmId (const QString &flarmId);

		QList<FlarmRecord *> flarmRecords;
		QMap<QString, int> flarmRecordByFlarmId;
};

#endif
