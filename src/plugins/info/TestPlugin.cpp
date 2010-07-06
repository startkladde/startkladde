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
	greetingName ("TestPlugin"), richText (false)
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

PluginSettingsPane *TestPlugin::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new TestPluginSettingsPane (this, parent);
}

void TestPlugin::readSettings (const QSettings &settings)
{
	InfoPlugin::readSettings (settings);
	greetingName=settings.value ("greetingName", greetingName).toString ();
	richText    =settings.value ("richText"    , richText    ).toBool   ();
}

void TestPlugin::writeSettings (QSettings &settings)
{
	InfoPlugin::writeSettings (settings);
	settings.setValue ("greetingName", greetingName);
	settings.setValue ("richText"    , richText    );
}

void TestPlugin::start ()
{
	// Construct the text parts
	QString helloText="Hallo";

	QString greetingText;
	if (blank (greetingName))
		greetingText="";
	else
		greetingText=QString (" %1").arg (greetingName);

	QString timeText=QString ("um %1").arg (QTime::currentTime ().toString ());

	// Add color if rich text is set
	if (richText)
	{
		   helloText=QString ("<font color=\"#FF3F00\">%1</font>").arg (   helloText);
		greetingText=QString ("<font color=\"#3F7F00\">%1</font>").arg (greetingText);
		    timeText=QString ("<font color=\"#003FFF\">%1</font>").arg (    timeText);
	}

	// Construct the final text
	QString text=QString ("%1%2 %3").arg (helloText, greetingText, timeText);

	// Output the text
	outputText (text, richText?Qt::RichText:Qt::PlainText);
}

void TestPlugin::terminate ()
{

}
