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
		virtual void terminate ();

		virtual QString getId          () const;
		virtual QString getName        () const;
		virtual QString getDescription () const;

		virtual QWidget *createSettingsPane ();

		virtual void loadSettings (const QSettings &settings);
		virtual void saveSettings (QSettings &settings);
};

#endif
