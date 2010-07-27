#ifndef SUNSETTIMEPLUGIN_H_
#define SUNSETTIMEPLUGIN_H_

#include "SunsetPluginBase.h"

class SunsetTimePlugin: public SunsetPluginBase
{
	SK_PLUGIN

	public:
		SunsetTimePlugin (QString caption=QString (), bool enabled=true, const QString &filename="sunsets.txt");
		virtual ~SunsetTimePlugin ();

		virtual void start ();

		value_accessor (bool, DisplayUtc, displayUtc);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);


	private:
		bool displayUtc;
};

#endif
