#ifndef LANGUAGECONFIGURATION_H_
#define LANGUAGECONFIGURATION_H_

#include <QString>

class LanguageConfiguration
{
	public:
		enum Type { manualLanguage, systemLanguage, noTranslation };

		LanguageConfiguration ();
		LanguageConfiguration (Type type);
		LanguageConfiguration (const QString &localeName);
		virtual ~LanguageConfiguration ();

		Type getType () { return type; }
		QString getLocaleName () { return localeName; }

	private:
		Type type;
		QString localeName; // Only for manualLanguage
};

#endif
