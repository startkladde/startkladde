#include "AbstractInterface.h"

#include <cassert>
#include <iostream>

#include "src/util/qString.h"

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

void AbstractInterface::setInfo (const DatabaseInfo &info)
{
	this->info=info;
}

QString AbstractInterface::transactionStatementString (TransactionStatement statement)
{
	switch (statement)
	{
		case transactionBegin   : return "Transaction";
		case transactionCommit  : return "Commit";
		case transactionRollback: return "Rollback";
		// no default
	}

	assert (!"Unhandled statement");
	return "?";
}
