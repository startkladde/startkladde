/*
 * TestPlugin.h
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#ifndef TESTPLUGIN_H_
#define TESTPLUGIN_H_

#include "src/plugin/info/InfoPlugin.h"

class TestPlugin: public InfoPlugin
{
	public:
		TestPlugin ();
		virtual ~TestPlugin ();

		virtual void start ();

		static QString getName             ();
		static QString getShortDescription ();
		static QString getLongDescription  ();

		virtual QWidget *createSettingsPane ();

		virtual void loadSettings (const QSettings &settings);
		virtual void saveSettings (const QSettings &settings);
};

#endif
