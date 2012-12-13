#include "src/util/qTime.h"

#include <QTime>

int toSeconds (const QTime &time)
{
	return QTime ().secsTo (time);
}
