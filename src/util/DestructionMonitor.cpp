#include "src/util/DestructionMonitor.h"

#include <QString>

#include "src/util/qString.h"

/**
 * Constructs a destruction monitor instance for the given object and stores the
 * given message
 */
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

/**
 * The slot called when the object is destroyed. Outputs a message.
 */
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

/**
 * Outputs a message when the specified object is destroyed
 *
 * The message includes the class of the object (determined via the QObject's
 * MetaObject), the object name (if it is set) and the text passed to this
 * method (unless it is empty).
 */
void DestructionMonitor::message (QObject *object, const QString &text)
{
	// Will be deleted by its parent, object
	new DestructionMonitor (object, text);
}
