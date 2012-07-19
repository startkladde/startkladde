#include "FlarmNetOverview.h"

#include <QtCore/QSettings>
#include <QSortFilterProxyModel>

#include "src/flarm/FlarmNetRecord.h"
#include "src/flarm/FlarmNetRecordModel.h"
#include "src/model/objectList/ObjectListModel.h"

FlarmNetOverview::FlarmNetOverview (QWidget *parent, DbManager* db) :
	SkDialog<Ui::FlarmNetOverviewDialog> (parent), dbManager (db)
{
	ui.setupUi (this);

	connect (ui.closeButton, SIGNAL (clicked()), this, SLOT (close()));

	// Setup the list
	QList<dbId> records = dbManager->getCache().getFlarmNetRecordIds (); 
	qDebug () << "number of flarmnet entries: " << records.count() << endl;
	
	//TODO: this is ugly code. This should be in the database
	//const AbstractObjectList<FlarmNetRecord *> *objectList = new MutableObjectList<FlarmNetRecord*>();
	MutableObjectList<FlarmNetRecord *> *objectList = new MutableObjectList<FlarmNetRecord*>();
	foreach (dbId id, records) {
		// we use the implicit copy constructor !?
		FlarmNetRecord* record = new FlarmNetRecord (dbManager->getCache ().getObject<FlarmNetRecord> (id));
		objectList->append (record);
	}
	
	ObjectModel<FlarmNetRecord *> *objectModel = new FlarmNetRecordModel ();
	ObjectListModel<FlarmNetRecord *> *objectListModel = new ObjectListModel<FlarmNetRecord *> (objectList, false,
			objectModel, true, this);

	// Set the list model as the table's model with a sort proxy
	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (objectListModel);
	proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter (true);
	ui.flarmNetTable->setModel (proxyModel);

	ui.flarmNetTable->resizeColumnsToContents ();
	ui.flarmNetTable->resizeRowsToContents ();
	ui.flarmNetTable->setAutoResizeRows (true);
	//ui.flarmTable->setAutoResizeColumns (false);
}

FlarmNetOverview::~FlarmNetOverview () {
} 
