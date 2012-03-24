#include "TranslationManager.h"

#include <iostream>

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QRegExp>
#include <QStringList>
#include <QDir>

#include "src/util/qString.h"


// ****************************
// ** Construction/singleton **
// ****************************

TranslationManager *TranslationManager::theInstance;

TranslationManager::TranslationManager ():
	currentConfiguration (LanguageConfiguration (LanguageConfiguration::noTranslation))
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


// ***********
// ** Setup **
// ***********

void TranslationManager::install (QApplication *application)
{
	application->installTranslator (&applicationTranslator);
	application->installTranslator (&qtTranslator);
}


// *************************************
// ** Language listing/identification **
// *************************************

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


// **********************
// ** Language loading **
// **********************

/**
 * Unloads the translation and updates the current configuration to "no
 * translation"
 */
void TranslationManager::unload ()
{
	std::cout << "Unloading translation" << std::endl;
	applicationTranslator.load ("");
	qtTranslator         .load ("");
	currentConfiguration=LanguageConfiguration (LanguageConfiguration::noTranslation);
}

/**
 * Loads the translation for the given locale and updates the current
 * configuration to "manual setting"
 *
 * @param localeName the name of the locale to load, typically something like
 *                   "de".
 */
void TranslationManager::loadForLocale (const QString &localeName)
{
	loadTranslation (applicationTranslator, filenameForLocaleName (localeName), translationsPath);
	loadTranslation (qtTranslator,          "qt_"+localeName                  , QLibraryInfo::location (QLibraryInfo::TranslationsPath));
	currentConfiguration=LanguageConfiguration (localeName);
}

/**
 * Loads the translation for the current locale and updates the current
 * configuration to "system language"
 */
void TranslationManager::loadForCurrentLocale ()
{
	// Note that loadForLocale sets the current configuration to "manual
	// setting", so we have to change it to "system language" afterwards.
	loadForLocale (QLocale::system ().name ());
	currentConfiguration=LanguageConfiguration (LanguageConfiguration::systemLanguage);
}

/**
 * Loads the translation specified by the given configuration and updates the
 * current configuration
 *
 * If the specified configuration is equal to the current configuration, nothing
 * is loaded, unless the force parameter is true.
 *
 * @param configuration the configuration to load
 * @param force load the configuration even if it is equal to the current
 *              configuration
 */
// FIXME what happens if it is invalid?
void TranslationManager::load (const LanguageConfiguration &configuration, bool force)
{
	std::cout << "Current configuration: " << currentConfiguration.toString () << "; new configuration: " << configuration.toString () << std::endl;
	if (configuration!=currentConfiguration || force)
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
}

void TranslationManager::toggleLanguage ()
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


// **************
// ** Settings **
// **************

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
