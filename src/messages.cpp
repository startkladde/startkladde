#include "messages.h"

char datetime_buf[20];

char *datetime ()/*{{{*/
	/*
	 * Makes a string containing the current date and time.
	 * Return value:
	 *   The string. This string is statically allocated.
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


void log_message (const char *message)/*{{{*/
	/*
	 * Writes a message, prefixed with date and time, to the log, whatever the
	 * log is.
	 * At the moment, it's stdout.
	 * Parameters:
	 *   - message: The message to write.
	 */
{
	printf (c_message "%s: %s\n" c_default, datetime (), message);
}/*}}}*/

void log_error (const char *message)/*{{{*/
	/*
	 * Writes an error message, prefixed with date and time, to the log,
	 * whatever the log is.
	 * At the moment, it's stdout.
	 * Parameters:
	 *   - message: The message to write.
	 */
{
	printf (c_error "%s: Error: %s\n" c_default, datetime (), message);
}/*}}}*/

void log_error (string message)/*{{{*/
{
	log_error (message.c_str ());
}/*}}}*/

