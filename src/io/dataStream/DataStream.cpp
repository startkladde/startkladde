#include "src/io/dataStream/DataStream.h"

DataStream::DataStream ()
{
}

DataStream::~DataStream ()
{
}


// *******************************
// ** DataStream::State methods **
// *******************************

DataStream::State::State ():
	open (false), connectionState (notConnected),
	dataReceived (false), dataTimeout (false)
{
}

bool DataStream::State::setOpen (bool open)
{
	bool result=(open!=this->open);
	this->open=open;
	return result;
}

bool DataStream::State::setConnectionState (DataStream::ConnectionState connectionState)
{
	bool result=(connectionState!=this->connectionState);
	this->connectionState=connectionState;
	return result;
}

bool DataStream::State::setDataReceived (bool dataReceived)
{
	bool result=(dataReceived!=this->dataReceived);
	this->dataReceived=dataReceived;
	return result;
}

bool DataStream::State::setDataTimeout (bool dataTimeout)
{
	bool result=(dataTimeout!=this->dataTimeout);
	this->dataTimeout=dataTimeout;
	return result;
}
