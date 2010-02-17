/*
 * ObjectEditorPane.cpp
 *
 *  Created on: Aug 22, 2009
 *      Author: mherrman
 */

#include "ObjectEditorPane.h"

#include "src/text.h"

// ***************************************
// ** ObjectEditorPaneBase construction **
// ***************************************

ObjectEditorPaneBase::ObjectEditorPaneBase (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent):
	QWidget (parent),
	dataStorage (dataStorage), mode (mode)
{
}

ObjectEditorPaneBase::~ObjectEditorPaneBase ()
{
}

// **********
// ** Misc **
// **********

void ObjectEditorPaneBase::errorCheck (const QString &problem, QWidget *widget)
{
	if (!confirmProblem (this, "Fehler", problem))
	{
		if (widget) widget->setFocus ();
		throw AbortedException ();
	}
}

// TODO use more
void ObjectEditorPaneBase::requiredField (const QString &value, QWidget *widget, const QString &problem)
{
	if (eintrag_ist_leer (value))
		errorCheck (problem, widget);
}
