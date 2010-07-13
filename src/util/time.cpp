#include "time.h"

#include <cstdlib>

#include "src/Longitude.h"

QTime nullSeconds (const QTime &time)
{
	if (!time.isValid ())
		return QTime ();

	return QTime (time.hour (), time.minute (), 0);
}

QDateTime nullSeconds (const QDateTime &dateTime)
{
	if (!dateTime.isValid ())
		return QDateTime ();

	return QDateTime (
		dateTime.date (),
		QTime (dateTime.time ().hour (), dateTime.time ().minute (), 0),
		dateTime.timeSpec ());
}

QTime currentTimeUtc ()
{
	return QDateTime::currentDateTime ().toUTC ().time ();
}

QString formatDuration (int seconds, bool includeSeconds)
{
	bool negative=(seconds<0);
	seconds=std::abs (seconds);

	uint minutes=seconds/60;
	seconds=seconds%60;

	uint hours=minutes/60;
	minutes=minutes%60;

	QString sign=negative?"-":"";

	if (includeSeconds)
		return QString ("%1%2:%3:%4")
			.arg (sign)
			.arg (hours)
			.arg (minutes, 2, 10, QChar ('0'))
			.arg (seconds, 2, 10, QChar ('0'));
	else
		return QString ("%1%2:%3")
			.arg (sign)
			.arg (hours)
			.arg (minutes, 2, 10, QChar ('0'));
}

QTime localSunset (const Longitude &longitude, const Longitude &referenceLongitude, const QTime &sunsetAtReference)
{
	double dLon=longitude.minusDegrees (referenceLongitude);

	// 360° -- 1 day
	//   1° -- 4 min = 240 sec
	int dT=240*dLon;

	// Bigger (easterner) longitude means earlier sunset
	return sunsetAtReference.addSecs (-dT);
}

QTime utcToLocal (const QTime &time)
{
	// TODO: currentDate probably returns local date
	QDateTime dateTime (QDate::currentDate (), time, Qt::UTC);
	return dateTime.toLocalTime ().time ();
}

// This function can be used, it's just not tested yet
//QTime localToUtc (const QTime &time)
//{
//	// TODO: currentDate probably returns local date
//	QDateTime dateTime (QDate::currentDate (), time, Qt::LocalTime);
//	return dateTime.toUTC ().time ();
//}
