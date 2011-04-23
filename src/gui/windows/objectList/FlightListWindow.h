#ifndef FLIGHTLISTWINDOW_H
#define FLIGHTLISTWINDOW_H

#include <QtGui/QMainWindow>
#include "ui_FlightListWindow.h"

#include "src/db/DbManager.h" // Required for DbManager::State

/**
 * A window for showing and exporting a flight list
 *
 * Note that this window is not based on ObjectListWindow as it is significantly
 * different from other object list windows:
 *   * it does not allow creating and editing of objects
 *   * the data it displays is not cached by the database
 *   * it allow retrieving different sets of data
 */
class FlightListWindow: public QMainWindow
{
	Q_OBJECT

	public:
		FlightListWindow (DbManager &manager, QWidget *parent=0);
		~FlightListWindow();

		static void show (DbManager &manager, QWidget *parent);

		using QMainWindow::show;

	public slots:
		virtual void on_actionClose_triggered ();

		virtual void on_actionSelectDate_triggered ();
		virtual void on_actionExport_triggered ();

	protected:
		void keyPressEvent (QKeyEvent *e);

	protected slots:
		virtual void databaseStateChanged (DbManager::State state);

	private:
		Ui::FlightListWindowClass ui;

		DbManager &manager;

//		ObjectList<Flight> *list;
//		ObjectModel<Flight> *objectModel;
//		ObjectListModel<Flight> *listModel;
//
//		QSortFilterProxyModel *proxyModel;

};

#endif
