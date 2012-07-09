#ifndef GPSTRACKER_H_
#define GPSTRACKER_H_

#include <QObject>
#include <QDateTime>

#include "src/numeric/GeoPosition.h"

class NmeaDecoder;
class GprmcSentence;

class GpsTracker: public QObject
{
		Q_OBJECT

	public:
		GpsTracker ();
		virtual ~GpsTracker ();

		void setNmeaDecoder (NmeaDecoder *nmeaDecoder);

		GeoPosition getPosition ();
		QDateTime getGpsTime ();

	signals:
		void positionChanged (const GeoPosition &position);

	public slots:
		void gprmcSentence (const GprmcSentence &sentence);

	private:
		NmeaDecoder *nmeaDecoder;

		GeoPosition position;
		double altitude;
		QDateTime gpsTime;
};

#endif
