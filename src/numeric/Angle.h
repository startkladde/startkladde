#ifndef ANGLE_H_
#define ANGLE_H_

class Angle
{
	public:
		Angle ();
		Angle (double value);
		virtual ~Angle ();

		double getValue () const;
		bool isValid () const;

	private:
		double value;
		bool valid;

};

#endif
