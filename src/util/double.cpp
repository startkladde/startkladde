#include "src/util/double.h"

#include <cmath>

double floor (double value, double unit)
{
	return floor (value/unit)*unit;
}
