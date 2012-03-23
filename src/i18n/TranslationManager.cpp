#include "TranslationManager.h"

#include <iostream>

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QRegExp>
#include <QStringList>
#include <QDir>

#include "src/util/qString.h"
#include "src/i18n/LanguageConfiguration.h"

TranslationManager *TranslationManager::theInstance;

TranslationManager::TranslationManager ()
{
	translationsPath="translations";

}

TranslationManager::~TranslationManager ()
{
}

TranslationManager &TranslationManager::instance ()
{
	if (!theInstance)
		theInstance=new TranslationManager ();

	return *theInstance;
}

void TranslationManager::changeLanguage ()
{
	if (applicationTranslator.isEmpty ())
		loadForCurrentLocale ();
	else
		unload ();
}

bool TranslationManager::loadTranslation (QTranslator &translator, const QString &filename, const QString &directory)
{
	std::cout << "Loading translation from " << filename << " in " << directory << "...";

	bool result=translator.load (filename, directory);

	if (result)
		std::cout << "success" << std::endl;
	else
		std::cout << "failed" << std::endl;

	return result;
}

void TranslationManager::unload ()
{
	applicationTranslator.load ("");
	qtTranslator         .load ("");
}

void TranslationManager::loadForLocale (const QString &localeName)
{
	loadTranslation (applicationTranslator, filenameForLocaleName (localeName), translationsPath);
	loadTranslation (qtTranslator,          "qt_"+localeName                  , QLibraryInfo::location (QLibraryInfo::TranslationsPath));
}

void TranslationManager::loadForCurrentLocale ()
{
	loadForLocale (QLocale::system ().name ());
}


void TranslationManager::install (QApplication *application)
{
	application->installTranslator (&applicationTranslator);
	application->installTranslator (&qtTranslator);
}

QString TranslationManager::filenameForLocaleName (const QString &localeName)
{
	return "startkladde_"+localeName;
}

QString TranslationManager::localeNameFromFilename (const QString &filename)
{
	QRegExp regexp ("startkladde_(.*)\\.qm");
	if (regexp.exactMatch (filename))
		return regexp.cap (1);
	else
		return QString ();
}



QList<TranslationManager::Language> TranslationManager::listLanguages ()
{
	QList<Language> result;

	QStringList nameFilters=QStringList () << "*.qm";

	QStringList fileList=QDir (translationsPath).entryList (nameFilters, QDir::Files);
	foreach (const QString &filename, fileList)
	{
		QString localeName=localeNameFromFilename (filename);

		if (!localeName.isEmpty ())
		{
			Language language;
			language.localeName=localeName;
			// FIXME handle properly if empty (at least: don't return it)
			language.languageName=determineLanguageNameForLocale (localeName);
			result.append (language);
		}
	}

	return result;
}

QString TranslationManager::determineLanguageNameForLocale (const QString &localeName)
{
	QTranslator translator;
	translator.load (filenameForLocaleName (localeName), translationsPath);

	struct { const char *source; const char *comment; } languageString=QT_TRANSLATE_NOOP3("Translation", "Default (English)", "Replace with the name of the translation language, in that language");

	return translator.translate ("Translation", languageString.source);
}

void TranslationManager::loadForConfiguration (const LanguageConfiguration &configuration)
{
	switch (configuration.getType ())
	{
		case LanguageConfiguration::manualSelection:
			loadForLocale (configuration.getLocaleName ());
			break;
		case LanguageConfiguration::noTranslation :
			unload ();
			break;
		case LanguageConfiguration::systemLanguage:
			loadForCurrentLocale ();
			break;
		// No default
	}
}
