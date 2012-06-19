#include <cmath>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <QtCore/QSettings>
#include "FlarmMap.h"
#include "FlarmHandler.h"

FlarmMap::FlarmMap (QWidget* parent) 
: QDialog (parent)
{
  setupUi (this);
  qwtPlot->setAxisScale (QwtPlot::yLeft, -2000.0, 2000.0);
  qwtPlot->setAxisScale (QwtPlot::xBottom, -2000.0, 2000.0);
  QwtPlotGrid* grid = new QwtPlotGrid();
  QwtPlotPanner* panner = new QwtPlotPanner (qwtPlot->canvas());
  panner->setAxisEnabled(QwtPlot::yLeft, true);
  panner->setAxisEnabled(QwtPlot::xBottom, true);
  panner->setMouseButton(Qt::LeftButton);
  panner->setEnabled (true);

  QwtPlotMagnifier* magnifier = new QwtPlotMagnifier(qwtPlot->canvas());
  magnifier->setMouseButton(Qt::MidButton);
  magnifier->setAxisEnabled(QwtPlot::yRight, true);
  magnifier->setAxisEnabled(QwtPlot::xBottom, true);
  // Default is 0.95. We want a factor near to 1.0 (no magnifying at all)
  magnifier->setMouseFactor (0.995);
  magnifier->setEnabled (true);

  connect (buttonClose, SIGNAL (clicked()), this, SLOT (close()));
  connect (toggleOrientation, SIGNAL(toggled(bool)), this, SLOT (orientationChanged(bool)));
  
  FlarmHandler* flarmHandler = FlarmHandler::getInstance();
  connect (flarmHandler, SIGNAL(homePosition(const QPointF&)),this, SLOT(drawAirfield(const QPointF&)));
  connect (flarmHandler, SIGNAL(statusChanged()), this, SLOT(refreshFlarm()));
        
  toggleOrientation->setIcon (QApplication::style()->standardIcon(QStyle::SP_ArrowUp));
  toggleOrientation->setToolTip (QString::fromUtf8 ("Karte nach Norden ausrichten"));
  northUp = -1.0;

  /*
  QMatrix matrix;
  matrix = matrix.translate (1000, 1000);
  QBrush brush (lageplan_xpm);
  brush.setMatrix (matrix);
  
  qwtPlot->setCanvasBackground (brush);
  */

  grid->attach (qwtPlot);
  data = NULL;
  //QTimer::singleShot (0, this, SLOT (storeVectors()));

  // Instead of reading the vectors from the configuration, you can set example
  // vectors by enabling the call to setExampleVectors instead of readVectors.
  // You can also store the example vectors by enabling the call to
  // storeVectors. After that, you can disable both calls and have the vectors
  // read from the database.
  //setExampleVectors ();
  //storeVectors ();
  readVectors();
}

FlarmMap::~FlarmMap () {
} 

void FlarmMap::setExampleVectors ()
{
	airfieldVector.clear ();
	const double p1lat = 52.9430789;
	const double p1lon = 12.7896215;
	const double p2lat = 52.94111;
	const double p2lon = 12.7889;
	const double p3lat = 52.9428;
	const double p3lon = 12.7703;
	const double p4lat = 52.9444;
	const double p4lon = 12.7706;
	airfieldVector
		<< QPointF (p1lon, p1lat)
		<< QPointF (p2lon, p2lat)
		<< QPointF (p3lon, p3lat)
		<< QPointF (p4lon, p4lat);

	patternVector.clear ();
	patternVector
		<< QPointF (12.789271548674964, 52.94212392967359 )
		<< QPointF (12.80044023244796 , 52.94122277500129 )
		<< QPointF (12.801773857118498, 52.941120848710995)
		<< QPointF (12.80262524603299 , 52.941320693327995)
		<< QPointF (12.803246042023268, 52.94174595841085 )
		<< QPointF (12.804757118515505, 52.947346447719795)
		<< QPointF (12.804512599999999, 52.94817706138535 )
		<< QPointF (12.803663546254134, 52.94857755117815 )
		<< QPointF (12.80248103315735 , 52.94881246640197 )
		<< QPointF (12.758622509608768, 52.952181313142034)
		<< QPointF (12.757164583633068, 52.95218020704071 )
		<< QPointF (12.75560434296937 , 52.95171226097781 )
		<< QPointF (12.754953987817434, 52.95107188923083 )
		<< QPointF (12.754016666274461, 52.94632345292743 )
		<< QPointF (12.754175670311906, 52.94546260279531 )
		<< QPointF (12.754900437234875, 52.944926213872   )
		<< QPointF (12.756229743111295, 52.94472066496655 )
		<< QPointF (12.770433806814804, 52.94351364365588 );
}

/**
  * Not used, only to initialize the table wich is stored in settings
  */
void FlarmMap::storeVectors () {
  QSettings settings ("startkladde", "startkladde");
  settings.beginGroup ("vectors");
  QVariantList variant;
  foreach (QPointF point, airfieldVector)
    variant << point.x() << point.y();
  settings.setValue ("airfield", variant);
  variant.clear();
  foreach (QPointF point, patternVector)
    variant << point.x() << point.y();
  settings.setValue ("pattern", variant);
  settings.endGroup ();
}

void FlarmMap::readVectors () {
  //qDebug () << "FlarmMap::readVectors: " << airfieldVector << endl;
  QSettings settings ("startkladde", "startkladde");
  settings.beginGroup ("vectors");
  QVariantList variantlist = settings.value ("airfield").toList();
  airfieldVector.clear();
  while (!variantlist.isEmpty()) {
    double x = variantlist.takeFirst().toDouble();
    double y = variantlist.takeFirst().toDouble();
    airfieldVector << QPointF (x, y);
  }

  variantlist = settings.value ("pattern").toList();
  patternVector.clear();
  while (!variantlist.isEmpty()) {
    double x = variantlist.takeFirst().toDouble();
    double y = variantlist.takeFirst().toDouble();
    patternVector << QPointF (x, y);
  }
  settings.endGroup();
  //qDebug () << "FlarmMap::readVectors new: " << airfieldVector << endl;
}

void FlarmMap::drawAirfield (const QPointF& home) {
  // only draw when moved more then 1 arc second
  if (abs (home.x() - old_home.x()) > 0.00027 || abs (home.y() - old_home.y()) > 0.00027) {
    old_home = home;
  }
  else
    return;
  
  #define PI 3.1415926536
  QVector <QPointF> airfieldDist;
  //invert all values if northUp is -1
  foreach (QPointF point, airfieldVector) {
    QPointF reldeg (point.x()-home.x(), point.y()-home.y());
    airfieldDist << QPointF (northUp * (reldeg.x() * (40000000.0 / 360.0)) * cos (point.y() * (PI / 180.0)),
                        northUp * reldeg.y() * (40000000.0 / 360.0));
  }
  // append the first element to close the rectangle
  airfieldDist << airfieldDist [0];

  QVector <QPointF> patternDist;
  foreach (QPointF point, patternVector) {
    QPointF reldeg (point.x()-home.x(), point.y()-home.y());
    patternDist << QPointF (northUp * (reldeg.x() * (40000000.0 / 360.0)) * cos (point.y() * (PI / 180.0)),
                        northUp * reldeg.y() * (40000000.0 / 360.0));
  }
  
  qwtPlot->detachItems (QwtPlotItem::Rtti_PlotCurve);

  QPen pen;
  pen.setWidth (2);

  QwtPlotCurve* curve1 = new QwtPlotCurve ("airfield");
  QwtPlotCurve* curve2 = new QwtPlotCurve ("pattern");
  QwtPointSeriesData* data1  = new QwtPointSeriesData (airfieldDist);
  QwtPointSeriesData* data2  = new QwtPointSeriesData (patternDist);
  curve1->setData (data1);
  curve2->setData (data2);
  curve1->setPen (pen);
  curve1->attach (qwtPlot);
  curve2->attach (qwtPlot);
}

void FlarmMap::refreshFlarm() {
  qwtPlot->detachItems (QwtPlotItem::Rtti_PlotMarker);

  QMap<QString,FlarmRecord*>* regMap = FlarmHandler::getInstance()->getRegMap();
  foreach (FlarmRecord* record, *regMap) {
    //qDebug () << i.value()->reg << "; " << i.value()->alt << "; " << i.value()->speed << endl;

    FlarmRecord::flarmState state = record->getState();
    // we will not show far away planes or planes on ground
    if (state == FlarmRecord::stateStarting || state == FlarmRecord::stateFlying || state == FlarmRecord::stateLanding ) {
      QwtPlotMarker* marker = new QwtPlotMarker ();
      QwtText text((record->registration + "\n%1/%2/%3").arg(record->getAlt()).arg(record->getSpeed()).arg(record->getClimb(), 0, 'f', 1));
      //if (record->getState() == FlarmRecord::stateFlyingFar) {
      //  text.setColor (Qt::white);
      //  text.setBackgroundBrush (QBrush(Qt::red));
      //}
      if (record->getClimb() > 0.0)
        text.setBackgroundBrush (QBrush(Qt::green));
      else
        text.setBackgroundBrush (QBrush(Qt::yellow));
      marker->setLabel (text);
      // south is top if northUp is -1
      marker->setXValue (northUp * record->east);
      marker->setYValue (northUp * record->north);
      marker->attach (qwtPlot);
    }
  }  

  QwtPlotMarker* marker = new QwtPlotMarker ();
  QwtText text ("Start");
  //text.setPaintAttribute (QwtText::PaintBackground, true);
  text.setBackgroundBrush (QBrush(Qt::red));
  marker->setLabel (text);
  marker->setXValue (0);
  marker->setYValue (0);
  marker->attach (qwtPlot);

  qwtPlot->replot();
}

void FlarmMap::orientationChanged (bool up) {
  QStyle* style = QApplication::style();
  if (up) {
    northUp = 1.0;
    toggleOrientation->setIcon (style->standardIcon(QStyle::SP_ArrowDown));
    toggleOrientation->setToolTip (QString::fromUtf8 ("Karte nach Süden ausrichten"));
  }
  else {
    northUp = -1.0;
    toggleOrientation->setIcon (style->standardIcon(QStyle::SP_ArrowUp));
    toggleOrientation->setToolTip (QString::fromUtf8 ("Karte nach Norden ausrichten"));
  }
  QPointF home = old_home;
  // force to redraw
  old_home = QPointF ();
  drawAirfield (home);
  refreshFlarm ();
}
