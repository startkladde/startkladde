#include "src/util/DestructionMonitor.h"

#include <QString>

#include "src/util/qString.h"

DestructionMonitor::DestructionMonitor (QObject *object, const QString &message):
	QObject (object),
	_object (object), _message (message)
{
	// Store the class name here. In the objectDestroyed slot, it always returns
	// QObject.
	_className=object->metaObject()->className ();
	connect (object, SIGNAL (destroyed ()), this, SLOT (objectDestroyed ()));
}

DestructionMonitor::~DestructionMonitor ()
{
}

void DestructionMonitor::objectDestroyed ()
{
	QString text="Object destroyed: " + _className;

	// Add the object name, if it is set
	QString objectName=_object->objectName ();
	if (!objectName.isEmpty ())
		text+=" "+objectName;

	// Add the message, if it is set
	if (!_message.isEmpty ())
		text+=" ("+_message+")";

	std::cout << text << std::endl;
}

void DestructionMonitor::message (QObject *object, const QString &text)
{
	// Will be deleted by its parent, object
	new DestructionMonitor (object, text);
}
