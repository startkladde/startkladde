#ifndef _color_h
#define _color_h

#include <QColor>

// TODO reduce dependencies
#include "src/model/Flight.h"

QColor flug_farbe (Flight::Mode, bool, bool, bool, bool);

QColor interpol (float position, const QColor &color0, const QColor &color1);
QColor interpol (float position, const QColor &color0, const QColor &color1, const QColor &color2);

#endif

