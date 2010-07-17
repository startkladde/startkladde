#ifndef IO_H_
#define IO_H_

#include <QString>

class QIODevice;
class QRegExp;

bool findInIoDevice (QIODevice &device, QRegExp &regexp);
QString findInIoDevice (QIODevice &device, const QRegExp &regexp, int group);

#endif
