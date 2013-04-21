#include "src/flarm/Flarm.h"

#include "src/config/Settings.h"
#include "src/flarm/FlarmList.h"
#include "src/io/dataStream/TcpDataStream.h"
#include "src/io/dataStream/SerialDataStream.h"
#include "src/nmea/NmeaDecoder.h"
#include "src/nmea/GpsTracker.h"

// Data flow:
//   Data stream --> NMEA decoder --.--> GPS tracker
//                                  '--> Flarm list

Flarm::Flarm (QObject *parent, DbManager &dbManager): QObject (parent),
	_dbManager (dbManager),
	_p_dataStream (NULL), _open (false)
{
	// Create the NMEA decoder
	// The data stream will be created and connected to the NMEA decoder later,
	// because the stream may change at runtime, depending on the configuration.
	_nmeaDecoder=new NmeaDecoder (this);

	// Create the GPS tracker and connect it to the NMEA decoder
	_gpsTracker=new GpsTracker (this);
	_gpsTracker->setNmeaDecoder (_nmeaDecoder);

	// Create the Flarm list and connect it to the NMEA decoder
	_flarmList=new FlarmList (this);
	_flarmList->setNmeaDecoder (_nmeaDecoder);
	_flarmList->setDatabase (&dbManager);

	// When the settings change, we may have to react
	connect (&Settings::instance (), SIGNAL (changed ()), this, SLOT (settingsChanged ()));
}

Flarm::~Flarm ()
{
	// _dataStream, _nmeaDecoder, _gpsTracker and _flarmList will be removed
	// automatically by Qt.
}

// ****************
// ** Connection **
// ****************

void Flarm::open  () { setOpen (true ); }
void Flarm::close () { setOpen (false); }

void Flarm::setOpen (bool o)
{
	_open=o;
	updateOpen ();
}

Flarm::ConnectionState Flarm::connectionState ()
{
	ConnectionState state;

	if (dataStream ())
	{
		state.enabled=true;
		state.dataStreamState=dataStream ()->state ();
	}
	else
	{
		state.enabled=false;
	}

	return state;
}

void Flarm::setDataStream (DataStream *dataStream)
{
	// Nothing to do if the data stream is already current (probably NULL).
	if (_p_dataStream==dataStream)
		return;

	// Delete the old data stream, if any, and replace it with the new one.
	delete _p_dataStream;
	_p_dataStream=dataStream;

	// If there is a data stream, connect its signals
	if (_p_dataStream)
	{
		connect (
			_p_dataStream , SIGNAL (lineReceived (const QString &)),
			_nmeaDecoder  , SLOT   (lineReceived (const QString &)));

		connect (
			_p_dataStream, SIGNAL (stateChanged            (DataStream::State)),
			this         , SLOT   (dataStream_stateChanged (DataStream::State)));
	}

	// The connection, and therefore, the connection state, changed
	emit connectionStateChanged (connectionState ());
}

template<class T> T *Flarm::ensureTypedDataStream ()
{
	// Try to cast the existing data stream (if any) to the requested type.
	T *typedDataStream=dynamic_cast<T *> (dataStream ());

	// If the cast returned NULL, there is either no data stream, or it has the
	// wrong type. In this case, create
	if (!typedDataStream)
	{
		// Either there is no data stream, or it has the wrong type. Create a
		// new one data stream with the correct type and store it.
		typedDataStream=new T (this);
		setDataStream (typedDataStream);
	}

	// Return the pre-existing or newly created data stream
	return typedDataStream;
}

void Flarm::updateOpen ()
{
	// Make sure that the data stream has the correct type or is NULL, depending
	// on the configuration.
	Settings &s=Settings::instance ();
	if (_open && s.flarmEnabled)
	{
		// Flarm stream
		switch (s.flarmConnectionType)
		{
			case Flarm::noConnection:
			{
				setDataStream (NULL);
			} break;
			case Flarm::serialConnection:
			{
				SerialDataStream *stream=ensureTypedDataStream<SerialDataStream> ();
				stream->setPort (s.flarmSerialPort, s.flarmSerialBaudRate);
				stream->open ();
			} break;
			case Flarm::tcpConnection:
			{
				TcpDataStream *stream=ensureTypedDataStream<TcpDataStream> ();
				stream->setTarget (s.flarmTcpHost, s.flarmTcpPort);
				stream->open ();
			} break;
			// no default
		}
	}
	else
	{
		setDataStream (NULL);
	}
}


// **************
// ** Settings **
// **************

void Flarm::settingsChanged ()
{
	// The "Flarm enabled" setting may have changed, so we may have to open or
	// close the connection. Same for the connection type and parameters.
	updateOpen ();
}


// *****************
// ** Data stream **
// *****************

bool Flarm::isDataValid ()
{
	if (dataStream ())
		return dataStream ()->state ().isDataOk ();
	else
		return false;
}

void Flarm::dataStream_stateChanged (DataStream::State state)
{
	(void)state;
	emit connectionStateChanged (connectionState ());
}


// ************************
// ** GPS tracker facade **
// ************************

GeoPosition Flarm::getPosition () const { return _gpsTracker->getPosition (); }
QDateTime   Flarm::getGpsTime  () const { return _gpsTracker->getGpsTime  (); }


// *********************
// ** Connection type **
// *********************

QString Flarm::ConnectionType_toString (ConnectionType type)
{
	switch (type)
	{
		case noConnection     : return "none";
		case serialConnection : return "serial";
		case tcpConnection    : return "tcp";
		// no default
	}

	return "unknown";
}

Flarm::ConnectionType Flarm::ConnectionType_fromString (const QString &string, ConnectionType defaultValue)
{
	if      (string=="none"  ) return noConnection;
	else if (string=="serial") return serialConnection;
	else if (string=="tcp"   ) return tcpConnection;
	else                       return defaultValue;
}

QString Flarm::ConnectionType_text (ConnectionType type)
{
	switch (type)
	{
		case noConnection     : return tr ("None");
		case serialConnection : return tr ("Serial/USB/Bluetooth");
		case tcpConnection    : return tr ("Network (TCP)");
		// no default
	}

	return "unknown";
}

QList<Flarm::ConnectionType> Flarm::ConnectionType_list ()
{
	QList<ConnectionType> list;

	list << noConnection;
	list << serialConnection;
	list << tcpConnection;

	return list;
}

