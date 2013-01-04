#include "src/graphics/grid.h"

#include <cstddef>

#include "src/util/double.h"

double getDecimalGridSize (double minimum)
{
	// Express the minimum value in as m*10^e degrees
	double m, e;
	toScientific (minimum, &m, &e, NULL);

	// Increase the mantissa to the next higher value from 2, 5 and 10
	if      (m<= 2) m=2;
	else if (m<= 5) m=5;
	else            m=10;

	// Convert the value back to a regular value
	return fromScientific (m, e, true);
}

double getAngleGridSize_min (double minimum_min)
{
	// For grid step sizes, the permissible values are a bit peculiar due to the
	// base 60 system: for values less than 1 degree, we allow 1, 2, 5, 10 and
	// 30 minutes. For values larger than 1 degree, we allow the usual decimal
	// values, 1*10^e, 2*10^e and 5*10^e.

	if      (minimum_min<= 1) return  1; //  1 minute
	else if (minimum_min<= 2) return  2; //  2 minutes
	else if (minimum_min<= 5) return  5; //  5 minutes
	else if (minimum_min<=10) return 10; // 10 minutes
	else if (minimum_min<=30) return 30; // 30 minutes (0.5°)
	else if (minimum_min<=60) return 60; //  1 degree (1 nautical mile)
	else return 60*getDecimalGridSize (minimum_min/60);
}
