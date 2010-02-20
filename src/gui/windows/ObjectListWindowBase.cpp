#include "ObjectListWindowBase.h"

#include <iostream>

#include <QKeyEvent>

ObjectListWindowBase::ObjectListWindowBase(QWidget *parent):
	QMainWindow(parent)
{
	ui.setupUi(this);
	setAttribute (Qt::WA_DeleteOnClose, true);
}

ObjectListWindowBase::~ObjectListWindowBase()
{
}

void ObjectListWindowBase::on_actionClose_triggered ()
{
	ui.closeButton->click ();
}

void ObjectListWindowBase::keyPressEvent (QKeyEvent *e)
{
	// KeyEvents are accepted by default
	switch (e->key ())
	{
		case Qt::Key_F2: ui.actionNew->trigger (); break;
		case Qt::Key_F4: ui.actionEdit->trigger (); break;
		case Qt::Key_F8: ui.actionDelete->trigger (); break;
		case Qt::Key_F12: ui.actionRefresh->trigger (); break;
		case Qt::Key_Escape: ui.actionClose->trigger(); break;
		case Qt::Key_Insert: ui.actionNew->trigger (); break;
		case Qt::Key_Delete: ui.actionDelete->trigger (); break;
		default: e->ignore (); break;
	}

	if (!e->isAccepted ()) QMainWindow::keyPressEvent (e);
}
