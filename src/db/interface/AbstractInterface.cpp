#include "AbstractInterface.h"

namespace Db { namespace Interface
{
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

} }
