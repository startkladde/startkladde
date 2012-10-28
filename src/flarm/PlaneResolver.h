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
				Maybe<Plane>          plane;
				Maybe<FlarmNetRecord> flarmNetRecord;

				Result (const Maybe<Plane> &plane, const Maybe<FlarmNetRecord> &flarmNetRecord):
					plane (plane), flarmNetRecord (flarmNetRecord)
				{
				}

				static Result invalid ()
				{
					return Result (Maybe<>::invalid, Maybe<>::invalid);
				}

				bool planeFound          () const { return plane.isValid          (); }
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
