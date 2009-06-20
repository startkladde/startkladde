#include "http_document.h"

const string http_document::mime_type_pdf="application/pdf";
const string http_document::mime_type_plaintext="text/plain";
const string http_document::mime_type_csv="text/comma-separated-values";
const string http_document::mime_type_form_urlencoded="application/x-www-form-urlencoded";
const string http_document::mime_type_multipart_form_data="multipart/form-data";


http_document::http_document ()/*{{{*/
{
	no_header=false;
}
/*}}}*/

void http_document::output_header (const string &mime_type, const string &charset)/*{{{*/
{
	if (no_header) return;

	string header;

	// Content-Type
	header.append ("Content-Type: "+mime_type);
	if (!charset.empty ()) header.append ("; charset="+charset);
	header.append ("\n");

	// Content-Disposition
	if (!content_disposition.empty ())
	{
		// Content-Disposition
		header.append ("Content-Disposition: "+content_disposition);
		// Filename
		if (!content_filename.empty ()) header.append (";filename=\""+content_filename+"\"");
		// ...;creation-date=`date -R`
		header.append ("\n");
	}

	// Content-Description
	if (!content_description.empty ()) header.append ("Content-Description: "+content_description+"\n");

	//header.append ("Cache-Control: no-cache\nPragma: no-cache\n")

	output_buffer << header;
	output_buffer << endl;
}
/*}}}*/

void http_document::output (const string &document, const string &mime_type, const string &charset)/*{{{*/
{
	output_header (mime_type, charset);
	output_buffer << document;
}
/*}}}*/

void http_document::output (const html_document &document)/*{{{*/
{
	output_header ("text/html", "ISO-8859-1");
	output_buffer << document.text ();
}
/*}}}*/

void http_document::output_redirect (string target)/*{{{*/
{
	output_buffer << "Location: " << target << endl;
	output_buffer << endl;
}
/*}}}*/

void http_document::do_output (ostream &stream)/*{{{*/
{
	stream << output_buffer.str ();
	output_buffer.str ("");
}
/*}}}*/

