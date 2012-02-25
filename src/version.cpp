#include "version.h"

#include "version/version.h"
#include "text.h"
#include "notr.h"

QString getVersion ()
{
	QString version (VERSION);

#ifdef SK_BUILD
	version+=qnotr (" (%1)").arg (STRINGIFY(SK_BUILD));
#endif

	return version;
}
