#include "messages.h"

#include "src/text.h"

char datetime_buf[20];

char *datetime ()/*{{{*/
	/*
	 * Makes a QString containing the current date and time.
	 * Return value:
	 *   The QString. This QString is statically allocated.
	 */
{
	time_t curtime;
	struct tm *loctime;

	curtime = time (NULL);
	loctime = localtime (&curtime);
	const char *format="%F %T";	// Pfui, aber sonst meckert -Wall
	strftime (datetime_buf, 20, format, loctime);

	return datetime_buf;
}/*}}}*/


void log_message (QString message)/*{{{*/
	/*
	 * Writes a message, prefixed with date and time, to the log, whatever the
	 * log is.
	 * At the moment, it's stdout.
	 * Parameters:
	 *   - message: The message to write.
	 */
{
	printf (c_message "%s: %s\n" c_default, datetime (), message.latin1());
}/*}}}*/

void log_error (QString message)/*{{{*/
	/*
	 * Writes an error message, prefixed with date and time, to the log,
	 * whatever the log is.
	 * At the moment, it's stdout.
	 * Parameters:
	 *   - message: The message to write.
	 */
{
	printf (c_error "%s: Error: %s\n" c_default, datetime (), message.latin1());
}/*}}}*/
