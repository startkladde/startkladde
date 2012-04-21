#include "version.h"


//#include "build/version.h"
#include "text.h"
#include "i18n/notr.h"
#include "build.h"

QString getVersion ()
{
	// TODO add date, svn revision and build type, e. g.:
	// Version 2.1.0-svn (2012-04-20)
	return qnotr ("Version %1.%2.%3")
		.arg (BUILD_VERSION_MAJOR)
		.arg (BUILD_VERSION_MINOR)
		.arg (BUILD_VERSION_REVISION);
}
