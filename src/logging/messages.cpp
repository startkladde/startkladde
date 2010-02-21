#include "messages.h"

#include <iostream>

#include <QDateTime>
#include <QString>

#include "src/io/colors.h"
#include "src/util/qString.h"

QString timestamp ()
{
	return QDateTime::currentDateTime ().toString (Qt::ISODate);
}

void log_message (const QString &message)
	/*
	 * Writes a message, prefixed with date and time, to the log, whatever the
	 * log is.
	 * At the moment, it's stdout.
	 * Parameters:
	 *   - message: The message to write.
	 */
{
	std::cout << QString (c_message "%1: %2" c_default)
		.arg (timestamp (), message) << std::endl;
}

void log_error (const QString &message)
	/*
	 * Writes an error message, prefixed with date and time, to the log,
	 * whatever the log is.
	 * At the moment, it's stderr.
	 * Parameters:
	 *   - message: The message to write.
	 */
{
	std::cerr << QString (c_error "%1: Error: %2" c_default)
		.arg (timestamp (), message) << std::endl;
}
