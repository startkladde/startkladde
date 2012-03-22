#include "LanguageConfiguration.h"

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
