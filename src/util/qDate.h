#ifndef QDATE_H_
#define QDATE_H_

#include <QString>

#include "src/notr.h"

class QDate;

QDate validDate (const QDate &date, const QDate &invalidOption);
// FIXME inconsistent parameter order
QString dateRangeToString (const QDate &first, const QDate &last, const QString &separator=notr (" - "), Qt::DateFormat format=Qt::TextDate);
QString dateRangeToString (const QDate &first, const QDate &last, const QString &format, const QString &separator=notr (" - "));
QDate firstOfYear (int year);
QDate firstOfYear (const QDate &date);
std::ostream &operator<< (std::ostream &s, const QDate &date);

QString defaultNumericDateFormat ();
QString defaultPaddedNumericDateFormat ();

QString defaultNumericDateTimeFormat ();
QString defaultPaddedNumericDateTimeFormat ();

#endif
