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
		void unload ();
		void loadForLocale (const QString &localeName);
		void loadForCurrentLocale ();
		void load (const LanguageConfiguration &configuration, bool force=false);
		void toggleLanguage ();

		// Current configuration
		LanguageConfiguration getCurrentConfiguration () { return currentConfiguration; }

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

		// Current configuration
		LanguageConfiguration currentConfiguration;
};

#endif
