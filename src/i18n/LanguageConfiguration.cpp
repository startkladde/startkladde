#include "LanguageConfiguration.h"

#include <iostream>

LanguageConfiguration::LanguageConfiguration ():
	type (systemLanguage)
{
}

LanguageConfiguration::LanguageConfiguration (Type type):
	type (type)
{
}

LanguageConfiguration::LanguageConfiguration (const QString &localeName):
	type (manualLanguage), localeName (localeName)
{
}

LanguageConfiguration::~LanguageConfiguration ()
{
}

LanguageConfiguration::LanguageConfiguration (const QVariant &value)
{
	if ((QMetaType::Type)value.type ()==QMetaType::QString)
	{
		this->type=manualLanguage;
		this->localeName=value.toString ();
	}
	else if ((QMetaType::Type)value.type ()==QMetaType::Int)
	{
		int type=value.toInt ();
		switch (type)
		{
			case systemLanguage: this->type=systemLanguage; break;
			case noTranslation:  this->type=noTranslation; break;
			default:
				std::cerr << "Invalid language choice in SettingsWindow::getSelectedLanguageConfiguration" << std::endl;
				this->type=systemLanguage;
		}
	}
}

