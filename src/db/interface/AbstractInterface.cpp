#include "AbstractInterface.h"

AbstractInterface::AbstractInterface (const DatabaseInfo &info):
	info (info)
{
}

AbstractInterface::~AbstractInterface ()
{
}

const DatabaseInfo &AbstractInterface::getInfo () const
{
	return info;
}
