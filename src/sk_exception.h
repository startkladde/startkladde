#ifndef sk_exception_h
#define sk_exception_h

#include <string>

using namespace std;

class sk_exception
{
	public:
		sk_exception () throw () {};
		virtual ~sk_exception () throw () {};
		virtual string description () const=0;
		virtual string description (bool verbose) const { return description (); }
};


#endif

