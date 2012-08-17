#ifndef ANGLE_H_
#define ANGLE_H_

class QPointF;

/**
 * FIXME merge with Longitude class
 */
/**
 * A class to store an angle value
 *
 * The angle can be initialized and converted to degrees and radians. An
 * additional flag indicates whether the angle is valid.
 *
 * Internally, the value is stored in radians. This is not visible on the
 * interface, but initializing from and converting to radians will be slightly
 * faster than for degrees.
 */
class Angle
{
	public:
		Angle ();
		virtual ~Angle ();

		static Angle fromRadians (double radians);
		static Angle fromDegrees (double degrees);

		Angle normalized () const;

		bool isValid () const;

		double toRadians () const;
		double toDegrees () const;

		Angle operator+ (const Angle &other) const;
		Angle operator- (const Angle &other) const;
		Angle operator* (double factor) const;
		Angle operator/ (double factor) const;

		bool operator== (const Angle &other) const;
		bool operator!= (const Angle &other) const;
		bool operator<  (const Angle &other) const;
		bool operator<= (const Angle &other) const;
		bool operator>  (const Angle &other) const;
		bool operator>= (const Angle &other) const;

		static Angle min (const Angle &a1, const Angle &a2);
		static Angle max (const Angle &a1, const Angle &a2);

		static Angle atan2 (const double y, const double x);
		static Angle atan2 (const QPointF &point);

		int compassSection (int numSections) const;

	private:
		Angle (double value);
		Angle (double value, bool valid);

		double value;
		bool valid;
};

#endif
