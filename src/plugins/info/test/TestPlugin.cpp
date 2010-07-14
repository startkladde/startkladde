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
#include "src/util/qString.h"

REGISTER_INFO_PLUGIN (TestPlugin)

TestPlugin::TestPlugin (QString caption, bool enabled, const QString &greetingName, bool richText):
	InfoPlugin (caption, enabled),
	greetingName (greetingName), richText (richText)
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
	return utf8 ("Gibt eine Begrüßung und die aktuelle Uhrzeit aus");
}

PluginSettingsPane *TestPlugin::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new TestPluginSettingsPane (this, parent);
}

void TestPlugin::infoPluginReadSettings (const QSettings &settings)
{
	greetingName=settings.value ("greetingName", greetingName).toString ();
	richText    =settings.value ("richText"    , richText    ).toBool   ();
}

void TestPlugin::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue ("greetingName", greetingName);
	settings.setValue ("richText"    , richText    );
}

void TestPlugin::start ()
{
	trigger ();
}

void TestPlugin::terminate ()
{

}

void TestPlugin::minuteChanged ()
{
	trigger ();
}

void TestPlugin::trigger ()
{
	// Construct the text parts
	QString helloText="Hallo";

	QString greetingText;
	if (isBlank (greetingName))
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

QString TestPlugin::configText () const
{
	return utf8 ("„%1“, %2 text").arg (greetingName, richText?"rich":"plain");
}
