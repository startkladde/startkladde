#ifndef EVENTTIMETRACKER_H_
#define EVENTTIMETRACKER_H_

#include <QHash>

#include "src/db/dbId.h"

class QDateTime;
class QTime;

class EventTimeTracker
{
	public:
		EventTimeTracker ();
		virtual ~EventTimeTracker ();

		void eventNow (dbId id);
		bool eventWithin (dbId id, const QTime &span);

	private:
		QHash<dbId, QDateTime> lastEvent;
};

#endif
