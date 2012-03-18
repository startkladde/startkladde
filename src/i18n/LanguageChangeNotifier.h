#ifndef LANGUAGECHANGENOTIFIER_H_
#define LANGUAGECHANGENOTIFIER_H_

#include <QWidget>

class LanguageChangeNotifier: public QWidget
{
		Q_OBJECT

	public:
		virtual ~LanguageChangeNotifier ();
		static void subscribe (QObject *subscriber);

	signals:
		void languageChanged ();

	protected:
		virtual void changeEvent (QEvent *event);

	private:
		static LanguageChangeNotifier *instance;

		LanguageChangeNotifier ();
};

#endif
