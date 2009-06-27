#ifndef _MimeHeader_h
#define _MimeHeader_h

/*
 * MimeHeader
 * martin
 * 2005-01-25
 */

#include <iostream>

#include "src/web/Argument.h"

class MimeHeader
{
	public:
		static const QString text_name_content_type;
		static const QString text_name_content_disposition;

		MimeHeader (const QString &text, const QString &_name="");

		QString name;
		QString value;
		ArgumentList args;

		static void test ();
		static void test (const QString &text, const QString &_name="");
};

std::ostream &operator<< (std::ostream &s, const MimeHeader &mh);

#endif

