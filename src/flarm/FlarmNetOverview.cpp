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

	//search button
        QStyle* style = QApplication::style();
	ui.clearButton->setIcon (style->standardIcon(QStyle::SP_DialogDiscardButton));

        connect (ui.searchEdit,  SIGNAL(textChanged(const QString&)), this, SLOT(searchTextChanged(const QString&)));
        connect (ui.clearButton, SIGNAL(pressed()), this, SLOT(searchClear()));

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
	proxyModel = new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (objectListModel);
	proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter (true);

	// filter all columns
	proxyModel->setFilterKeyColumn (-1);
	ui.flarmNetTable->setModel (proxyModel);
	
	// Setup the table view
	ui.flarmNetTable->setModel (proxyModel);
	ui.flarmNetTable->resizeColumnsToContents ();
	ui.flarmNetTable->resizeRowsToContents ();
}

FlarmNetOverview::~FlarmNetOverview ()
{
} 

void FlarmNetOverview::searchClear () {
        // qDebug () << "FlarmNetOverview::searchClear: " << endl;
        ui.searchEdit->clear();
}

void FlarmNetOverview::searchTextChanged (const QString& search) {
        // qDebug () << "FlarmNetOverview::searchTextChanged: " << search << endl;
        proxyModel->setFilterRegExp (QRegExp (search, Qt::CaseInsensitive, QRegExp::FixedString));
} 
