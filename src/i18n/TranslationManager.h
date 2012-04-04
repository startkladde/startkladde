#ifndef TRANSLATIONMANAGER_H_
#define TRANSLATIONMANAGER_H_

#include <QTranslator>
#include <QString>

#include "src/i18n/LanguageConfiguration.h"

class QApplication;
class QTranslator;

/**
 * A singleton class that handles translation of the application
 */
class TranslationManager
{
	public:
		// A language description, consisting of a locale name and the name of
		// the language in that language
		class Language
		{
			public:
				QString localeName;
				QString languageName;
		};

		// Construction/singleton
		static TranslationManager &instance ();
		virtual ~TranslationManager ();

		// Setup
		void install (QApplication *application);

		// Language listing
		QList<Language> listLanguages ();

		// Language loading
		bool unload (bool force=false);
		bool loadForLocale (const QString &localeName, bool force=false);
		bool loadForCurrentLocale (bool force=false);
		bool load (const LanguageConfiguration &configuration, bool force=false);
		void toggleLanguage ();

	protected:
		// Settings
		QString filenameForLocaleName (const QString &localeName);
		QString localeNameFromFilename (const QString &filename);

		// Language identification
		QString determineLanguageNameForLocale (const QString &localeName);

	private:
		// Singleton
		TranslationManager ();
		static TranslationManager *theInstance;

		// Translators
		QTranslator applicationTranslator;
		QTranslator qtTranslator;

		// Settings
		QString translationsPath;

		// Language loading
		bool loadTranslation (QTranslator &translator, const QString &filename, const QString &directory);

		// Current translation
		QString currentTranslation;
};

#endif
