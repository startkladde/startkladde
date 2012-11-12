#include "src/FlightReference.h"

#include "src/model/Flight.h"

const FlightReference FlightReference::invalid;

FlightReference::FlightReference ():
	_id (invalidId), _towflight (false)
{
}

FlightReference::FlightReference (dbId id, bool towflight):
	_id (id), _towflight (towflight)
{
}

FlightReference::FlightReference (const Flight &flight):
	_id (flight.getId ()), _towflight (flight.isTowflight ())
{
}

FlightReference FlightReference::flight (dbId id)
{
	return FlightReference (id, false);
}

FlightReference FlightReference::towflight (dbId id)
{
	return FlightReference (id, true);
}

FlightReference::~FlightReference ()
{
}

dbId FlightReference::id () const
{
	return _id;
}

bool FlightReference::towflight () const
{
	return _towflight;
}

bool FlightReference::isValid () const
{
	return idValid (_id);
}

bool FlightReference::operator== (const FlightReference &other) const
{
	return (other._id==this->_id) && (other._towflight==this->_towflight);
}

uint qHash (const FlightReference &flightReference)
{
	if (flightReference.towflight ())
		return ~flightReference.id ();
	else
		return flightReference.id ();
}

