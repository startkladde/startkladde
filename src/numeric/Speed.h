#ifndef SPEED_H_
#define SPEED_H_

/**
 * Defines some constants for speed calculations
 *
 * All values are in SI units, that is, m/s.
 *
 * Example:
 *   double speed = 5 * Speed::km_h;
 *   double speed_in_knots = speed / Speed::knot; // 2.70
 */
class Speed
{
	public:
//		Speed ();
//		virtual ~Speed ();

		static const double m_s = 1.0;
		static const double km_h = 1000.0/3600.0;
		static const double knot = 1852.0/3600.0;
};

#endif
