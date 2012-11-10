#ifndef FLIGHTREFERENCE_H_
#define FLIGHTREFERENCE_H_

#include "src/db/dbId.h"

class Flight;

class FlightReference
{
	public:
		FlightReference ();
		FlightReference (dbId id, bool towflight);
		FlightReference (const Flight &flight);
		virtual ~FlightReference ();

		static FlightReference flight (dbId id);
		static FlightReference towflight (dbId id);

		static const FlightReference invalid;

		dbId id () const;
		bool towflight () const;

		bool isValid () const;

	private:
		dbId _id;
		bool _towflight;
};

#endif
