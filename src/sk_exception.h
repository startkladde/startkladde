#ifndef sk_exception_h
#define sk_exception_h

#include <QString>

class sk_exception
{
	public:
		sk_exception () throw () {};
		virtual ~sk_exception () throw () {};
		virtual QString description () const=0;
		virtual QString description (bool verbose) const { return description (); }
};


#endif

