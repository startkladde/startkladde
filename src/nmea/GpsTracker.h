#ifndef GPSTRACKER_H_
#define GPSTRACKER_H_

#include <QObject>
#include <QDateTime>

#include "src/numeric/GeoPosition.h"

class NmeaDecoder;
class GprmcSentence;

/**
 * Tracks GPS state such as the own position
 */
class GpsTracker: public QObject
{
		Q_OBJECT

	public:
		GpsTracker (QObject *parent);
		virtual ~GpsTracker ();

		void setNmeaDecoder (NmeaDecoder *nmeaDecoder);

		GeoPosition getPosition () const;
		QDateTime getGpsTime () const;

	signals:
		/** Emitted whenever a new position record is received */
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
