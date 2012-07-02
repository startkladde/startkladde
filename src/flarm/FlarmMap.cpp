#include "FlarmMap.h"

#include <cmath>

#include <QtCore/QSettings>
#include <QSortFilterProxyModel>

#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>

#include "src/flarm/FlarmHandler.h"
#include "src/flarm/FlarmRecordModel.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/numeric/Angle.h"
#include "src/numeric/Velocity.h"
#include "src/numeric/GeoPosition.h"

FlarmMap::FlarmMap (QWidget *parent) :
	SkDialog<Ui::FlarmMapDialog> (parent),
	airfieldCurve (NULL), patternCurve (NULL)
{
	ui.setupUi (this);

	// FIXME consider aspect ratio when scaling
	ui.qwtPlot->setAxisScale (QwtPlot::yLeft, -2000.0, 2000.0);
	ui.qwtPlot->setAxisScale (QwtPlot::xBottom, -2000.0, 2000.0);
	QwtPlotGrid* grid = new QwtPlotGrid ();
	QwtPlotPanner* panner = new QwtPlotPanner (ui.qwtPlot->canvas ());
	panner->setAxisEnabled (QwtPlot::yLeft, true);
	panner->setAxisEnabled (QwtPlot::xBottom, true);
	panner->setMouseButton (Qt::LeftButton);
	panner->setEnabled (true);

	QwtPlotMagnifier* magnifier = new QwtPlotMagnifier (ui.qwtPlot->canvas ());
	magnifier->setMouseButton (Qt::MidButton);
	magnifier->setAxisEnabled (QwtPlot::yRight, true);
	magnifier->setAxisEnabled (QwtPlot::xBottom, true);

	// Positive value - mouse wheel down (back) means zooming out. This is the
	// convention that many other applications, including Firefox and Gimp, use.
	magnifier->setMouseFactor (1.05);
	magnifier->setWheelFactor (1.1);
	magnifier->setEnabled (true);

	FlarmHandler* flarmHandler = FlarmHandler::getInstance ();
	connect (flarmHandler, SIGNAL(homePosition(const GeoPosition &)), this, SLOT(ownPositionChanged(const GeoPosition &)));
	connect (flarmHandler, SIGNAL(statusChanged()), this, SLOT(flarmStatusChanged()));

	// FIXME load default
//	transform.rotate (180);

	grid->attach (ui.qwtPlot);

	// Setup the list
	const AbstractObjectList<FlarmRecord *> *objectList = FlarmHandler::getInstance ()->getFlarmRecords ();
	ObjectModel<FlarmRecord *> *objectModel = new FlarmRecordModel ();
	ObjectListModel<FlarmRecord *> *objectListModel = new ObjectListModel<FlarmRecord *> (objectList, false,
			objectModel, true, this);

	// Set the list model as the table's model with a sort proxy
	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (objectListModel);
	proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter (true);
	ui.flarmTable->setModel (proxyModel);

	ui.flarmTable->resizeColumnsToContents ();
	ui.flarmTable->resizeRowsToContents ();
	ui.flarmTable->setAutoResizeRows (true);
	ui.flarmTable->setAutoResizeColumns (false);


	// ***** Static markers

	QwtPlotMarker *startMarker = new QwtPlotMarker ();
	QwtText text ("Start");
	//text.setPaintAttribute (QwtText::PaintBackground, true);
	QColor startColor=Qt::red;
	startColor.setAlpha (127);
	text.setBackgroundBrush (QBrush (startColor));
	startMarker->setLabel (text);
	startMarker->setXValue (0);
	startMarker->setYValue (0);
	startMarker->attach (ui.qwtPlot);


	// ***** Static curves

	airfieldCurve = new QwtPlotCurve ("airfield");
	patternCurve  = new QwtPlotCurve ("pattern");

	airfieldCurve->setRenderHint (QwtPlotItem::RenderAntialiased);
	patternCurve->setRenderHint (QwtPlotItem::RenderAntialiased);

	QPen pen;
	pen.setWidth (2);
	airfieldCurve->setPen (pen);


	airfieldData = new QwtPointSeriesData ();
	patternData = new QwtPointSeriesData ();
	airfieldCurve->setData (airfieldData);
	patternCurve->setData (patternData);

	airfieldCurve->attach (ui.qwtPlot);
	patternCurve->attach (ui.qwtPlot);


	// Instead of reading the vectors from the configuration, you can set example
	// vectors by enabling the call to setExampleVectors instead of readVectors.
	// You can also store the example vectors by enabling the call to
	// storeVectors. After that, you can disable both calls and have the vectors
	// read from the configuration.
//	setExampleVectors ();
//	storeVectors ();
	readVectors ();
}

FlarmMap::~FlarmMap () {
} 

void FlarmMap::setExampleVectors ()
{
	airfieldVector.clear ();
	airfieldVector
		<< GeoPosition::fromDegrees (52.943078, 12.789621)
		<< GeoPosition::fromDegrees (52.94111 , 12.7889  )
		<< GeoPosition::fromDegrees (52.9428  , 12.7703  )
		<< GeoPosition::fromDegrees (52.9444  , 12.7706  );
	airfieldVector << airfieldVector[0];

	patternVector.clear ();
	patternVector
		<< GeoPosition::fromDegrees (52.942123, 12.789271)
		<< GeoPosition::fromDegrees (52.941222, 12.800440)
		<< GeoPosition::fromDegrees (52.941120, 12.801773)
		<< GeoPosition::fromDegrees (52.941320, 12.802625)
		<< GeoPosition::fromDegrees (52.941745, 12.803246)
		<< GeoPosition::fromDegrees (52.947346, 12.804757)
		<< GeoPosition::fromDegrees (52.948177, 12.804512)
		<< GeoPosition::fromDegrees (52.948577, 12.803663)
		<< GeoPosition::fromDegrees (52.948812, 12.802481)
		<< GeoPosition::fromDegrees (52.952181, 12.758622)
		<< GeoPosition::fromDegrees (52.952180, 12.757164)
		<< GeoPosition::fromDegrees (52.951712, 12.755604)
		<< GeoPosition::fromDegrees (52.951071, 12.754953)
		<< GeoPosition::fromDegrees (52.946323, 12.754016)
		<< GeoPosition::fromDegrees (52.945462, 12.754175)
		<< GeoPosition::fromDegrees (52.944926, 12.754900)
		<< GeoPosition::fromDegrees (52.944720, 12.756229)
		<< GeoPosition::fromDegrees (52.943513, 12.770433);
}

/**
 * Not used, only to initialize the table which is stored in settings
 */
void FlarmMap::storeVectors ()
{
	QSettings settings ("startkladde", "startkladde");
	settings.beginGroup ("vectors");

	GeoPosition::storeVector (settings, "airfield", airfieldVector);
	GeoPosition::storeVector (settings, "pattern" , patternVector );

	settings.endGroup ();
}

void FlarmMap::readVectors ()
{
	QSettings settings ("startkladde", "startkladde");
	settings.beginGroup ("vectors");

	airfieldVector=GeoPosition::readVector (settings, "airfield");
	patternVector =GeoPosition::readVector (settings, "pattern");

	settings.endGroup ();
}

void FlarmMap::redrawStaticData ()
{
	QPolygonF airfieldPolygon (GeoPosition::relativePositionTo (airfieldVector, ownPosition));
	QPolygonF patternPolygon  (GeoPosition::relativePositionTo (patternVector , ownPosition));

	airfieldData->setSamples (transform.map (airfieldPolygon));
	patternData ->setSamples (transform.map (patternPolygon));

	ui.qwtPlot->replot ();
}


void FlarmMap::addMinimalPlaneMarker (const FlarmRecord *record)
{
	// ***** Marker


	// Create and setup the marker
	QwtSymbol *symbol=new QwtSymbol (); // Will be deleted in ~Marker
	symbol->setStyle (QwtSymbol::Cross);
	symbol->setSize (8);
	symbol->setPen (QPen (Qt::blue));
	QwtPlotMarker *marker = new QwtPlotMarker ();
	marker->setSymbol (symbol);
	marker->setValue (transform.map (record->getRelativePosition ()));

	// Attach and store the marker
	marker->attach (ui.qwtPlot);
	flarmMarkers.append (marker);
}

void FlarmMap::addVerbosePlaneMarker (const FlarmRecord *record)
{
	QColor climbColor  =Qt::green;  climbColor  .setAlpha (127);
	QColor descentColor=Qt::yellow; descentColor.setAlpha (127);

	// Prepare data
	QwtText text (qnotr ("%1\n%2/%3/%4")
		.arg (record->getRegistration ())
		.arg (record->getRelativeAltitude ())
		.arg (record->getGroundSpeed () / Velocity::km_h)
		.arg (record->getClimbRate (), 0, 'f', 1));

	if (record->getClimbRate () > 0.0)
		text.setBackgroundBrush (QBrush (climbColor));
	else
		text.setBackgroundBrush (QBrush (descentColor));

	// Create and setup the marker
	QwtPlotMarker *marker = new QwtPlotMarker ();
	marker->setLabel (text);
	marker->setValue (transform.map (record->getRelativePosition ()));

	// Attach and store the marker
	marker->attach (ui.qwtPlot);
	flarmMarkers.append (marker);

}

void FlarmMap::addTrail (const FlarmRecord *record)
{
	// Prepare data
	QPolygonF polygon (record->getPreviousRelativePositions ().toVector ());
	QwtPointSeriesData *data = new QwtPointSeriesData (transform.map (polygon));

	// Create and setup the curve
	QwtPlotCurve* curve = new QwtPlotCurve ("history");

	QPen pen;
	pen.setWidth (2);
	curve->setPen (pen);
	curve->setRenderHint (QwtPlotItem::RenderAntialiased);

	curve->setData (data);

	// Attach and store the curve
	curve->attach (ui.qwtPlot);
	flarmCurves.append (curve);
}

void FlarmMap::redrawFlarmData ()
{
	// Detach all Flarm items. Better solution: keep them and just set the
	// updated data; but we'll have to track removal of Flarm records (FIXME)

	foreach (QwtPlotMarker *marker, flarmMarkers)
	{
		marker->detach ();
		delete marker;
	}
	flarmMarkers.clear ();

	foreach (QwtPlotCurve *curve, flarmCurves)
	{
		curve->detach ();
		delete curve;
	}
	flarmCurves.clear ();

	foreach (FlarmRecord *record, FlarmHandler::getInstance ()->getFlarmRecords ()->getList ())
	{
		switch (record->getState ())
		{
			case FlarmRecord::stateStarting:
			case FlarmRecord::stateFlying:
			case FlarmRecord::stateLanding:
				addVerbosePlaneMarker (record);
				addTrail (record);
				break;
			case FlarmRecord::stateOnGround:
				addMinimalPlaneMarker (record);
				break;
			case FlarmRecord::stateUnknown:
			case FlarmRecord::stateFlyingFar:
				// don't draw
				break;
			// no default
		}
	}

	ui.qwtPlot->replot ();
}

void FlarmMap::ownPositionChanged (const GeoPosition &ownPosition)
{
	redrawStaticData ();

	// Only redraw if either the reference position for the static data is
	// invalid (i. e., we didn't draw yet), or the position changed by more
	// than a given threshold. The usual noise on the Flarm GPS data seems to be
	// substantially less than 1 m. Currently, the threshold is set to 0, so the
	// static data is redrawn every time a position is received (typically once
	// per second).
	if (
		!this->ownPosition.isValid () ||
		ownPosition.distanceTo (this->ownPosition)>0)
	{
		this->ownPosition=ownPosition;
		redrawStaticData ();
	}

}

void FlarmMap::flarmStatusChanged ()
{
	redrawFlarmData ();
}

/**
 * Sets the orientation such that upDirection is shown in the up direction in
 * the window
 *
 * The default is north up, or upDirection=0. upDirection=90° means east up.
 *
 * @param upDirection, in radians
 */
void FlarmMap::setOrientation (const Angle &upDirection)
{
	transform=QTransform ();
	transform.rotateRadians (upDirection.toRadians ());

	redrawStaticData ();
	redrawFlarmData ();
}


// **************
// ** UI input **
// **************

void FlarmMap::on_mapOrientationInput_valueChanged (int value)
{
	setOrientation (Angle::fromDegrees (value));
}

void FlarmMap::on_northButton_clicked ()
{
	ui.mapOrientationInput->setValue (0);
}

void FlarmMap::on_reverseButton_clicked ()
{
	int currentOrientation=ui.mapOrientationInput->value ();
	if (currentOrientation>=180)
		ui.mapOrientationInput->setValue (currentOrientation-180);
	else
		ui.mapOrientationInput->setValue (currentOrientation+180);
}
