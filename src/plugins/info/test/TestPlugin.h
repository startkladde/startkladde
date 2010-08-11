/*
 * TestPlugin.h
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#ifndef TESTPLUGIN_H_
#define TESTPLUGIN_H_

#include "src/plugin/info/InfoPlugin.h"

/**
 * A simple info plugin which displays a greeting message, either as plain text
 * or as rich text
 *
 * This plugin serves as a sample implementation of info plugins.
 *
 * Settings:
 *  - greetingName: the name of the entity to greet
 *  - richText: whether to use rich text or plain text
 */
class TestPlugin: public InfoPlugin
{
		SK_PLUGIN

	public:
		TestPlugin (const QString &caption=QString (), bool enabled=true, const QString &greetingName="TestPlugin", bool richText=false);
		virtual ~TestPlugin ();

		virtual void start ();
		virtual void terminate ();

		virtual PluginSettingsPane *infoPluginCreateSettingsPane (QWidget *parent=NULL);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);

		value_accessor (QString, GreetingName, greetingName);
		value_accessor (bool   , RichText    , richText    );

		virtual void minuteChanged ();

		virtual QString configText () const;

	private:
		QString greetingName;
		bool richText;

		void trigger ();
};

#endif