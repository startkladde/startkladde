#ifndef LAUNCHMETHODSELECTIONWINDOW_H
#define LAUNCHMETHODSELECTIONWINDOW_H

#include <QtGui/QDialog>
#include "ui_LaunchMethodSelectionWindow.h"

#include "src/db/dbId.h"

class Cache;

class LaunchMethodSelectionWindow: public QDialog
{
    Q_OBJECT

	public:
		LaunchMethodSelectionWindow (QWidget *parent=NULL);
		~LaunchMethodSelectionWindow ();

		static bool select (Cache &cache, dbId &value, QWidget *parent=NULL);

	private:
		Ui::LaunchMethodSelectionWindowClass ui;
};

#endif
