#include "LanguageConfiguration.h"

#include <iostream>

#include <QSettings>

#include "src/util/qString.h"
#include "src/i18n/notr.h"

LanguageConfiguration::LanguageConfiguration ():
	type (systemLanguage)
{
}

LanguageConfiguration::LanguageConfiguration (Type type):
	type (type)
{
}

LanguageConfiguration::LanguageConfiguration (const QString &localeName):
	type (manualSelection), localeName (localeName)
{
}

LanguageConfiguration::LanguageConfiguration (const QVariant &value)
{
	if ((QMetaType::Type)value.type ()==QMetaType::QString)
	{
		this->type=manualSelection;
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

LanguageConfiguration::~LanguageConfiguration ()
{
}

void LanguageConfiguration::load (QSettings &settings)
{
	QString typeString=settings.value ("type", "systemLanguage").toString ();
	if (typeString==notr ("manualSelection"))
	{
		type=manualSelection;
		localeName=settings.value (notr ("localeName")).toString ();
	}
	else if (typeString==notr ("systemLanguage"))
	{
		type=systemLanguage;
		localeName="";
	}
	else if (typeString==notr ("noTranslation"))
	{
		type=noTranslation;
		localeName="";
	}
	else
	{
		std::cerr << notr ("Invalid language configuration type ") << typeString << notr (" in configuration") << std::endl;
		type=systemLanguage;
		localeName="";
	}
}

void LanguageConfiguration::save (QSettings &settings)
{
	enum Type { manualLanguage, systemLanguage, noTranslation };
	switch (type)
	{
		case manualLanguage:
			settings.setValue (notr ("type"), notr ("manualSelection"));
			settings.setValue (notr ("localeName"), localeName);
			break;
		case systemLanguage:
			settings.setValue (notr ("type"), notr ("systemLanguage"));
			settings.setValue (notr ("localeName"), "");
			break;
		case noTranslation:
			settings.setValue (notr ("type"), notr ("noTranslation"));
			settings.setValue (notr ("localeName"), "");
			break;
	}
}
