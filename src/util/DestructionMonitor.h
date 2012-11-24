#ifndef DESTRUCTIONMONITOR_H_
#define DESTRUCTIONMONITOR_H_

#include <QObject>

class QString;

class DestructionMonitor: public QObject
{
		Q_OBJECT

	public:
		virtual ~DestructionMonitor ();

		// QObject must be an ancestor of T
		template<class T> static T *message (
			T *object, const QString &message=QString ());

//		static void message (QObject *object, const QString &message=QString ());

	private slots:
		void objectDestroyed ();

	private:
		DestructionMonitor (QObject *object, const QString &message);

		QObject *_object;
		QString _message;

		QString _className;
		QString _objectName;
};


template<class T> T *DestructionMonitor::message (T *object, const QString &text)
{
	// Will be deleted by its parent, object
	if (object)
		new DestructionMonitor (object, text);
	// FIXME else error message

	return object;
}

#endif
