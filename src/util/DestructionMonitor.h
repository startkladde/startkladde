#ifndef DESTRUCTIONMONITOR_H_
#define DESTRUCTIONMONITOR_H_

#include <iostream>

#include <QObject>

#include "src/util/qString.h"

class QString;

class DestructionMonitor: public QObject
{
		Q_OBJECT

	public:
		virtual ~DestructionMonitor ();

		// QObject must be an ancestor of T
		template<class T> static T *message (
			T *object, const QString &message=QString ());

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
	else
	{
		if (text.isEmpty ())
			std::cerr << "Destruction monitor requested for NULL object" << std::endl;
		else
			std::cerr << "Destruction monitor requested for NULL object: " << text << std::endl;

	}

	return object;
}

#endif
