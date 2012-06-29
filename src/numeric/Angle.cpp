#include "Angle.h"

#include <cmath>

/**
 * Constructs an invalid angle
 */
Angle::Angle ():
	value (0), valid (false)
{
}

/**
 * Constructs a valid angle with the given value
 *
 * This method is private.
 *
 * @param value the angle value, in radians
 */
Angle::Angle (double value):
	value (value), valid (true)
{
}

/**
 * Constructs an angle with the given value and validity
 *
 * This method is private.
 *
 * @param value the value, in radians
 * @param valid true for a valid angle, false else
 */
Angle::Angle (double value, bool valid):
	value (value), valid (valid)
{
}

Angle::~Angle ()
{
}

/**
 * Constructs an angle with a given value in radians
 *
 * @param radians the value, in radians
 * @return an Angle with the given value
 */
Angle Angle::fromRadians (double radians)
{
	return Angle (radians);
}

/**
 * Constructs an angle with a given value in degrees
 *
 * @param radians the value, in degrees
 * @return an Angle with the given value
 */
Angle Angle::fromDegrees (double degrees)
{
	return Angle (degrees/360*(2*M_PI));
}

/**
 * Returns the status of the valid flag
 */
bool Angle::isValid () const
{
	return valid;
}

/**
 * Returns the value, in radians
 *
 * If the Angle is invalid, the result is undefined.
 */
double Angle::toRadians () const
{
	return value;
}

/**
 * Returns the value, in degrees
 *
 * If the Angle is invalid, the result is undefined.
 */
double Angle::toDegrees () const
{
	return value/(2*M_PI)*360;
}

/**
 * Returns the sum of this and other
 */
Angle Angle::operator+ (const Angle &other) const
{
	if (valid && other.valid)
		return Angle (value+other.value);
	else
		return Angle ();
}

/**
 * Returns the difference between this and other
 */
Angle Angle::operator- (const Angle &other) const
{
	if (valid && other.valid)
		return Angle (value-other.value);
	else
		return Angle ();
}

/**
 * Returns the result of multiplying this angle with the scalar factor
 */
Angle Angle::operator* (double factor) const
{
	return Angle (value*factor, valid);
}

/**
 * Returns the result of dividing this angle by the scalar factor
 */
Angle Angle::operator/ (double factor) const
{
	return Angle (value/factor, valid);
}

