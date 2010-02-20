#ifndef _SkException_h
#define _SkException_h

#include <QString>

class SkException
{
	public:
		SkException () {};
		virtual ~SkException () {};

		virtual QString description () const=0;
};

#endif

