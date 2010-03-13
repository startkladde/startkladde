#include "ObjectListWindowBase.h"

#include <iostream>

#include <QKeyEvent>
#include <QInputDialog>

#include "src/util/qString.h"

ObjectListWindowBase::ObjectListWindowBase (DbManager &manager, QWidget *parent):
	QMainWindow(parent), manager (manager),
	editPasswordRequired (false), editPasswordOk (false)
{
	ui.setupUi(this);
	setAttribute (Qt::WA_DeleteOnClose, true);

	QObject::connect (&manager, SIGNAL (stateChanged (DbManager::State)), this, SLOT (databaseStateChanged (DbManager::State)));
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

void ObjectListWindowBase::databaseStateChanged (DbManager::State state)
{
	if (state==DbManager::stateDisconnected)
		close ();
}

void ObjectListWindowBase::requireEditPassword (const QString &password)
{
	editPassword=password;
	editPasswordRequired=true;
}

bool ObjectListWindowBase::allowEdit (QString message)
{
	if (!editPasswordRequired) return true;
	if (editPasswordOk) return true;

	while (true)
	{
		bool ok=false;
		QString enteredPassword=QInputDialog::getText (this, "Passwort erforderlich",
			utf8 ("%1 Bitte Passwort eingeben:").arg (message), QLineEdit::Password, QString (), &ok);

		// Canceled
		if (!ok) return false;

		if (enteredPassword==editPassword)
		{
			editPasswordOk=true;
			return true;
		}

		message="Das eingegebene Passwort ist nicht korrekt.";
	}
}

