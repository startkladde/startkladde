#include "FlarmWindow.h"

#include <cmath>

#include <QtCore/QSettings>
#include <QSortFilterProxyModel>
#include <QPen>

#include "src/flarm/FlarmList.h"
#include "src/flarm/FlarmRecordModel.h"
#include "src/model/objectList/ObjectListModel.h"


FlarmWindow::FlarmWindow (QWidget *parent): SkDialog<Ui::FlarmWindowClass> (parent)
{
	ui.setupUi (this);

	// FIXME load default orientation

	ui.flarmTable->resizeColumnsToContents ();
	ui.flarmTable->resizeRowsToContents ();
	ui.flarmTable->setAutoResizeRows (true);
	ui.flarmTable->setAutoResizeColumns (false);

	// Instead of reading the vectors from the configuration, you can set example
	// vectors by enabling the call to setExampleVectors instead of readVectors.
	// You can also store the example vectors by enabling the call to
	// storeVectors. After that, you can disable both calls and have the vectors
	// read from the configuration.
//	setExampleVectors ();
//	storeVectors ();
	readVectors ();
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

	// FIXME create & setyo the proxy model in the constructor
	// Set the list model as the table's model with a sort proxy
	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (objectListModel);
	proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter (true);
	ui.flarmTable->setModel (proxyModel);
}

void FlarmWindow::setExampleVectors ()
{
	// FIXME
//	QVector<GeoPosition> vector;
//
//	vector.clear ();
//	vector
//		<< GeoPosition::fromDegrees (52.943078, 12.789621)
//		<< GeoPosition::fromDegrees (52.94111 , 12.7889  )
//		<< GeoPosition::fromDegrees (52.9428  , 12.7703  )
//		<< GeoPosition::fromDegrees (52.9444  , 12.7706  );
//	vector << vector[0];
//	ui.flarmMap->setAirfieldVector (vector);
//
//	vector.clear ();
//	vector
//		<< GeoPosition::fromDegrees (52.942123, 12.789271)
//		<< GeoPosition::fromDegrees (52.941222, 12.800440)
//		<< GeoPosition::fromDegrees (52.941120, 12.801773)
//		<< GeoPosition::fromDegrees (52.941320, 12.802625)
//		<< GeoPosition::fromDegrees (52.941745, 12.803246)
//		<< GeoPosition::fromDegrees (52.947346, 12.804757)
//		<< GeoPosition::fromDegrees (52.948177, 12.804512)
//		<< GeoPosition::fromDegrees (52.948577, 12.803663)
//		<< GeoPosition::fromDegrees (52.948812, 12.802481)
//		<< GeoPosition::fromDegrees (52.952181, 12.758622)
//		<< GeoPosition::fromDegrees (52.952180, 12.757164)
//		<< GeoPosition::fromDegrees (52.951712, 12.755604)
//		<< GeoPosition::fromDegrees (52.951071, 12.754953)
//		<< GeoPosition::fromDegrees (52.946323, 12.754016)
//		<< GeoPosition::fromDegrees (52.945462, 12.754175)
//		<< GeoPosition::fromDegrees (52.944926, 12.754900)
//		<< GeoPosition::fromDegrees (52.944720, 12.756229)
//		<< GeoPosition::fromDegrees (52.943513, 12.770433);
//	ui.flarmMap->setPatternVector (vector);
}

/**
 * Not used, only to initialize the table which is stored in settings
 */
void FlarmWindow::storeVectors ()
{
	// FIXME
//	QSettings settings ("startkladde", "startkladde");
//	settings.beginGroup ("vectors");
//
//	GeoPosition::storeVector (settings, "airfield", ui.flarmMap->getAirfieldVector ());
//	GeoPosition::storeVector (settings, "pattern" , ui.flarmMap->getPatternVector  ());
//
//	settings.endGroup ();
}

void FlarmWindow::readVectors ()
{
	QSettings settings ("startkladde", "startkladde");
	settings.beginGroup ("vectors");

	QPen airfieldPen;
	airfieldPen.setWidth (2);

	ui.flarmMap->addStaticCurve ("airfield", GeoPosition::readVector (settings, "airfield"), airfieldPen);
	ui.flarmMap->addStaticCurve ("pattern" , GeoPosition::readVector (settings, "pattern" ), QPen ());

	// FIXME read from config or file
	ui.flarmMap->addStaticMarker ("Halle", GeoPosition::fromDegrees (52.945078, 12.786621), QColor (255, 255, 255, 127));

	settings.endGroup ();

	ui.flarmMap->replot ();
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
