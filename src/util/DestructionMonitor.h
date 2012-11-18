#ifndef DESTRUCTIONMONITOR_H_
#define DESTRUCTIONMONITOR_H_

#include <QObject>

class QString;

class DestructionMonitor: public QObject
{
		Q_OBJECT

	public:
		virtual ~DestructionMonitor ();

		static void message (QObject *object, const QString &message=QString ());

	private slots:
		void objectDestroyed ();

	private:
		DestructionMonitor (QObject *object, const QString &message);

		QObject *_object;
		QString _message;

		QString _className;
};

#endif
