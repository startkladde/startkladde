#ifndef PLANEIDENTIFICATION_H_
#define PLANEIDENTIFICATION_H_

#include <src/db/dbId.h>

class QWidget;
class DbManager;
class Plane;
class FlarmNetRecord;

/**
 * A helper class for identifying the plane of a flight
 *
 * Note that plane identification is not the same as plane lookup (done by the
 * PlaneLookup class): plane lookup finds the plane for a given Flarm ID,
 * whereas plane identification finds the plane for a given flight. In the best
 * case, this consists of just one plane lookup, but it may also involve
 * letting the user choose from several candidates or even creating a new plane.
 * Plane lookup, on the other hand, is non-interactive.
 *
 * See also: the wiki page "Flarm handling".
 */
class PlaneIdentification
{
	public:
		PlaneIdentification ();
		virtual ~PlaneIdentification ();

		static dbId interactiveIdentifyPlane (QWidget *parent, DbManager &dbManager, const QString &flarmId);
		static dbId interactiveIdentifyPlane (QWidget *parent, DbManager &dbManager, dbId flightId);

	protected:
		static bool queryUsePlane (QWidget *parent, const Plane &plane);
		static bool queryCreatePlane (QWidget *parent, const FlarmNetRecord &flarmNetRecord);
};

#endif
