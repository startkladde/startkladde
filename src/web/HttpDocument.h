#ifndef _HttpDocument_h
#define _HttpDocument_h

/*
 * HttpDocument
 * martin
 * 2005-01-01
 */

#include <iostream>
#include <QString>
#include <sstream>

#include "src/accessor.h"
#include "src/web/HtmlDocument.h"

class HttpDocument
{
	public:
		static const QString mime_type_pdf;
		static const QString mime_type_plaintext;
		static const QString mime_type_csv;
		static const QString mime_type_form_urlencoded;
		static const QString mime_type_multipart_form_data;

		HttpDocument ();
		void output (const QString &document, const QString &mime_type, const QString &charset="ISO-8859-1");
		void output (const HtmlDocument &document);
		void output_redirect (QString target);
		void do_output (std::ostream &stream=std::cout);

		QString content_disposition;
		QString content_filename;
		QString content_description;

		bool no_header;

	protected:
		void output_header (const QString &mime_type, const QString &charset);
		std::ostringstream output_buffer;
};


#endif

