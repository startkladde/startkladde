/*
 * TestPlugin.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "TestPlugin.h"

#include <QDebug>
#include <QTime>

#include "src/plugin/info/InfoPluginFactory.h"

REGISTER_INFO_PLUGIN (TestPlugin)

TestPlugin::TestPlugin ()
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

QWidget *TestPlugin::createSettingsPane ()
{
	return NULL;
}

void TestPlugin::loadSettings (const QSettings &settings)
{
	InfoPlugin::loadSettings (settings);
}

void TestPlugin::saveSettings (QSettings &settings)
{
	InfoPlugin::saveSettings (settings);
}

void TestPlugin::start ()
{
	outputText (QString ("Hello TestPlugin at %1!").arg (QTime::currentTime ().toString ()));
}

void TestPlugin::terminate ()
{

}
