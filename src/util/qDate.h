#ifndef QDATE_H_
#define QDATE_H_

#include <QString>

#include "src/notr.h"

class QDate;

QDate validDate (const QDate &date, const QDate &invalidOption);
QString toString (const QDate &date, bool zeroPad=false);
QString toString (const QDate &first, const QDate &last, const QString &separator=notr (" - "), bool zeroPad=false);
QDate firstOfYear (int year);
QDate firstOfYear (const QDate &date);
std::ostream &operator<< (std::ostream &s, const QDate &date);

#endif
