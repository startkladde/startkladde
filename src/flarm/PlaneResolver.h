#ifndef PLANERESOLVER_H_
#define PLANERESOLVER_H_

#include "src/db/dbId.h"
#include "src/container/Maybe.h"
#include "src/flarm/FlarmNetRecord.h"

class Cache;
class Plane;

class PlaneResolver
{
	public:
		class Result
		{
			public:
				dbId planeId;
				Maybe<FlarmNetRecord> flarmNetRecord;

				Result (dbId planeId, const Maybe<FlarmNetRecord> &flarmNetRecord):
					planeId (planeId), flarmNetRecord (flarmNetRecord)
				{
				}

				static Result invalid ()
				{
					return Result (invalidId, Maybe<FlarmNetRecord>::invalid ());
				}

				bool planeFound          () const { return idValid (planeId);  }
				bool flarmNetRecordFound () const { return flarmNetRecord.isValid (); }
		};

		PlaneResolver (Cache &cache);
		virtual ~PlaneResolver ();

		Result resolvePlane (const QString &flarmId);

	protected:
		Result resolvePlaneByFlarmId          (const QString &flarmId);
		Result resolvePlaneByFlarmNetDatabase (const QString &flarmId);

	private:
		Cache &cache;
};

#endif
