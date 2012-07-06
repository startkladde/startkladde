#include <src/flarm/FlarmMapWidget.h>

#include <cassert>
#include <iostream>

#include <QModelIndex>

#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>

#include "src/flarm/FlarmRecord.h"
#include "src/numeric/Velocity.h"
#include "src/numeric/GeoPosition.h"
#include "src/flarm/FlarmList.h"
#include "src/util/qHash.h"
#include "src/i18n/notr.h"


// ******************
// ** Construction **
// ******************

FlarmMapWidget::FlarmMapWidget (QWidget *parent): QwtPlot (parent),
	climbColor   (  0, 255, 0, 127),
	descentColor (255, 255, 0, 127),
	model (NULL)
{
	// Setup the axes
	// FIXME consider aspect ratio when scaling
	setAxisScale (QwtPlot::yLeft, -2000.0, 2000.0);
	setAxisScale (QwtPlot::xBottom, -2000.0, 2000.0);

	// Setup the panner
	QwtPlotPanner* panner = new QwtPlotPanner (canvas ());
	panner->setAxisEnabled (QwtPlot::yLeft, true);
	panner->setAxisEnabled (QwtPlot::xBottom, true);
	panner->setMouseButton (Qt::LeftButton);
	panner->setEnabled (true);

	// Setup the magnifier
	QwtPlotMagnifier* magnifier = new QwtPlotMagnifier (canvas ());
	magnifier->setMouseButton (Qt::MidButton);
	magnifier->setAxisEnabled (QwtPlot::yRight, true);
	magnifier->setAxisEnabled (QwtPlot::xBottom, true);
	// Positive value - mouse wheel down (back) means zooming out. This is the
	// convention that many other applications, including Firefox and Gimp, use.
	magnifier->setMouseFactor (1.05);
	magnifier->setWheelFactor (1.1);
	magnifier->setEnabled (true);

	// Add the grid
	QwtPlotGrid* grid = new QwtPlotGrid ();
	grid->attach (this);

	// Add static markers
	addStaticMarker ("Start", QColor (255, 0, 0, 127), QPointF (0, 0));

	updateStaticCurves ();
	refreshFlarmData ();
	replot ();
}

FlarmMapWidget::~FlarmMapWidget ()
{
}


// ***********
// ** Model **
// ***********

/**
 * Sets the model to use for getting the Flarm data
 *
 * If a model was set before, it is replaced by the new model. If the new model
 * is the same as before, nothing is changed.
 *
 * This method calls replot().
 *
 * @param model the new model. This view does not take ownership of the model.
 */
void FlarmMapWidget::setModel (FlarmList *model)
{
	if (model==this->model)
		return;

	// Note that we can ignore layoutChanged because we don't consider the
	// order of the entries anyway, and we refer to entries by Flarm ID, not
	// index.

	if (this->model)
	{
		disconnect (this->model, SIGNAL (destroyed()), this, SLOT (modelDestroyed ()));
		disconnect (this->model, SIGNAL (dataChanged (QModelIndex, QModelIndex)), this, SLOT (dataChanged (QModelIndex, QModelIndex)));
		disconnect (this->model, SIGNAL (rowsInserted (QModelIndex, int, int)), this, SLOT (rowsInserted (QModelIndex, int, int)));
		disconnect (this->model, SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),this, SLOT (rowsAboutToBeRemoved (QModelIndex, int, int)));
		disconnect (this->model, SIGNAL (modelReset ()), this, SLOT (modelReset ()));
	}

	this->model=model;

	if (model)
	{
		connect (this->model, SIGNAL (destroyed()), this, SLOT (modelDestroyed ()));
		connect (this->model, SIGNAL (dataChanged (QModelIndex, QModelIndex)), this, SLOT (dataChanged (QModelIndex, QModelIndex)));
		connect (this->model, SIGNAL (rowsInserted (QModelIndex, int, int)), this, SLOT (rowsInserted (QModelIndex, int, int)));
		connect (this->model, SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),this, SLOT (rowsAboutToBeRemoved (QModelIndex, int, int)));
		connect (this->model, SIGNAL (modelReset ()), this, SLOT (modelReset ()));
	}

	refreshFlarmData ();
	replot ();
}


// **********
// ** View **
// **********

/**
 * Sets the orientation such that the given compass direction is shown in the up
 * direction in the window
 *
 * The default is north up, or upDirection=0. upDirection=90° means east up.
 *
 * This method calls replot().
 */
void FlarmMapWidget::setOrientation (const Angle &upDirection)
{
	transform=QTransform ();
	transform.rotateRadians (upDirection.toRadians ());

	updateStaticCurves ();
	refreshFlarmData ();
	replot ();
}

/**
 * Updates the static curves for the new position
 *
 * Call this method when the own position changes.
 *
 * This method calls replot().
 */
void FlarmMapWidget::ownPositionChanged (const GeoPosition &ownPosition)
{
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
		updateStaticCurves ();
		replot ();
	}
}



// *****************
// ** Static data **
// *****************

/**
 * Adds a static curve to the list of static curves
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 *
 * @param name a name for the curve, e. g. "airfield"
 * @param points a vector of points that make up the curve. The curve will not
 *               be closed automatically.
 * @param pen the pen to use for drawing the curve
 */
void FlarmMapWidget::addStaticCurve (const QString &name, const QVector<GeoPosition> &points, QPen pen)
{
	StaticCurve curve;

	curve.name=name;

	curve.points=points;

	curve.data=new QwtPointSeriesData (); // Deleted by curve (FIXME really?)

	curve.curve=new QwtPlotCurve (name); // Deleted by QwtPlot
	curve.curve->setRenderHint (QwtPlotItem::RenderAntialiased);
	curve.curve->setPen (pen);
	curve.curve->setData (curve.data);
	curve.curve->attach (this);

	staticCurves.append (curve);
}

/**
 * Adds a static marker relative to the own position
 *
 * This method does not call replot(). You have to call it yourself to update
 * the display.
 *
 * @param text the text to display on the label
 * @param color the background color of the label
 * @param point the position, in meters east and north of the own position
 */
// FIXME add a message for setting absolute positions
void FlarmMapWidget::addStaticMarker (const QString &text, const QColor &color, const QPointF &point)
{
	QwtText qwtText (text);
	qwtText.setBackgroundBrush (QBrush (color));

	QwtPlotMarker *marker=new QwtPlotMarker (); // Deleted by plot
	marker->setLabel (qwtText);
	marker->setValue (point);
	marker->attach (this);
}

void FlarmMapWidget::updateStaticCurves ()
{
	bool valid=ownPosition.isValid ();
	foreach (const StaticCurve &curve, staticCurves)
	{
		if (valid)
		{
			QPolygonF polygon (GeoPosition::relativePositionTo (curve.points, ownPosition));
			curve.data->setSamples (transform.map (polygon));
		}
		curve.curve->setVisible (valid);
	}
}


// ***********************************
// ** Flarm data individual updates **
// ***********************************

void FlarmMapWidget::updateMarkerMinimal (QwtPlotMarker *marker, const FlarmRecord &record)
{
	marker->setVisible (true);

	Q_UNUSED (record);

	// Symbol: small blue cross
	QwtSymbol *symbol=new QwtSymbol (); // Will be deleted by the marker
	symbol->setStyle (QwtSymbol::Cross);
	symbol->setSize (8);
	symbol->setPen (QPen (Qt::blue));
	marker->setSymbol (symbol);

	// Label: none
	marker->setLabel (QwtText ());
}

void FlarmMapWidget::updateMarkerVerbose (QwtPlotMarker *marker, const FlarmRecord &record)
{
	marker->setVisible (true);

	// Symbol: none
	marker->setSymbol (NULL);

	// Label: verbose text
	QwtText text (qnotr ("%1\n%2/%3/%4")
		.arg (record.getRegistration ())
		.arg (record.getRelativeAltitude ())
		.arg (record.getGroundSpeed () / Velocity::km_h)
		.arg (record.getClimbRate (), 0, 'f', 1));

	if (record.getClimbRate () > 0.0)
		text.setBackgroundBrush (QBrush (climbColor));
	else
		text.setBackgroundBrush (QBrush (descentColor));

	marker->setLabel (text);
}

void FlarmMapWidget::updateTrail (QwtPlotCurve *curve, const FlarmRecord &record)
{
	curve->setVisible (true);

	// Prepare data. The data will be deleted by the curve.
	QPolygonF polygon (record.getPreviousRelativePositions ().toVector ());
	// Will be deleted by the curve
	QwtPointSeriesData *data = new QwtPointSeriesData (transform.map (polygon));
	curve->setData (data);
}


// ****************
// ** Flarm data **
// ****************

void FlarmMapWidget::addFlarmData (const FlarmRecord &record)
{
	// Create, attach and store the marker
	QwtPlotMarker *marker = new QwtPlotMarker ();
	marker->attach (this);
	flarmMarkers.insert (record.getFlarmId (), marker);

	// Create, attach and store the curve
	QwtPlotCurve* curve = new QwtPlotCurve ("history");
	curve->attach (this);
	flarmCurves.insert (record.getFlarmId (), curve);

	// Setup the curve
	QPen pen;
	pen.setWidth (2);
	curve->setPen (pen);
	curve->setRenderHint (QwtPlotItem::RenderAntialiased);

	// Update the data (marker and trail)
	updateFlarmData (record);
}

void FlarmMapWidget::updateFlarmData (const FlarmRecord &record)
{
	QwtPlotMarker *marker=flarmMarkers.value (record.getFlarmId (), NULL);
	QwtPlotCurve  *curve =flarmCurves .value (record.getFlarmId (), NULL);

	// Always set the position, even if the marker is not visible
	marker->setValue (transform.map (record.getRelativePosition ()));

	switch (record.getState ())
	{
		case FlarmRecord::stateStarting:
		case FlarmRecord::stateFlying:
		case FlarmRecord::stateLanding:
			// Verbose marker, trail
			updateMarkerVerbose (marker, record);
			updateTrail (curve, record);
			break;
		case FlarmRecord::stateOnGround:
			// Minimal marker, no trail
			updateMarkerMinimal (marker, record);
			curve->setVisible (false);
			break;
		case FlarmRecord::stateUnknown:
		case FlarmRecord::stateFlyingFar:
			// No marker, no trail
			marker->setVisible (false);
			curve->setVisible (false);
			break;
		// no default
	}
}

/**
 * You will have to call replot yourself.
 *
 * @param flarmId
 */
void FlarmMapWidget::removeFlarmData (const FlarmRecord &record)
{
	// Items will be detached automatically on deletion.
	QString flarmId=record.getFlarmId ();
	removeAndDeleteIfExists (flarmMarkers, flarmId);
	removeAndDeleteIfExists (flarmCurves , flarmId);
}

void FlarmMapWidget::refreshFlarmData ()
{
	// Items will be detached automatically on deletion.
	clearAndDelete (flarmMarkers);
	clearAndDelete (flarmCurves);

	// Only draw if we have a model
	if (model)
	{
		for (int i=0, n=model->size (); i<n; ++i)
		{
			const FlarmRecord &record=model->at (i);
			addFlarmData (record);
		}
	}
}


// *****************
// ** Model slots **
// *****************

void FlarmMapWidget::rowsInserted (const QModelIndex &parent, int start, int end)
{
	Q_UNUSED (parent);

	if (model)
		for (int i=start; i<=end; ++i)
			addFlarmData (model->at (i));

	replot ();
}

void FlarmMapWidget::dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	if (model)
		for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
			updateFlarmData (model->at (i));

	replot ();
}

void FlarmMapWidget::rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
{
	Q_UNUSED (parent);

	if (model)
		for (int i=start; i<=end; ++i)
			removeFlarmData (model->at (i));

	replot ();
}

void FlarmMapWidget::modelReset ()
{
	refreshFlarmData ();
	replot ();
}

void FlarmMapWidget::modelDestroyed ()
{
	setModel (NULL);
}

