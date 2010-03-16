#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <QString>
#include <QStringList>
#include <QList>
#include <qfile.h>

class LaunchMethod;

/**
 * The old settings class, to be replaced by Settings
 */
class Options
{
	public:
		static QString effectiveConfigFileName ();
		static QList<LaunchMethod> readConfiguredLaunchMethods ();

};

#endif
