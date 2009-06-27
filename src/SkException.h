#ifndef _SkException_h
#define _SkException_h

#include <QString>

class SkException
{
	public:
		SkException () throw () {};
		virtual ~SkException () throw () {};
		virtual QString description () const=0;
		virtual QString description (bool verbose) const { return description (); }
};


#endif

