#ifndef QDATE_H_
#define QDATE_H_

#include <QString>

#include "src/notr.h"

class QDate;

QDate validDate (const QDate &date, const QDate &invalidOption);
QString dateRangeToString (const QDate &first, const QDate &last, const QString &separator=notr (" - "), Qt::DateFormat format=Qt::TextDate);
QDate firstOfYear (int year);
QDate firstOfYear (const QDate &date);
std::ostream &operator<< (std::ostream &s, const QDate &date);

#endif
