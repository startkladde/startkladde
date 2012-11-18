#ifndef DESTRUCTIONMONITOR_H_
#define DESTRUCTIONMONITOR_H_

#include <QObject>

class QString;

/**
 * Reacts to the destruction of a QObject
 *
 * Currently, the only implemented reaction is outputting a message on standard
 * output.
 *
 * This class is useful for verifying that an object is actually deleted.
 *
 * This class is not constructed directly; use the static message method:
 *     DestructionMonitor::message (this->layout (), "main window layout");
 */
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
