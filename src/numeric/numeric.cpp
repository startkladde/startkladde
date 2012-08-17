#include <src/numeric/numeric.h>

#include <cmath>

double numeric::roundToPlaces (double value, int places)
{
	double decimalFactor=pow (10, places);
	return round (value*decimalFactor)/decimalFactor;
}
