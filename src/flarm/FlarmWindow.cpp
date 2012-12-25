#include "FlarmWindow.h"

#include <cmath>

#include <QtCore/QSettings>
#include <QSortFilterProxyModel>
#include <QPen>
#include <QMessageBox>
#include <QStatusBar>

#include "qwt_compass.h"
#include "qwt_dial_needle.h"
#include "qwt_compass_rose.h"

#include "src/flarm/FlarmList.h"
#include "src/flarm/FlarmRecordModel.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/config/Settings.h"
#include "src/util/qString.h"
#include "src/nmea/GpsTracker.h"
#include "src/numeric/Distance.h"

FlarmWindow::FlarmWindow (QWidget *parent): SkDialog<Ui::FlarmWindowClass> (parent),
	gpsTracker (NULL)
{
	ui.setupUi (this);

	// Add a status bar
	statusBar=new QStatusBar (this);
	layout ()->addWidget (statusBar);

	// Add the mouse position label to the status bar
	mousePositionLabel=new QLabel (this);
	statusBar->addPermanentWidget (mousePositionLabel);

	ui.kmlWarning->hide ();
	ui.gpsWarning->hide ();
	ui.positionWarning->hide ();

	ui.tabWidget->setCurrentIndex (0);

	connect (ui.kmlWarning     , SIGNAL (linkActivated (const QString &)), this, SLOT (linkActivated (const QString &)));
	connect (ui.gpsWarning     , SIGNAL (linkActivated (const QString &)), this, SLOT (linkActivated (const QString &)));
	connect (ui.positionWarning, SIGNAL (linkActivated (const QString &)), this, SLOT (linkActivated (const QString &)));

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

	// Try to read the KML file. If reading fails, the Flarm radar widget will
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
	// Setup the flarm radar
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
	if (link=="kmlFileNotSpecified")
	{
		QString title=tr ("KML file not specified");
		QString text=tr ("You can specify a KML file to show map elements"
			" as lines (e. g. airfield boundaries, traffic circuit..)"
			" or points (e. g. hangars, buildings...)."
			" KML files can be created with Google Earth or"
			" Google Maps.");
		QMessageBox::information (this, title, text);
	}
	else if (link=="kmlFileDoesNotExist")
	{
		QString title=tr ("KML file does not exist");
		QString text=tr ("The KML file specified in the settings does not exist:\n"
			"%1\n"
			"The file can be changed in the settings dialog.")
			.arg (Settings::instance ().flarmMapKmlFileName);
		QMessageBox::warning (this, title, text);
	}
	else if (link=="resetPosition")
	{
		ui.flarmMap->resetPosition ();
	}
	else if (link=="noKmlElementsVisible")
	{
		double distance;
		Angle bearing;

		bool ok=ui.flarmMap->findClosestStaticElement (&distance, &bearing);
		if (ok)
		{
			// Note that this is not generic functionality - the wording and
			// translation may depend on the context. For example, "north-west"
			// may become "nordwestlich" or "Nordwesten" in German.
			QString bearingString;
			switch (bearing.compassSection (8))
			{
				case 0: bearingString=tr ("north"     ); break;
				case 1: bearingString=tr ("north-east"); break;
				case 2: bearingString=tr ("east"      ); break;
				case 3: bearingString=tr ("south-east"); break;
				case 4: bearingString=tr ("south"     ); break;
				case 5: bearingString=tr ("south-west"); break;
				case 6: bearingString=tr ("west"      ); break;
				case 7: bearingString=tr ("north-west"); break;
				default: bearingString=notr ("???"); break;
			}

			QString title=tr ("No KML elements visible");
			QString text=tr ("None of the elements read from the KML file are currently visible."
				" The KML element closest to the current position is located %1 to the %2 of the current position.")
				.arg (Distance::format (distance, 0))
				.arg (bearingString);
			QMessageBox::warning (this, title, text);
		}
		else
		{
			QString title=tr ("No KML elements visible");
			QString text=tr ("None of the elements read from the KML file are currently visible.");
			QMessageBox::warning (this, title, text);
		}
	}
}

void FlarmWindow::updateWarnings ()
{
	// FIXME test all cases
	// FIXME proper messages
	// FIXME is it possible that by moving the origin out of the window, the
	// layout changes in such a way that the origin is moved in again?
	// => yes, move it out to the right or left, when the widget is wider than
	//    high

	switch (ui.flarmMap->getKmlStatus ())
	{
		case FlarmMapWidget::kmlNone:
			ui.kmlWarning->showInformation (tr ("No KML file specified  (<a href=\"kmlFileNotSpecified\">details</a>)"));
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
			ui.kmlWarning->showWarning (tr ("The specified KML file does not contain any elements"));
			break;
		case FlarmMapWidget::kmlOk:
			if (ui.flarmMap->isOwnPositionKnown () && !ui.flarmMap->isAnyStaticElementVisible ())
				ui.kmlWarning->showWarning (tr ("None of the KML elements are visible (<a href=\"noKmlElementsVisible\">details</a>)"));
			else
				ui.kmlWarning->hide ();
			break;
		// no default
	}

	if (ui.flarmMap->isOwnPositionKnown ()) // GPS good
		ui.gpsWarning->hide ();
	else
	{
		if (ui.flarmMap->getKmlStatus ()==FlarmMapWidget::kmlOk)
			// We could show the KML data, if only we had GPS
			ui.gpsWarning->showWarning (tr ("No GPS data. The data from the KML file cannot be shown."));
		else
			ui.gpsWarning->showWarning (tr ("No GPS data"));
	}

	if (!ui.flarmMap->isOwnPositionVisible ())
		ui.positionWarning->showWarning (tr ("The own position is not visible (<a href=\"resetPosition\">reset</a>)"));
	else
		ui.positionWarning->hide ();
}

void FlarmWindow::flarmMapViewChanged ()
{
	updateWarnings ();
}

void FlarmWindow::flarmMapOwnPositionUpdated ()
{
	updateWarnings ();
}

void FlarmWindow::on_flarmMap_mouseMoved_p (QPointF position_p)
{
	double north=position_p.y ();
	double east=position_p.x ();

	QString northText;
	QString eastText;

	if (north>=0)
		northText=tr ("%1 N").arg (Distance::format (north, 0));
	else
		northText=tr ("%1 S").arg (Distance::format (-north, 0));

	if (east>=0)
		eastText=tr ("%1 E").arg (Distance::format (east, 0));
	else
		eastText=tr ("%1 W").arg (Distance::format (-east, 0));


	mousePositionLabel->setText (tr ("%1, %2").arg (northText, eastText));
}

void FlarmWindow::on_flarmMap_mouseLeft ()
{
	mousePositionLabel->clear ();
}

void FlarmWindow::on_flarmMap_orientationChanged ()
{
	int degrees=round (ui.flarmMap->orientation ().toDegrees ());
	ui.mapOrientationInput->setValue (degrees);
	ui.compass->setValue (degrees);
}
