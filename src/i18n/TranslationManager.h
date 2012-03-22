#ifndef TRANSLATIONMANAGER_H_
#define TRANSLATIONMANAGER_H_

#include <QTranslator>
#include <QString>

class QApplication;
class QTranslator;

class LanguageConfiguration;

class TranslationManager
{
	public:
		class Language
		{
			public:
				QString localeName;
				QString languageName;
		};

		static TranslationManager &instance ();

		virtual ~TranslationManager ();

		void install (QApplication *application);

		void unload ();
		void loadForLocale (const QString &localeName);
		void loadForCurrentLocale ();
		void loadForConfiguration (const LanguageConfiguration &configuration);

		void changeLanguage ();

		QList<Language> listLanguages ();

	protected:
		QString filenameForLocaleName (const QString &localeName);
		QString localeNameFromFilename (const QString &filename);

		QString determineLanguageNameForLocale (const QString &localeName);

	private:
		TranslationManager ();
		static TranslationManager *theInstance;

		QTranslator applicationTranslator;
		QTranslator qtTranslator;

		QString translationsPath;

		bool loadTranslation (QTranslator &translator, const QString &filename, const QString &directory);
};

#endif

