#include <src/flarm/FlarmMapWidget.h>

#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>

#include "src/flarm/FlarmRecord.h"
#include "src/flarm/FlarmHandler.h"
#include "src/numeric/Velocity.h"
#include "src/numeric/GeoPosition.h"
#include "src/flarm/FlarmList.h"

#include "src/i18n/notr.h"

FlarmMapWidget::FlarmMapWidget (QWidget *parent): QwtPlot (parent),
	airfieldCurve (NULL), patternCurve (NULL),
	flarmList (NULL)
{
	// FIXME consider aspect ratio when scaling
	setAxisScale (QwtPlot::yLeft, -2000.0, 2000.0);
	setAxisScale (QwtPlot::xBottom, -2000.0, 2000.0);
	QwtPlotGrid* grid = new QwtPlotGrid ();
	QwtPlotPanner* panner = new QwtPlotPanner (canvas ());
	panner->setAxisEnabled (QwtPlot::yLeft, true);
	panner->setAxisEnabled (QwtPlot::xBottom, true);
	panner->setMouseButton (Qt::LeftButton);
	panner->setEnabled (true);

	QwtPlotMagnifier* magnifier = new QwtPlotMagnifier (canvas ());
	magnifier->setMouseButton (Qt::MidButton);
	magnifier->setAxisEnabled (QwtPlot::yRight, true);
	magnifier->setAxisEnabled (QwtPlot::xBottom, true);

	// Positive value - mouse wheel down (back) means zooming out. This is the
	// convention that many other applications, including Firefox and Gimp, use.
	magnifier->setMouseFactor (1.05);
	magnifier->setWheelFactor (1.1);
	magnifier->setEnabled (true);

	grid->attach (this);

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
	startMarker->attach (this);


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

	airfieldCurve->attach (this);
	patternCurve->attach (this);

	redrawFlarmData ();
}

FlarmMapWidget::~FlarmMapWidget ()
{
}

// TODO all temporary, we want an arbitrary number of vectors
void FlarmMapWidget::setAirfieldVector (const QVector<GeoPosition> &vector)
{
	this->airfieldVector=vector;
	redrawStaticData ();
}

void FlarmMapWidget::setPatternVector (const QVector<GeoPosition> &vector)
{
	this->patternVector=vector;
	redrawStaticData ();
}

QVector<GeoPosition> FlarmMapWidget::getAirfieldVector () const
{
	return airfieldVector;
}

QVector<GeoPosition> FlarmMapWidget::getPatternVector () const
{
	return patternVector;
}


void FlarmMapWidget::redrawStaticData ()
{
	QPolygonF airfieldPolygon (GeoPosition::relativePositionTo (airfieldVector, ownPosition));
	QPolygonF patternPolygon  (GeoPosition::relativePositionTo (patternVector , ownPosition));

	airfieldData->setSamples (transform.map (airfieldPolygon));
	patternData ->setSamples (transform.map (patternPolygon));

	replot ();
}


void FlarmMapWidget::addMinimalPlaneMarker (const FlarmRecord *record)
{
	// Create and setup the marker
	QwtSymbol *symbol=new QwtSymbol (); // Will be deleted in ~Marker
	symbol->setStyle (QwtSymbol::Cross);
	symbol->setSize (8);
	symbol->setPen (QPen (Qt::blue));
	QwtPlotMarker *marker = new QwtPlotMarker ();
	marker->setSymbol (symbol);
	marker->setValue (transform.map (record->getRelativePosition ()));

	// Attach and store the marker
	marker->attach (this);
	flarmMarkers.append (marker);
}

void FlarmMapWidget::addVerbosePlaneMarker (const FlarmRecord *record)
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
	marker->attach (this);
	flarmMarkers.append (marker);

}

void FlarmMapWidget::addTrail (const FlarmRecord *record)
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
	curve->attach (this);
	flarmCurves.append (curve);
}

void FlarmMapWidget::redrawFlarmData ()
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

	if (flarmList)
	{
		for (int i=0, n=flarmList->size (); i<n; ++i)
		{
			const FlarmRecord *record=&FlarmHandler::getInstance ()->getFlarmList ().at (i);

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
	}

	replot ();
}

void FlarmMapWidget::ownPositionChanged (const GeoPosition &ownPosition)
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

/**
 * Sets the orientation such that upDirection is shown in the up direction in
 * the window
 *
 * The default is north up, or upDirection=0. upDirection=90° means east up.
 *
 * @param upDirection, in radians
 */
void FlarmMapWidget::setOrientation (const Angle &upDirection)
{
	transform=QTransform ();
	transform.rotateRadians (upDirection.toRadians ());

	redrawStaticData ();
	redrawFlarmData ();
}


// *********************
// ** Flarm list view **
// *********************

// FIXME this should also set the model for the list, but create the
// FlarmMapWidget first
void FlarmMapWidget::setFlarmList (FlarmList *list)
{
	assert (!this->flarmList); // FIXME allow changing the model, disconnect the signals

	this->flarmList=list;

	connect (list, SIGNAL (destroyed()), this, SLOT(flarmListDestroyed ()));
	connect (list, SIGNAL (dataChanged (QModelIndex, QModelIndex)), this, SLOT (flarmListDataChanged (QModelIndex, QModelIndex)));
	connect (list, SIGNAL (rowsInserted (QModelIndex, int, int)), this, SLOT (flarmListRowsInserted (QModelIndex, int, int)));
	connect (list, SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),this, SLOT (flarmListRowsAboutToBeRemoved (QModelIndex, int, int)));
	connect (list, SIGNAL (modelReset ()), this, SLOT (flarmListReset ()));
	connect (list, SIGNAL (layoutChanged ()), this, SLOT (flarmListLayoutChanged ()));

	redrawFlarmData ();
}

void FlarmMapWidget::flarmListDestroyed ()
{
	this->flarmList=NULL;
}

void FlarmMapWidget::flarmListDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	// FIXME don't redraw all
	redrawFlarmData ();
}

void FlarmMapWidget::flarmListRowsInserted (const QModelIndex &parent, int start, int end)
{
	// FIXME don't redraw all
	redrawFlarmData ();
}

void FlarmMapWidget::flarmListRowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
{
	// FIXME don't redraw all
	redrawFlarmData ();
}

void FlarmMapWidget::flarmListReset ()
{
	redrawFlarmData ();
}

void FlarmMapWidget::flarmListLayoutChanged ()
{
	redrawFlarmData ();
}
