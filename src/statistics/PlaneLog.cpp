#include "PlaneLog.h"

#include "src/text.h"

PlaneLogEntry::PlaneLogEntry ()
{
	invalid=false;
	anzahl_landungen=0;
	insassen=0;
}

QString PlaneLogEntry::insassen_string () const
{
	// TODO unify for more
	if (insassen==0) return "0";
	else if (insassen==1) return "1";
	else if (insassen==2) return "2";
	else if (insassen==3) return "1/2";
	else return "?";
}

QString PlaneLogEntry::datum_string () const
{
	return datum.toString ("yyyy-MM-dd");
}

QString PlaneLogEntry::zeit_start_string (bool no_letters) const
{
	if (zeit_start.is_null ())
		return "-";
	else
		return zeit_start.to_string ("%H:%M", tz_utc, 0, no_letters);
}

QString PlaneLogEntry::zeit_landung_string (bool no_letters) const
{
	if (zeit_landung.is_null ())
		return "-";
	else
		return zeit_landung.to_string ("%H:%M", tz_utc, 0, no_letters);
}

QString PlaneLogEntry::betriebsdauer_string () const
{
	return betriebsdauer.to_string ("%H:%M", tz_timespan);
}

QString PlaneLogEntry::anzahl_landungen_string () const
{
	if (anzahl_landungen==0)
		return "-";
	else
		return QString::number (anzahl_landungen);
}


bool makePlaneLogEntry (PlaneLogEntry *bb_entry, Database *db, QList<Flight *> &flights, Plane &fz, QDate date)
	// No plane/date checking is done.
{
	// Skip if the the list is empty
	if (!flights.isEmpty ())
	{
		// Some of the information can be filled in immediately
		bb_entry->registration=fz.registration;
		bb_entry->flugzeug_typ=fz.typ;
		bb_entry->datum=date;
		bb_entry->invalid=false;
		bb_entry->club=fz.club;

		// The list is sorted
		// Iterate over the flights, gathering information.
		foreach (Flight *flight, flights)
		{
			// If any flight is not yet finished the collective entry is
			// invalid
			if (flight->finished ())
			{
				bb_entry->anzahl_landungen+=flight->landungen;

				// See how many persons there were
				// TODO add num_persons (flugtyp)
				if (!begleiter_erlaubt (flight->flugtyp) || id_invalid (flight->begleiter))
					bb_entry->insassen|=1;
				else
					bb_entry->insassen|=2;

				// Add up total operation time
				bb_entry->betriebsdauer.add_time (flight->flugdauer ());
			}
			else
			{
				bb_entry->invalid=true;
			}
		}

		// TODO error checking last pilot exists?

		// Get additional data
		Person last_pilot;
		db->get_person (&last_pilot, flights.last ()->pilot);

		// Fill in the rest of the information
		bb_entry->name=last_pilot.name ();	// TODO all pilots
		bb_entry->ort_von=flights.first ()->startort;
		bb_entry->ort_nach=flights.last ()->zielort;
		bb_entry->zeit_start=flights.first ()->startzeit;
		bb_entry->zeit_landung=flights.last ()->landezeit;

		if (flights.first ()->flugtyp==ftTow)
		{
			if (!bb_entry->bemerkungen.isEmpty ()) bb_entry->bemerkungen="; "+bb_entry->bemerkungen;
			if (flights.count ()<=1)
				bb_entry->bemerkungen="Schleppflug"+bb_entry->bemerkungen;
			else
				bb_entry->bemerkungen="Schleppflüge"+bb_entry->bemerkungen;
		}

		return true;
	}
	else
	{
		return false;
	}
}

void addPlaneLogEntry (QList<PlaneLogEntry *> &bb, Database *db, QList<Flight *> &flights, Plane &fz, QDate date)
{
	PlaneLogEntry *bb_entry=new PlaneLogEntry;
	bool entry_ok=makePlaneLogEntry (bb_entry, db, flights, fz, date);

	if (entry_ok)
		bb.append (bb_entry);
	else
		delete bb_entry;
}

void makePlaneLogPlane (QList<PlaneLogEntry *> &bb, Database *db, QDate date, Plane &fz, QList<Flight *> &flights)
	// flights may contain flights which don't belong to the plane
{
	// First, make a sorted list of all fligths that belong to this plane
	QList <Flight *> interesting_flights;
	// Iterate over all of the flights in the list.
	foreach (Flight *select_flug, flights)
	{
		// Select only flights with matching plane and date.
		if (select_flug->flugzeug==fz.id && select_flug->effdatum ()==date)
		{
			interesting_flights.append (select_flug);
		}
	}
	qSort (interesting_flights);

	// Iterate over the interesting flights, grouping those who can form a
	// collective entry, and add them to the list.
	QList<Flight *> entry_flights;
	Flight *prev=NULL;
	QListIterator<Flight *> flight (interesting_flights);
	while (flight.hasNext ())
	{
		Flight *f=flight.next ();
		// If this entry cannot be merged with the previous, we have to make an
		// entry from the list and clear the list before continuing.
		if (prev && !f->collective_bb_entry_possible (prev, fz))
		{
			addPlaneLogEntry (bb, db, entry_flights, fz, date);
			entry_flights.clear ();
		}

		entry_flights.append (f);
		prev=f;
	}
	addPlaneLogEntry (bb, db, entry_flights, fz, date);
}

void makePlaneLogDay (QList<PlaneLogEntry *> &bb, Database *db, QDate date, QList<Plane *> planes, QList<Flight *> flights, QString *club)
	// If club is specified, only planes of this club are used.
{
	foreach (Plane *plane, planes)
	{
		// TODO emit progress
		if (club==NULL || simplify_club_name (plane->club)==simplify_club_name (*club))
		{
			makePlaneLogPlane (bb, db, date, *plane, flights);
		}
	}
}

void makePlaneLogDay (QList<PlaneLogEntry *> &planeLog, Database *db, QDate date)
{
	// TODO error handling

	QList<Plane *> planes;
	// Find out which planes had flights today
	db->list_planes_date (planes, &date);

	QList<Flight *> flights;
	// We need all flights of that date anyway. For speed, we don't query the
	// database for each plane but retrieve all flights here.
	db->list_flights_date (flights, &date);


	db_id self_start_id=db->get_startart_id_by_type (sat_self);

	// Walk through all flights. If the startart is an airtow, add the tow
	// plane id to the towplane id list if it is not already present and add
	// the tow flight to the flight list.
	// TODO in Funktion schieben, evtl. in Database
	QList<db_id> towplane_ids;
	foreach (Flight *flight, flights)
	{
		// Determine the startart
		db_id sa_id=flight->startart;
		LaunchType sa;
		bool sa_ok=(db->get_startart (&sa, sa_id)==db_ok);

		if (sa_ok && sa.is_airtow ())
		{
			// Take the ID of the tow plane from the startart, if known, or
			// from the flight else.
			db_id towplane_id;
			if (sa.towplane_known ())
			{
				Plane towplane;
				// TODO error checking
				db->get_plane_registration (&towplane, sa.get_towplane ());
				towplane_id=towplane.id;
			}
			else
			{
				towplane_id=flight->towplane;
			}

			// Only continue if towplane_id denotes a valid plane.
			if (!id_invalid (towplane_id))
			{
				// If this towplane is not already in the list, add it
				if (towplane_ids.find (towplane_id)==towplane_ids.end ())
					towplane_ids.append (towplane_id);

				// Create a new flight, set it to the tow flight and add it to the
				// flight list.
				Flight *towflight=new Flight;
				flight->get_towflight (towflight, towplane_id, self_start_id);

				// Do some very special processing
				if (towflight->modus==fmLeaving) towflight->landezeit.set_null ();
				flights.append (towflight);
			}
		}
	}

	// For all entries in the towplane ID list...
	for (QList<db_id>::iterator plane_id=towplane_ids.begin (); plane_id!=towplane_ids.end (); ++plane_id)
	{
		// Get the plane from the database
		Plane *towplane=new Plane;
		bool plane_ok=(db->get_plane (towplane, *plane_id)==db_ok);

		if (plane_ok)
		{
			// If that plane exists, add it to the list, if it is not already
			// there.
			bool towplane_already_present=false;
			foreach (Plane *search_pl, planes)
			{
				if (search_pl->registration==towplane->registration)
				{
					towplane_already_present=true;
					break;
				}
			}
			if (!towplane_already_present) planes.append (towplane);
		}
		else
			// If not, free the memory now.
			delete towplane;
	}

	makePlaneLogDay (planeLog, db, date, planes, flights);

	foreach (Plane *p, planes) delete p;
	foreach (Flight *f, flights) delete f;
}



