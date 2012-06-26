#ifndef PFLAASENTENCE_H_
#define PFLAASENTENCE_H_

#include <QString>

class PflaaSentence
{
	public:
		PflaaSentence (const QString &sentence);
		virtual ~PflaaSentence ();

		QString sentence;
		bool isValid;

		int relativeNorth;    // true in meters
		int relativeEast;     // true in meters
		int relativeVertical; // above in meters
		QString flarmId;
		int groundSpeed;      // in m/s
		double climbRate;     // in m/s
};

#endif
