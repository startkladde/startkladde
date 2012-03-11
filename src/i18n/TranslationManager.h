#ifndef TRANSLATIONMANAGER_H_
#define TRANSLATIONMANAGER_H_

#include <QTranslator>

class QApplication;
class QTranslator;

class TranslationManager
{
	public:
		TranslationManager ();
		virtual ~TranslationManager ();

		void install (QApplication *application);

		void unload ();
		void loadForLocale (const QString &localeName);
		void loadForCurrentLocale ();

		void changeLanguage ();


	private:
		QTranslator applicationTranslator;
		QTranslator qtTranslator;

		bool loadTranslation (QTranslator &translator, const QString &filename, const QString &directory);
};

#endif

