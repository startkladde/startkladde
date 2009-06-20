#ifndef http_document_h
#define http_document_h

/*
 * http_document
 * martin
 * 2005-01-01
 */

#include <iostream>
#include <string>
#include <sstream>
#include "html_document.h"
#include "accessor.h"

using namespace std;

class http_document
{
	public:
		static const string mime_type_pdf;
		static const string mime_type_plaintext;
		static const string mime_type_csv;
		static const string mime_type_form_urlencoded;
		static const string mime_type_multipart_form_data;

		http_document ();
		void output (const string &document, const string &mime_type, const string &charset="ISO-8859-1"); 
		void output (const html_document &document); 
		void output_redirect (string target);
		void do_output (ostream &stream=cout);

		string content_disposition;
		string content_filename;
		string content_description;

		bool no_header;

	protected:
		void output_header (const string &mime_type, const string &charset);
		ostringstream output_buffer;
};


#endif

