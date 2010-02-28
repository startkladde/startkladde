#include "Monitor.h"

namespace Db { namespace Event
{
	Monitor::Monitor (QObject &source, const char *signal, Monitor::Listener &listener):
		listener (listener)
	{
		connect (&source, signal, this, SLOT (dbEvent (Db::Event::Event)));
	}

	Monitor::~Monitor ()
	{
	}

	void Monitor::dbEvent (Db::Event::Event event)
	{
		listener.dbEvent (event);
	}
} }
