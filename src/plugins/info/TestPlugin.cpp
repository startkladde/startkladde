/*
 * TestPlugin.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "TestPlugin.h"

#include <QDebug>

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

QString TestPlugin::getName ()
{
	return "Test";
}

QString TestPlugin::getShortDescription ()
{
	return "Test-Plugin";
}

QString TestPlugin::getLongDescription ()
{
	return "Ein Plugin zum Testen";
}

QWidget *TestPlugin::createSettingsPane ()
{
	return NULL;
}

void TestPlugin::loadSettings (const QSettings &settings)
{
	(void)settings;
}

void TestPlugin::saveSettings (const QSettings &settings)
{
	(void)settings;
}

void TestPlugin::start ()
{
	outputText ("Hello TestPlugin!");
}
