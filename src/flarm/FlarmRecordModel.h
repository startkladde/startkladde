#ifndef FLARMRECORDMODEL_H_
#define FLARMRECORDMODEL_H_

#include <QVariant>

#include "src/model/objectList/ObjectModel.h"

class FlarmRecord;

class FlarmRecordModel: public ObjectModel<FlarmRecord *>
{
	public:
		FlarmRecordModel ();
		virtual ~FlarmRecordModel ();

		virtual int columnCount () const;

	protected:
		virtual QVariant displayHeaderData (int column) const;
		virtual QVariant data (FlarmRecord * const &flarmRecord, int column, int role) const;
		virtual QVariant displayData (FlarmRecord * const &flarmRecord, int column) const;
		virtual QVariant alignmentData (FlarmRecord * const &flarmRecord, int column) const;
};

#endif
