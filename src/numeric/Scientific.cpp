#include "src/numeric/Scientific.h"

#include <cmath>

Scientific::Scientific (double mantissa, int exponent, bool sign):
	_mantissa (mantissa), _exponent (exponent), _sign (sign)
{

}

Scientific::Scientific (double value)
{
	setValue (value);
}

Scientific::~Scientific ()
{
}

void Scientific::setValue (double value)
{
	if (value>=0)
	{
		// Positive
		_sign=true;
	}
	else
	{
		// Negative, continue with negated value
		_sign=false;
		value=-value;
	}

	_exponent=floor (log (value)/log (10));
	_mantissa=value/pow (10, _exponent);
}

double Scientific::toValue ()
{
	if (_sign)
		return _mantissa * pow (10, _exponent);
	else
		return -_mantissa * pow (10, _exponent);
}
