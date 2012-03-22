#ifndef LANGUAGECONFIGURATION_H_
#define LANGUAGECONFIGURATION_H_

#include <QString>
#include <QVariant>

class LanguageConfiguration
{
	public:
		enum Type { manualLanguage, systemLanguage, noTranslation };

		LanguageConfiguration ();
		LanguageConfiguration (Type type);
		LanguageConfiguration (const QString &localeName);
		LanguageConfiguration (const QVariant &value);
		virtual ~LanguageConfiguration ();


		Type getType () const { return type; }
		QString getLocaleName () const { return localeName; }

	private:
		Type type;
		QString localeName; // Only for manualLanguage
};

#endif
