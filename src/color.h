#ifndef _color_h
#define _color_h

#include <QColor>

#include "src/model/Flight.h"

QColor flug_farbe (FlightMode, bool, bool, bool, bool);

QColor interpol (float position, const QColor &color0, const QColor &color1);
QColor interpol (float position, const QColor &color0, const QColor &color1, const QColor &color2);

#endif

