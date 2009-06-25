#include "db_proxy.h"

db_id schleppmaschine_flog (sk_db *db, db_id sa_id, sk_time_t t)
	/*
	 * Proxy for db->plane_flying, taking a !!Startart (which should designate
	 * an airtow) instead of an ID.
	 * Parameters:
	 *   - sa: the startart.
	 *   - t: the time, passed on to db_flugzeug_flog.
	 * Return value:
	 *   - the return value of db_flugzeug_flog.
	 */
{
	// TODO move to sk_db

	Plane plane;

	if (schleppflugzeug_aus_startart (db, &plane, sa_id)==0)
	{
		// TODO check more than one
		return db->plane_flying (plane.id, &t);
	}
	else
	{
		return invalid_id;
	}
}	

int schleppflugzeug_aus_startart (sk_db *db, Plane *fz, int sa_num)
	/*
	 * Proxy for db->get_plane_registration, taking a !!Startart (which should designate
	 * an airtow) instead of an ID.
	 * Parameters:
	 *   - sa: the startart.
	 * Parameters set:
	 *   - *fz: the plane data, passed on to get_plane_registration.
	 * Return value:
	 *   - the return value of get_plane_registration.
	 */
{
	// TODO move to sk_db

	LaunchType sa;
	db->get_startart (&sa, sa_num);

	if (!sa.is_airtow ()) return db_err_parameter_error;

	QString registration=sa.get_towplane ();
	if (registration.isEmpty ()) return db_err_parameter_error;

	// TODO Fehler pr�fen.
	QPtrList<Plane> planes; planes.setAutoDelete (true);
	db->list_planes_registration (planes, registration);

	if (planes.isEmpty ())
	{
		return -1;
	}
	else
	{
		// TODO check more than one
		*fz=*(planes.first ());
		return 0;
	}
}

