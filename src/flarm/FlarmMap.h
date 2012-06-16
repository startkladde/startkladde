#ifndef FLARM_MAP_H
#define FLARM_MAP_H
#include <QtGui/QWidget>
#include <QtGui/QDialog>
#include <qwt_series_data.h>
#include "ui_FlarmMap.h"

class FlarmMap: public QDialog, private Ui_Map {
Q_OBJECT

public:
  FlarmMap (QWidget*);
  ~FlarmMap ();

private slots:
  void refreshFlarm ();
  void storeVectors();
  void readVectors();
  void orientationChanged (bool);
  void drawAirfield (const QPointF&);

private:
  QwtPointSeriesData* data;
  QPointF old_home;
  QVector<QPointF> airfieldVector;
  QVector<QPointF> patternVector;
  double northUp;
};

#endif
