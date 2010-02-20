#ifndef OBJECTLISTWINDOWBASE_H
#define OBJECTLISTWINDOWBASE_H

#include <QtGui/QMainWindow>

#include "ui_ObjectListWindowBase.h"

/*
 * TODO: Menu View->Sort and hotkey
 */

class ObjectListWindowBase : public QMainWindow
{
    Q_OBJECT

	public:
		ObjectListWindowBase(QWidget *parent = 0);
		~ObjectListWindowBase();

	public slots:
		virtual void on_actionNew_triggered ()=0;
		virtual void on_actionEdit_triggered ()=0;
		virtual void on_actionDelete_triggered ()=0;
		virtual void on_actionRefresh_triggered ()=0;
		virtual void on_actionClose_triggered ();

		virtual void on_table_activated (const QModelIndex &index)=0;

	protected:
		void keyPressEvent (QKeyEvent *e);
		Ui::ObjectListWindowBaseClass ui;
};

#endif // OBJECTLISTWINDOWBASE_H
