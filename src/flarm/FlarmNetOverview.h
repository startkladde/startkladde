#ifndef FLARMNETOVERVIEW_H
#define FLARMNETOVERVIEW_H

#include "ui_FlarmNetOverview.h"
#include "src/gui/SkDialog.h"

class DbManager;
class QSortFilterProxyModel;
class FlarmNetRecord;
template<class T> class ObjectListModel;

class FlarmNetOverview: public SkDialog<Ui::FlarmNetOverviewDialog>
{
	Q_OBJECT

	public:
		FlarmNetOverview (DbManager &dbManager, QWidget *parent);
		virtual ~FlarmNetOverview ();

	private:
		DbManager &dbManager;

		ObjectListModel<FlarmNetRecord> *objectListModel;
		QSortFilterProxyModel* proxyModel;
		QMenu *contextMenu;

	private slots:
		void searchClear();
		void searchTextChanged (const QString&);

		void on_flarmNetTable_customContextMenuRequested (const QPoint &pos);
		void on_createPlaneAction_triggered ();

};

#endif
