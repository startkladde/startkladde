#ifndef FLARMNETOVERVIEW_H
#define FLARMNETOVERVIEW_H

#include "ui_FlarmNetOverview.h"
#include "src/gui/SkDialog.h"

class DbManager;
class QSortFilterProxyModel;

class FlarmNetOverview: public SkDialog<Ui::FlarmNetOverviewDialog>
{
	Q_OBJECT

	public:
		FlarmNetOverview (DbManager &dbManager, QWidget *parent);
		~FlarmNetOverview ();

	private:
		QSortFilterProxyModel* proxyModel;

	private slots:
		void searchClear();
		void searchTextChanged (const QString&);
};

#endif
