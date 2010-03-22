#include "time.h"

QTime nullSeconds (const QTime &time)
{
	return QTime (time.hour (), time.minute (), 0);
}

QDateTime nullSeconds (const QDateTime &time)
{
	return QDateTime (time.date (),
		QTime (time.time ().hour (), time.time ().minute (), 0),
		time.timeSpec ());
}
