#include "src/flarm/Kml.h"

#include <cmath>

// ***********
// ** Style **
// ***********

Kml::Style::Style ():
	lineWidth (1)
{
}

QPen Kml::Style::linePen ()
{
	QPen pen (lineColor);
	pen.setWidth (round (lineWidth));
	return pen;
}


// ***************
// ** Functions **
// ***************

QColor Kml::parseColor (const QString color)
{
	// It seems like white, fully opaque, may be stored as an empty string.
	if (color.isEmpty ())
		return Qt::white;

	if (color.length ()!=8)
		return QColor ();

	// aabbggrr
	int a=color.mid (0, 2).toInt (NULL, 16);
	int b=color.mid (2, 2).toInt (NULL, 16);
	int g=color.mid (4, 2).toInt (NULL, 16);
	int r=color.mid (6, 2).toInt (NULL, 16);

	return QColor (r, g, b, a);
}

