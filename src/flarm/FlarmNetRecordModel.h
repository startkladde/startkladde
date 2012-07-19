#ifndef FLARMNETRECORDMODEL_H_
#define FLARMNETRECORDMODEL_H_

#include <QVariant>

#include "src/model/objectList/ObjectModel.h"

class FlarmNetRecord;

class FlarmNetRecordModel: public ObjectModel<FlarmNetRecord *>
{
	public:
		FlarmNetRecordModel ();
		virtual ~FlarmNetRecordModel ();

		virtual int columnCount () const;

	protected:
		virtual QVariant displayHeaderData (int column) const;
		virtual QVariant data (FlarmNetRecord * const &flarmNetRecord, int column, int role) const;
		virtual QVariant displayData (FlarmNetRecord * const &flarmNetRecord, int column) const;
		virtual QVariant alignmentData (FlarmNetRecord * const &flarmNetRecord, int column) const;
};

#endif
