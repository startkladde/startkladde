#include "FlarmWindow.h"

#include <cmath>

#include <QtCore/QSettings>
#include <QSortFilterProxyModel>
#include <QPen>
#include <QMessageBox>

#include "qwt_compass.h"
#include "qwt_dial_needle.h"
#include "qwt_compass_rose.h"

#include "src/flarm/FlarmList.h"
#include "src/flarm/FlarmRecordModel.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/config/Settings.h"
#include "src/util/qString.h"
#include "src/nmea/GpsTracker.h"

FlarmWindow::FlarmWindow (QWidget *parent): SkDialog<Ui::FlarmWindowClass> (parent)
{
	ui.setupUi (this);

	ui.kmlWarning->hide ();
	ui.gpsWarning->hide ();

	connect (ui.kmlWarning, SIGNAL (linkActivated (const QString &)), this, SLOT (linkActivated (const QString &)));
	connect (ui.gpsWarning, SIGNAL (linkActivated (const QString &)), this, SLOT (linkActivated (const QString &)));

	connect (ui.flarmMap, SIGNAL (ownPositionUpdated ()), this, SLOT (flarmMapOwnPositionUpdated ()));
	connect (ui.flarmMap, SIGNAL (viewChanged        ()), this, SLOT (flarmMapViewChanged        ()));

	// FIXME load default orientation

	// Setup the table
	ui.flarmTable->resizeColumnsToContents ();
	ui.flarmTable->resizeRowsToContents ();
	ui.flarmTable->setAutoResizeRows (true);
	ui.flarmTable->setAutoResizeColumns (false);

	// Setup the compass
	// The needle will be deleted by compass
	ui.compass->setNeedle (new QwtCompassMagnetNeedle (QwtCompassMagnetNeedle::TriangleStyle, Qt::blue, Qt::red));
	// no use to assign a compass rose, the widget is too small
	// 4 arrows, no levels
	//ui.compass->setRose (new QwtSimpleCompassRose (4,-1));

	// Try to read the KML file. If reading fails, the Flarm map widget will
	// indicate that by its kmlStatus property.
	// The warnings - if any - will be updated as soon as the GPS tracker is
	// set and the FlarmMapWidget pulls the position.
	QString kmlFileName=Settings::instance ().flarmMapKmlFileName;
	ui.flarmMap->readKml (kmlFileName);

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
	ui.compass->setValue (value);
}

void FlarmWindow::on_compass_sliderMoved (double value)
{
	ui.mapOrientationInput->setValue (value);
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

void FlarmWindow::linkActivated (const QString &link)
{
	if (link=="kmlFileDoesNotExist")
	{
		QString title=tr ("KML file does not exist");
		QString text=tr ("The KML file specified in the settings does not exist:\n"
			"%1\n"
			"The file can be changed in the settings dialog.")
			.arg (Settings::instance ().flarmMapKmlFileName);
		QMessageBox::warning (this, title, text);
	}
}

void FlarmWindow::updateWarnings ()
{
	bool ownPositionKnown=ui.flarmMap->isOwnPositionKnown ();
	bool positionWarningShown=false;

	// FIXME test all cases
	// FIXME proper messages
	// FIXME is it possible that by moving the origin out of the window, the
	// layout changes in such a way that the origin is moved in again?
	switch (ui.flarmMap->getKmlStatus ())
	{
		case FlarmMapWidget::kmlNone:
			ui.kmlWarning->showInformation (tr ("No KML file specified"));
			break;
		case FlarmMapWidget::kmlNotFound:
			ui.kmlWarning->showWarning (tr ("The specified KML file does not exist (<a href=\"kmlFileDoesNotExist\">details</a>)"));
			break;
		case FlarmMapWidget::kmlReadError:
			ui.kmlWarning->showWarning (tr ("The specified KML file could not be read"));
			break;
		case FlarmMapWidget::kmlParseError:
			ui.kmlWarning->showWarning (tr ("The specified KML file could not be parsed"));
			break;
		case FlarmMapWidget::kmlEmpty:
			ui.kmlWarning->showWarning (tr ("The specified KML file does not contain any visible elements"));
			break;
		case FlarmMapWidget::kmlOk:
			if (!ownPositionKnown)
			{
				ui.kmlWarning->showWarning (tr ("Own position unknown - KML file useless")); // FIXME proper message
				positionWarningShown=true;
			}
			else if (!ui.flarmMap->isOwnPositionVisible ())
				ui.kmlWarning->showWarning (tr ("Own position not visible"));
//			else if (!ui.flarmMap->anyKmlElementsVisible ()) // FIXME enable
//				ui.kmlWarning->showWarning (tr ("No KML elements visible"));
			else
				ui.kmlWarning->hide ();
			break;
		// no default
	}

	ui.flarmMap->isOwnPositionVisible ();

	// FIXME the problem is that by changing the layout, the graph may be
	// resized before it has been replotted

	if (ownPositionKnown || positionWarningShown)
		ui.gpsWarning->hide ();
	else
		ui.gpsWarning->showWarning (tr ("No GPS data"));
}

void FlarmWindow::flarmMapViewChanged ()
{
	updateWarnings ();
}

void FlarmWindow::flarmMapOwnPositionUpdated ()
{
	updateWarnings ();
}
