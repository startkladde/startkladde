#ifndef SKEXCEPTION_H_
#define SKEXCEPTION_H_

#include <QString>

class SkException
{
	public:
		SkException () {};
		virtual ~SkException () {};

		virtual QString description () const=0;
};

#endif

