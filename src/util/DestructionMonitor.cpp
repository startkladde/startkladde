#include "src/util/DestructionMonitor.h"

#include <iostream>

#include <QString>
#include <QDebug>

#include "src/util/qString.h"

/**
 * Note that the destruction monitor does not become a parent of the object: if
 * it did, it would be deleted before receiving the destroyed() signal from the
 * object.
 */
DestructionMonitor::DestructionMonitor (QObject *object, const QString &message):
	QObject (NULL),
	_object (object), _message (message)
{
	// Store the class name here. In the objectDestroyed slot, it always returns
	// QObject.
	//qDebug () << "Add destruction monitor for" << object;
	_className=object->metaObject()->className ();
	_objectName=object->objectName ();
	connect (object, SIGNAL (destroyed ()), this, SLOT (objectDestroyed ()));
}

DestructionMonitor::~DestructionMonitor ()
{
}

void DestructionMonitor::objectDestroyed ()
{
	QString text="Object destroyed: " + _className;

	// Add the object name, if it is set
	if (!_objectName.isEmpty ())
		text+=" "+_objectName;

	// Add the message, if it is set
	if (!_message.isEmpty ())
		text+=" ("+_message+")";

	std::cerr << text << std::endl;

	this->deleteLater ();
}

//void DestructionMonitor::message (QObject *object, const QString &text)
//{
//	// FIXME error message
//	if (!object)
//		return;

//	// Will be deleted by its parent, object
//	new DestructionMonitor (object, text);
//}
