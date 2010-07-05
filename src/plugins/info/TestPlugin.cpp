/*
 * TestPlugin.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "TestPlugin.h"

#include <QDebug>
#include <QTime>
#include <QSettings>

#include "src/plugin/info/InfoPluginFactory.h"
#include "TestPluginSettingsPane.h"
#include "src/text.h"

REGISTER_INFO_PLUGIN (TestPlugin)

TestPlugin::TestPlugin ():
	greetingName ("TestPlugin")
{
	qDebug () << "Creating test plugin";
}

TestPlugin::~TestPlugin ()
{
	qDebug () << "Destroying test plugin";
}

QString TestPlugin::getId () const
{
	return "test";
}

QString TestPlugin::getName () const
{
	return "Test";
}

QString TestPlugin::getDescription () const
{
	return "Gibt eine Begrüßung und die Startzeit aus";
}

PluginSettingsPane *TestPlugin::createSettingsPane (QWidget *parent)
{
	return new TestPluginSettingsPane (this, parent);
}

void TestPlugin::readSettings (const QSettings &settings)
{
	InfoPlugin::readSettings (settings);
	greetingName=settings.value ("greetingName").toString ();
}

void TestPlugin::writeSettings (QSettings &settings)
{
	InfoPlugin::writeSettings (settings);
	settings.setValue ("greetingName", greetingName);
}

void TestPlugin::start ()
{
	if (blank (greetingName))
		outputText (QString ("Hallo um %1!").arg (QTime::currentTime ().toString ()));
	else
		outputText (QString ("Hallo %1 um %2!").arg (greetingName, QTime::currentTime ().toString ()));
}

void TestPlugin::terminate ()
{

}
