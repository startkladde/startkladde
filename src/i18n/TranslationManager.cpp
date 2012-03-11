#include "TranslationManager.h"

#include <iostream>

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>

#include "src/util/qString.h"

TranslationManager::TranslationManager ()
{
}

TranslationManager::~TranslationManager ()
{
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
	loadTranslation (applicationTranslator, "startkladde_"+localeName, "translations");
	loadTranslation (qtTranslator,          "qt_"         +localeName, QLibraryInfo::location (QLibraryInfo::TranslationsPath));
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


