/*
 * ObjectEditorWindowBase.cpp
 *
 *  Created on: Aug 23, 2009
 *      Author: mherrman
 */

#include "ObjectEditorWindowBase.h"

ObjectEditorWindowBase::ObjectEditorWindowBase (DataStorage &dataStorage, QWidget *parent, Qt::WindowFlags flags):
	QDialog (parent, flags),
	dataStorage (dataStorage)
{
	ui.setupUi (this);
}

ObjectEditorWindowBase::~ObjectEditorWindowBase ()
{

}

