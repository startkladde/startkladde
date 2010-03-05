#include "DbEventMonitor.h"

namespace Db { namespace Event
{
	DbEventMonitor::DbEventMonitor (QObject &source, const char *signal, DbEventMonitor::Listener &listener):
		listener (listener)
	{
		connect (&source, signal, this, SLOT (dbEvent (Db::Event::DbEvent)));
	}

	DbEventMonitor::~DbEventMonitor ()
	{
	}

	void DbEventMonitor::dbEvent (DbEvent event)
	{
		listener.dbEvent (event);
	}
} }
