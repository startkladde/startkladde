#include "src/util/double.h"

#include <cmath>

/**
 * Like standard floor, but rounds down to whole multiples of unit instead of
 * whole numbers
 *
 * More precisely, this function returns the largest value n*unit that is not
 * larger than value, where n is a whole number.
 */
double floor (double value, double unit)
{
	return floor (value/unit)*unit;
}
