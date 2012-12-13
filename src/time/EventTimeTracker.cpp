#include "src/time/EventTimeTracker.h"

#include <QDateTime>
#include <QTime>

#include "src/util/qTime.h"

// TODO we should clean this hash up from time to time (not that it's a big
// deal).

EventTimeTracker::EventTimeTracker ()
{
}

EventTimeTracker::~EventTimeTracker ()
{
}

void EventTimeTracker::eventNow (dbId id)
{
	lastEvent.insert (id, QDateTime::currentDateTimeUtc ());
}

bool EventTimeTracker::eventWithin (dbId id, const QTime &span)
{
	if (!lastEvent.contains (id))
		return false;

	QDateTime lastTime=lastEvent[id];
	QDateTime currentTime=QDateTime::currentDateTimeUtc ();
	int secondsSinceLast=lastTime.secsTo (currentTime);
	return secondsSinceLast<=toSeconds (span);
}
