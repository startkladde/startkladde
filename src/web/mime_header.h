#ifndef content_type_h
#define content_type_h

/*
 * mime_header
 * martin
 * 2005-01-25
 */

#include <iostream>

#include "src/web/argument.h"

class mime_header
{
	public:
		static const QString text_name_content_type;
		static const QString text_name_content_disposition;

		mime_header (const QString &text, const QString &_name="");

		QString name;
		QString value;
		argument_list args;

		static void test ();
		static void test (const QString &text, const QString &_name="");
};

std::ostream &operator<< (std::ostream &s, const mime_header &mh);

#endif

