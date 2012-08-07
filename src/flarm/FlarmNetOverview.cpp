#include "FlarmNetOverview.h"

#include <QSettings>
#include <QSortFilterProxyModel>

#include "src/db/DbManager.h"
#include "src/flarm/FlarmNetRecord.h"
#include "src/flarm/FlarmNetRecordModel.h"
#include "src/model/objectList/ObjectListModel.h"

class FlarmNetRecord;

FlarmNetOverview::FlarmNetOverview (DbManager &dbManager, QWidget *parent):
	SkDialog<Ui::FlarmNetOverviewDialog> (parent)
{
	ui.setupUi (this);

	// Get the list of FlarmNet records from the database. It will be deleted by
	// the ObjectListModel.
	EntityList<FlarmNetRecord> *flarmNetRecords=new EntityList<FlarmNetRecord> (dbManager.getCache ().getFlarmNetRecords ());
	
	// Create the object model. It will be deleted by the ObjectListModel.
	ObjectModel<FlarmNetRecord> *flarmNetRecordModel = new FlarmNetRecordModel ();

	// Create the object list model. It will be deleted by its parent, this.
	ObjectListModel<FlarmNetRecord> *objectListModel = new ObjectListModel<FlarmNetRecord> (
		flarmNetRecords, true,
		flarmNetRecordModel, true,
		this);

	// Create a sort/filter proxy model. It will be deleted by its parent, this.
	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (objectListModel);
	proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter (true);

	// Setup the table view
	ui.flarmNetTable->setModel (proxyModel);
	ui.flarmNetTable->resizeColumnsToContents ();
	ui.flarmNetTable->resizeRowsToContents ();
}

FlarmNetOverview::~FlarmNetOverview ()
{
} 
