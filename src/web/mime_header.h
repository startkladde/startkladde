#ifndef content_type_h
#define content_type_h

/*
 * mime_header
 * martin
 * 2005-01-25
 */

using namespace std;

#include <iostream>

#include "src/web/argument.h"

class mime_header
{
	public:
		static const string text_name_content_type;
		static const string text_name_content_disposition;

		mime_header (const string &text, const string &_name="");

		string name;
		string value;
		argument_list args;

		static void test ();
		static void test (const string &text, const string &_name="");
};

ostream &operator<< (ostream &s, const mime_header &mh);

#endif

