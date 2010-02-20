#ifndef _SkException_h
#define _SkException_h

// TODO reduce dependencies
#include <QString>

class SkException
{
	public:
		SkException () throw () {};
		virtual ~SkException () throw () {};
		virtual QString description () const=0;
		virtual QString description (bool verbose) const { (void)verbose; return description (); }
};


#endif

