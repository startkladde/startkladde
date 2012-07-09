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

