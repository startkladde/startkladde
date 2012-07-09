#include "FlarmHandler.h"

#include <QtCore/QTime>
#include <QtCore/QMultiMap>
#include <QtCore/QStringList>
#include <QtCore/QPointF>

#include "src/model/objectList/MutableObjectList.h"
#include "src/config/Settings.h"
#include "src/model/Plane.h"
#include "src/io/dataStream/TcpDataStream.h"
#include "src/nmea/Nmea.h" // FIXME rmeove?
#include "src/nmea/GprmcSentence.h"
#include "src/nmea/PflaaSentence.h"
#include "src/nmea/NmeaDecoder.h"


// ****************************
// ** Construction/singleton **
// ****************************

FlarmHandler* FlarmHandler::instance = NULL;

FlarmHandler::FlarmHandler (QObject* parent): QObject (parent),
	nmeaDecoder (NULL),
	trace (NULL), stream (NULL)
{
//	QDate today (QDate::currentDate());
//	QString filename = QString ("/var/log/startkladde/startkladde-%1.trc").arg(today.toString("yyyyMMdd"));
//	trace = new QFile (filename, this);
//	trace->open (QIODevice::Append);
//	stream = new QTextStream (trace);
}

FlarmHandler::~FlarmHandler ()
{
	if (trace) trace->close();
	delete trace;
	delete stream;
}

FlarmHandler* FlarmHandler::getInstance ()
{
	if (!instance)
		instance = new FlarmHandler (NULL);
        return instance;
}


// ****************
// ** Properties **
// ****************

void FlarmHandler::setNmeaDecoder (NmeaDecoder *nmeaDecoder)
{
	if (this->nmeaDecoder)
	{
		this->nmeaDecoder->disconnect (this);
	}

	this->nmeaDecoder=nmeaDecoder;

	if (this->nmeaDecoder)
	{
		connect (this->nmeaDecoder, SIGNAL (gprmcSentence (const GprmcSentence &)), this, SLOT (gprmcSentence (const GprmcSentence &)));
		connect (this->nmeaDecoder, SIGNAL (pflaaSentence (const PflaaSentence &)), this, SLOT (pflaaSentence (const PflaaSentence &)));
	}
}

QDateTime FlarmHandler::getGpsTime ()
{
	return gpsTime;
}


void FlarmHandler::pflaaSentence (const PflaaSentence &sentence)
{
	flarmList.processPflaaSentence (sentence);
}

void FlarmHandler::gprmcSentence (const GprmcSentence &sentence)
{
	if (!sentence.isValid)
	{
		qDebug () << QString ("Sentence invalid: ")+sentence.sentence;
		return;
	}

	if (sentence.status)
	{
		// FIXME store, and use GeoPosition
		emit homePosition (sentence.position);
		// FIXME signal
		gpsTime=sentence.timestamp;
	}
}

