#ifndef http_document_h
#define http_document_h

/*
 * http_document
 * martin
 * 2005-01-01
 */

#include <iostream>
#include <QString>
#include <sstream>

#include "src/accessor.h"
#include "src/web/html_document.h"

class http_document
{
	public:
		static const QString mime_type_pdf;
		static const QString mime_type_plaintext;
		static const QString mime_type_csv;
		static const QString mime_type_form_urlencoded;
		static const QString mime_type_multipart_form_data;

		http_document ();
		void output (const QString &document, const QString &mime_type, const QString &charset="ISO-8859-1");
		void output (const html_document &document);
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

