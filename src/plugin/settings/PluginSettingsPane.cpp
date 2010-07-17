/*
 * PluginSettingsPane.cpp
 *
 *  Created on: 05.07.2010
 *      Author: Martin Herrmann
 */

#include "PluginSettingsPane.h"

#include "src/gui/windows/SettingsWindow.h"
#include "src/config/Settings.h"

#include <iostream> // remove

PluginSettingsPane::PluginSettingsPane (QWidget *parent):
	QWidget (parent),
	settingsWindow (NULL)
{
}

PluginSettingsPane::~PluginSettingsPane ()
{
}

QStringList PluginSettingsPane::getEffectivePluginPaths ()
{
	if (settingsWindow)
	{
		std::cout << "using settings window" << std::endl;
		return settingsWindow->getPluginPaths ();
	}
	else
	{
		std::cout << "using settings instance" << std::endl;
		return Settings::instance ().pluginPaths;
	}
}
