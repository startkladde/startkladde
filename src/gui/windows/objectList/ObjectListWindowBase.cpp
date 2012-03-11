#include "ObjectListWindowBase.h"

#include <iostream>

#include <QKeyEvent>
#include <QPushButton>

#include "src/util/qString.h"

ObjectListWindowBase::ObjectListWindowBase (DbManager &manager, QWidget *parent):
	QMainWindow(parent), manager (manager),
	editPermission (this)
{
	ui.setupUi(this);

	QObject::connect (&manager, SIGNAL (stateChanged (DbManager::State)), this, SLOT (databaseStateChanged (DbManager::State)));
}

ObjectListWindowBase::~ObjectListWindowBase()
{
}

void ObjectListWindowBase::on_actionClose_triggered ()
{
	close ();
}

void ObjectListWindowBase::keyPressEvent (QKeyEvent *e)
{
//	std::cout << "ObjectListWindowBase key " << e->key () << std::endl;

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

void ObjectListWindowBase::databaseStateChanged (DbManager::State state)
{
	if (state==DbManager::stateDisconnected)
		close ();
}

void ObjectListWindowBase::requireEditPassword (const QString &password)
{
	editPermission.requirePassword (password);
}

bool ObjectListWindowBase::allowEdit (QString message)
{
	return editPermission.permit (message);
}

void ObjectListWindowBase::changeEvent (QEvent *event)
{
    if (event->type () == QEvent::LanguageChange)
		// The language changed. Retranslate the UI.
    	ui.retranslateUi (this);
    else
        QWidget::changeEvent (event);
}


