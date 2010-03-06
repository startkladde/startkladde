#include "qString.h"

/**
 * A shortcut for QString::fromUtf8
 *
 * We are currently not using tr due to unclear performance impact (this
 * program is supposed to run smoothly even on slow (533 MHz) hardware).
 */
QString utf8 (const char *text)
{
	return QString::fromUtf8 (text);
}

std::ostream &operator<< (std::ostream &s, const QString &c)
{
	return s << qPrintable (c);
}

/** Converts a std::string to a QString */
QString std2q (const std::string &s)
{
	return QString (s.c_str ());
}

/** Converts a QString to a std::string */
std::string q2std (const QString &s)
{
	if (s.isNull ()) return "";
	return std::string (s.utf8 ().constData ());
}
