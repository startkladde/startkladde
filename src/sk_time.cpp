#include "sk_time.h"

sk_time::sk_time ()/*{{{*/
	:QTime ()
	/*
	 * Constructs an empty sk_time instance.
	 */
{
}/*}}}*/

QString sk_time::table_string (const char *format)/*{{{*/
	/*
	 * Makes a text for the table.
	 * Parameters:
	 *   - format: the format of the text.
	 * Return value:
	 *   the text.
	 */
{
//	if (isNull ())
//		return "";
//	else
		return toString (format);
}/*}}}*/

QString sk_time::table_string (bool gelandet)/*{{{*/
	/*
	 * Makes a text for the table depending on a flight status.
	 * Parameters:
	 *   - gelandet: whether the flight has landed.
	 * Return value:
	 *   the text.
	 */
{
	if (gelandet)
		return table_string ("hh:mm");
	else
		return table_string ("(hh:mm)");
}/*}}}*/

sk_time::sk_time (const QTime &t)/*{{{*/
	:QTime (t)
	/*
	 * Constructs an sk_time instance given the time as QTime.
	 * Parameters:
	 *   - t: the time.
	 */
{
}/*}}}*/

sk_time::sk_time (int h, int m, int s, int ms)/*{{{*/
	:QTime (h, m, s, ms)
	/*
	 * Constructs an sk_time instance given the time as hour, minute, second
	 * and millisecond.
	 * Parameters:
	 *   - h, m, s, ms: the hour, minute, second and millisecond.
	 */
{
}
/*}}}*/

