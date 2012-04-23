#include "version.h"

#include "text.h"
#include "i18n/notr.h"
#include "build.h"

QString getVersion ()
{
	// TODO add date, svn revision and build type, e. g.:
	// Version 2.1.0-svn (2012-04-20)
	QString version=qnotr ("Version %1.%2.%3")
		.arg (BUILD_VERSION_MAJOR)
		.arg (BUILD_VERSION_MINOR)
		.arg (BUILD_VERSION_REVISION);

	QString buildType (BUILD_TYPE);
	if (!buildType.isEmpty ())
		version+=qnotr (" (build type: %1)").arg (buildType);

	return version;
}
