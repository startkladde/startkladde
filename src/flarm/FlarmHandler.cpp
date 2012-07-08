#include "FlarmHandler.h"

#include <QtCore/QTime>
#include <QtCore/QMultiMap>
#include <QtCore/QStringList>
#include <QtCore/QPointF>

#include "src/model/objectList/MutableObjectList.h"
#include "src/config/Settings.h"
#include "src/model/Plane.h"
#include "src/io/dataStream/TcpDataStream.h"
#include "src/nmea/Nmea.h"
#include "src/nmea/GprmcSentence.h"
#include "src/nmea/PflaaSentence.h"


// ****************************
// ** Construction/singleton **
// ****************************

FlarmHandler* FlarmHandler::instance = NULL;

FlarmHandler::FlarmHandler (QObject* parent): QObject (parent),
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

void FlarmHandler::setDatabase (DbManager *db)
{
	dbManager = db;
}

QDateTime FlarmHandler::getGpsTime ()
{
	return gpsTime;
}


void FlarmHandler::processPflaaSentence (const PflaaSentence &sentence)
{
	flarmList.processPflaaSentence (sentence);
}

void FlarmHandler::processGprmcSentence (const GprmcSentence &sentence)
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

void FlarmHandler::lineReceived (const QString &line)
{
	//	qDebug () << "Flarm handler: process sentence " << line;

	if (stream) *stream << line;

	// TODO also process GPGGA and GPGSA/GPGSV sentences
	if (line.startsWith ("$PFLAA"))
		processPflaaSentence (PflaaSentence (line));
	else if (line.startsWith ("$GPRMC"))
		processGprmcSentence (GprmcSentence (line));
}
