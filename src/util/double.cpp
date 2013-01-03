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


void toScientific (double value, double *mantissa, double *exponent, bool *sign)
{
	bool s=true;
	if (value<0)
	{
		s=false;
		value=-value;
	}

	double e=floor (log (value)/log (10));
	double m=value/pow (10, e);

	if (mantissa) (*mantissa)=m;
	if (exponent) (*exponent)=e;
	if (sign)     (*sign    )=s;
}

double fromScientific (double mantissa, double exponent, bool sign)
{
	if (sign)
		return mantissa*pow (10, exponent);
	else
		return -mantissa*pow (10, exponent);
}


// Regular modulus arithmetic:
//   Given y = ax + b, with b<x (I)
//     Then a = y div x        = floor (y/x)
//     and  b = y mod x = y-ax = y - floor (y/x)*x
//
// Logarithmic modulus arithmetic:
//   Given y = x^a * b, with b<x (II)
//     Then a = y logDiv x
//     and  b = y logMod x
//   Taking the logarithm of (II):
//     ln(y) = a*ln(x) + ln(b), with ln(b)<ln(x)
//   Comparing with (I):
//     a      = floor (ln(y)/ln(x))
//   Substituting into (II), or comparing with (I) and exponentiating:
//     b = y/(x^a)

//double logDiv (double y, double x)
//{
//	return floor (log (y)/log (x));
//}
//
//double logMod (double y, double x)
//{
//	double ld=logDiv (y, x);
//	return y/pow (x, ld);
//}
