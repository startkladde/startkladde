#include "TransactionFailedException.h"

#include <cassert>

#include "src/io/AnsiColors.h"

TransactionFailedException::TransactionFailedException (const QSqlError &error,
	AbstractInterface::TransactionStatement statement):
	SqlException (error), statement (statement)
{
}

TransactionFailedException *TransactionFailedException::clone () const
{
	return new TransactionFailedException (error, statement);
}

void TransactionFailedException::rethrow () const
{
	throw TransactionFailedException (error, statement);
}

QString TransactionFailedException::toString () const
{
	return makeString (QString (
		"Transaction failed:\n"
		"    Statement     : %1\n")
		.arg (AbstractInterface::transactionStatementString (statement))
		);
}

QString TransactionFailedException::colorizedString () const
{
	AnsiColors c;

	return makeColorizedString (QString (
		"%1Transaction failed%2:\n"
		"    Statement     : %3")
		.arg (c.red ()).arg (c.reset ())
		.arg (AbstractInterface::transactionStatementString (statement))
		);
}