#ifndef IO_H_
#define IO_H_

#include <QString>

class QIODevice;
class QRegExp;

QString readLineUtf8 (QIODevice &device);
bool findInIoDevice (QIODevice &device, QRegExp &regexp);
QString findInIoDevice (QIODevice &device, const QRegExp &regexp, int group);

#endif
