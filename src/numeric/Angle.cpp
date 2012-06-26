#include "Angle.h"

Angle::Angle ():
	value (0), valid (false)
{
}

Angle::Angle (double value):
	value (value), valid (true)
{
}

Angle::~Angle ()
{
}

double Angle::getValue () const
{
	return value;
}

bool Angle::isValid () const
{
	return valid;
}
