#include "version.h"

#include "build/version.h"
#include "text.h"
#include "i18n/notr.h"

QString getVersion ()
{
	QString version (VERSION);

#ifdef SK_BUILD
	version+=qnotr (" (%1)").arg (STRINGIFY(SK_BUILD));
#endif

	return version;
}
