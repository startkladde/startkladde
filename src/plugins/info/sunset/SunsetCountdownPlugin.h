#ifndef SUNSETCOUNTDOWNPLUGIN_H_
#define SUNSETCOUNTDOWNPLUGIN_H_

#include "SunsetPluginBase.h"

class SunsetCountdownPlugin: public SunsetPluginBase
{
	public:
		SunsetCountdownPlugin (QString caption=QString (), bool enabled=true, const QString &filename="sunsets.txt");
		virtual ~SunsetCountdownPlugin ();

		virtual QString getId          () const;
		virtual QString getName        () const;
		virtual QString getDescription () const;

		virtual void start ();

		virtual void minuteChanged ();

	private:
		virtual void update ();
};

#endif
