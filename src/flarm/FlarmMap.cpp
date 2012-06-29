#include "FlarmMap.h"

#include <cmath>

#include <QtCore/QSettings>
#include <QSortFilterProxyModel>

#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_series_data.h>

#include "src/flarm/FlarmHandler.h"
#include "src/flarm/FlarmRecordModel.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/numeric/Angle.h"
#include "src/numeric/Speed.h"
#include "src/numeric/GeoPosition.h"

FlarmMap::FlarmMap (QWidget *parent) :
	SkDialog<Ui::FlarmMapDialog> (parent)
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
	// Default is 0.95. We want a factor near to 1.0 (no magnifying at all)
	magnifier->setMouseFactor (0.995);
	magnifier->setEnabled (true);

	connect (ui.closeButton, SIGNAL (clicked()), this, SLOT (close()));

	FlarmHandler* flarmHandler = FlarmHandler::getInstance ();
	connect (flarmHandler, SIGNAL(homePosition(const GeoPosition &)), this, SLOT(drawAirfield(const GeoPosition &)));
	connect (flarmHandler, SIGNAL(statusChanged()), this, SLOT(refreshFlarm()));

	ui.toggleOrientationButton->setIcon (QApplication::style ()->standardIcon (QStyle::SP_ArrowUp));
	ui.toggleOrientationButton->setToolTip (QString::fromUtf8 ("Karte nach Norden ausrichten"));
	northUp = -1.0;

	grid->attach (ui.qwtPlot);
	data = NULL;
	//QTimer::singleShot (0, this, SLOT (storeVectors()));

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

	QVariantList valueList;
	foreach (const GeoPosition &point, airfieldVector)
		valueList << point.getLongitude ().toDegrees () << point.getLatitude ().toDegrees ();
	settings.setValue ("airfield", valueList);

	valueList.clear ();
	foreach (const GeoPosition &point, patternVector)
	valueList << point.getLongitude ().toDegrees () << point.getLatitude ().toDegrees ();
	settings.setValue ("pattern", valueList);

	settings.endGroup ();
}

void FlarmMap::readVectors ()
{
	QSettings settings ("startkladde", "startkladde");
	settings.beginGroup ("vectors");

	QVariantList valueList = settings.value ("airfield").toList ();
	airfieldVector.clear ();
	while (valueList.size ()>=2)
	{
		double lon = valueList.takeFirst ().toDouble ();
		double lat = valueList.takeFirst ().toDouble ();
		airfieldVector << GeoPosition::fromDegrees (lat, lon);
	}

	valueList = settings.value ("pattern").toList ();
	patternVector.clear ();
	while (valueList.size ()>=2)
	{
		double lon = valueList.takeFirst ().toDouble ();
		double lat = valueList.takeFirst ().toDouble ();
		patternVector << GeoPosition::fromDegrees (lat, lon);
	}

	settings.endGroup ();
}

void FlarmMap::drawAirfield (const GeoPosition &home)
{
	// only draw when moved more then 1 arc second
	// FIXME don't always redraw, remove history curves but not static curves
	// (airfield and traffic circuit)
//	if (abs (home.x () - old_home.x ()) > 0.00027 || abs (home.y () - old_home.y ()) > 0.00027)
//	{
//		old_home = home;
//	}
//	else
//		return;

	//invert all values if northUp is -1

	QVector<QPointF> airfieldDist;
	foreach (const GeoPosition &point, airfieldVector)
		airfieldDist << northUp * point.relativePositionTo (home);
	// append the first element to close the rectangle
	if (!airfieldDist.empty ())
		airfieldDist << airfieldDist[0];

	QVector<QPointF> patternDist;
	foreach (const GeoPosition &point, patternVector)
		patternDist << northUp * point.relativePositionTo (home);

	ui.qwtPlot->detachItems (QwtPlotItem::Rtti_PlotCurve);

	QPen pen;
	pen.setWidth (2);

	QwtPlotCurve* curve1 = new QwtPlotCurve ("airfield");
	QwtPlotCurve* curve2 = new QwtPlotCurve ("pattern");
	QwtPointSeriesData* data1 = new QwtPointSeriesData (airfieldDist);
	QwtPointSeriesData* data2 = new QwtPointSeriesData (patternDist);
	curve1->setData (data1);
	curve2->setData (data2);
	curve1->setPen (pen);
	curve1->attach (ui.qwtPlot);
	curve2->attach (ui.qwtPlot);
}

void FlarmMap::refreshFlarm ()
{
	ui.qwtPlot->detachItems (QwtPlotItem::Rtti_PlotMarker);
	// FIXME cannot detach all curves unless we always recreate the static
	// curves (airfield, traffic circuit)
	// FIXME done in drawAirfield, this sucks
//	qwtPlot->detachItems (QwtPlotItem::Rtti_PlotCurve);

	//  QMap<QString,FlarmRecord*>* regMap = FlarmHandler::getInstance()->getRegMap();
	//  foreach (FlarmRecord* record, *regMap) {
	foreach (FlarmRecord *record, FlarmHandler::getInstance ()->getFlarmRecords ()->getList ())
	{
		//qDebug () << i.value()->reg << "; " << i.value()->alt << "; " << i.value()->speed << endl;

		FlarmRecord::flarmState state = record->getState ();
		// we will not show far away planes or planes on ground
		if (state == FlarmRecord::stateStarting || state == FlarmRecord::stateFlying || state
				== FlarmRecord::stateLanding)
		{
			QwtPlotMarker* marker = new QwtPlotMarker ();
			QwtText text (
					(record->getRegistration () + "\n%1/%2/%3").arg (record->getRelativeAltitude ()).arg (
							record->getGroundSpeed () / Speed::km_h).arg (record->getClimbRate (), 0, 'f', 1));
			//if (record->getState() == FlarmRecord::stateFlyingFar) {
			//  text.setColor (Qt::white);
			//  text.setBackgroundBrush (QBrush(Qt::red));
			//}
			QColor climbColor=Qt::green;
			QColor descentColor=Qt::yellow;

			climbColor.setAlpha (127);
			descentColor.setAlpha (127);

			if (record->getClimbRate () > 0.0)
				text.setBackgroundBrush (QBrush (climbColor));
			else
				text.setBackgroundBrush (QBrush (descentColor));
			marker->setLabel (text);
			// south is top if northUp is -1
			marker->setValue (northUp*record->getRelativePosition ());
			marker->attach (ui.qwtPlot);


			QVector<QPointF> points=record->getPreviousRelativePositions ().toVector ();
			for (int i=0, n=points.size (); i<n; ++i)
				points[i]=northUp*points[i];

			QPen pen;
			pen.setWidth (2);

			// FIXME delete it?
			QwtPlotCurve* curve = new QwtPlotCurve ("history");
			QwtPointSeriesData* data = new QwtPointSeriesData (points);
			curve->setData (data);
			curve->setPen (pen);
			curve->attach (ui.qwtPlot);
		}
	}

	QwtPlotMarker* marker = new QwtPlotMarker ();
	QwtText text ("Start");
	//text.setPaintAttribute (QwtText::PaintBackground, true);
	text.setBackgroundBrush (QBrush (Qt::red));
	marker->setLabel (text);
	marker->setXValue (0);
	marker->setYValue (0);
	marker->attach (ui.qwtPlot);

	ui.qwtPlot->replot ();
}

void FlarmMap::on_toggleOrientationButton_toggled (bool on)
{
	QStyle* style = QApplication::style ();

	if (on)
	{
		northUp = 1.0;
		ui.toggleOrientationButton->setIcon (style->standardIcon (QStyle::SP_ArrowDown));
		ui.toggleOrientationButton->setToolTip (QString::fromUtf8 ("Karte nach Süden ausrichten"));
	}
	else
	{
		northUp = -1.0;
		ui.toggleOrientationButton->setIcon (style->standardIcon (QStyle::SP_ArrowUp));
		ui.toggleOrientationButton->setToolTip (QString::fromUtf8 ("Karte nach Norden ausrichten"));
	}
	// force to redraw
	// FIXME this stinks
	GeoPosition home = oldHome;
	oldHome=GeoPosition ();
	drawAirfield (home);
	refreshFlarm ();
}
