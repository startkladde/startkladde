#include "FlarmWindow.h"

#include <cmath>

#include <QtCore/QSettings>
#include <QSortFilterProxyModel>
#include <QPen>

#include "src/flarm/FlarmList.h"
#include "src/flarm/FlarmRecordModel.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/config/Settings.h"

FlarmWindow::FlarmWindow (QWidget *parent): SkDialog<Ui::FlarmWindowClass> (parent)
{
	ui.setupUi (this);

	// FIXME load default orientation

	ui.flarmTable->resizeColumnsToContents ();
	ui.flarmTable->resizeRowsToContents ();
	ui.flarmTable->setAutoResizeRows (true);
	ui.flarmTable->setAutoResizeColumns (false);

	// FIXME what if it doesn't exist?
	ui.flarmMap->readKml (Settings::instance ().flarmMapKmlFileName);

	ui.flarmMap->setFocus ();
}

FlarmWindow::~FlarmWindow () {
} 

void FlarmWindow::setGpsTracker (GpsTracker *gpsTracker)
{
	ui.flarmMap->setGpsTracker (gpsTracker);
}

void FlarmWindow::setFlarmList (FlarmList *flarmList)
{
	// Setup the map
	ui.flarmMap->setFlarmList (flarmList);

	// Setup the list
	const AbstractObjectList<FlarmRecord> *objectList = flarmList;
	ObjectModel<FlarmRecord> *objectModel = new FlarmRecordModel ();
	ObjectListModel<FlarmRecord> *objectListModel = new ObjectListModel<FlarmRecord> (objectList, false,
			objectModel, true, this);

	// FIXME create & set the proxy model in the constructor
	// Set the list model as the table's model with a sort proxy
	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (objectListModel);
	proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter (true);
	ui.flarmTable->setModel (proxyModel);
}


// ***************
// ** UI inputs **
// ***************

void FlarmWindow::on_mapOrientationInput_valueChanged (int value)
{
	ui.flarmMap->setOrientation (Angle::fromDegrees (value));
}

void FlarmWindow::on_northButton_clicked ()
{
	ui.mapOrientationInput->setValue (0);
}

void FlarmWindow::on_reverseButton_clicked ()
{
	int currentOrientation=ui.mapOrientationInput->value ();
	if (currentOrientation>=180)
		ui.mapOrientationInput->setValue (currentOrientation-180);
	else
		ui.mapOrientationInput->setValue (currentOrientation+180);
}
