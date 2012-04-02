#include "TranslationManager.h"

#include <iostream>

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QRegExp>
#include <QStringList>
#include <QDir>

#include "src/util/qString.h"

// TODO: when switching from "automatic" to, e. g., "German" in a German
// environment, the translation is still reloaded, because the old locale is
// "de_DE" and the new one is "de". This could, for example, be solved by
// storing the name of the translation file instead of the locale name (or could
// it?).


// ****************************
// ** Construction/singleton **
// ****************************

TranslationManager *TranslationManager::theInstance;

TranslationManager::TranslationManager ()
{
	translationsPath=QCoreApplication::applicationDirPath ()+"/translations";

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
	QList<Language> languages;
	QList<Language> languagesWithoutName;

	QStringList nameFilters=QStringList () << "*.qm";

	QStringList fileList=QDir (translationsPath).entryList (nameFilters, QDir::Files);
	foreach (const QString &filename, fileList)
	{
		QString localeName=localeNameFromFilename (filename);

		if (!localeName.isEmpty ())
		{
			Language language;
			language.localeName=localeName;

			// If the language name is empty, use the locale name instead. This
			// may happen if there is a language which does not translate its own
			// name.
			// TODO this functionality should be in the caller, but callers
			// would forget to handle it. There should be a parameter for what
			// to do in this case: ignore it (return empty name, the caller
			// handles it), ignore the language (don't include it in the list),
			// use locale name.
			language.languageName=determineLanguageNameForLocale (localeName);

			if (language.languageName.isEmpty ())
			{
				language.languageName=language.localeName;
				languagesWithoutName.append (language);
			}
			else
			{
				languages.append (language);
			}
		}
	}

	return languages+languagesWithoutName;
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
 * Unloads the translation, if any is loaded
 *
 * If no translation is loaded, nothing happens, unless force is true.
 */
void TranslationManager::unload (bool force)
{
	if (currentTranslation!="" || force)
	{
		// Output a message
		std::cout << "Unloading translation" << std::endl;

		applicationTranslator.load ("");
		qtTranslator         .load ("");

		currentTranslation="";
	}
}

/**
 * Loads the translation for the given locale name
 *
 * If the translation is already loaded, nothing happens, unless force is true.
 */
void TranslationManager::loadForLocale (const QString &localeName, bool force)
{
	//std::cout << "Current: " << currentTranslation << "; new: " << localeName << std::endl;
	if (currentTranslation!=localeName || force)
	{
		// A message will be output by loadTranslation

		loadTranslation (applicationTranslator, filenameForLocaleName (localeName), translationsPath);
		loadTranslation (qtTranslator,          "qt_"+localeName                  , QLibraryInfo::location (QLibraryInfo::TranslationsPath));

		currentTranslation=localeName;
	}
}

/**
 * Loads the translation for the current locale
 *
 * If the translation is already loaded, nothing happens, unless force is true.
 */
void TranslationManager::loadForCurrentLocale (bool force)
{
	loadForLocale (QLocale::system ().name (), force);
}

/**
 * Loads the translation specified by the given configuration
 *
 * If the translation is already loaded, nothing nothing happens, unless the
 * force parameter is true.
 *
 * @param configuration the language configuration to load
 * @param force load the language even if it is already loaded
 */
// FIXME what happens if it is invalid (e. g. invalid value in configuration)?
void TranslationManager::load (const LanguageConfiguration &configuration, bool force)
{
	switch (configuration.getType ())
	{
		case LanguageConfiguration::manualSelection:
			loadForLocale (configuration.getLocaleName (), force);
			break;
		case LanguageConfiguration::noTranslation :
			unload (force);
			break;
		case LanguageConfiguration::systemLanguage:
			loadForCurrentLocale (force);
			break;
		// No default
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
