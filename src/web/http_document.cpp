#include "http_document.h"

const QString http_document::mime_type_pdf="application/pdf";
const QString http_document::mime_type_plaintext="text/plain";
const QString http_document::mime_type_csv="text/comma-separated-values";
const QString http_document::mime_type_form_urlencoded="application/x-www-form-urlencoded";
const QString http_document::mime_type_multipart_form_data="multipart/form-data";


http_document::http_document ()/*{{{*/
{
	no_header=false;
}
/*}}}*/

void http_document::output_header (const QString &mime_type, const QString &charset)/*{{{*/
{
	if (no_header) return;

	QString header;

	// Content-Type
	header.append ("Content-Type: "+mime_type);
	if (!charset.isEmpty ()) header.append ("; charset="+charset);
	header.append ("\n");

	// Content-Disposition
	if (!content_disposition.isEmpty ())
	{
		// Content-Disposition
		header.append ("Content-Disposition: "+content_disposition);
		// Filename
		if (!content_filename.isEmpty ()) header.append (";filename=\""+content_filename+"\"");
		// ...;creation-date=`date -R`
		header.append ("\n");
	}

	// Content-Description
	if (!content_description.isEmpty ()) header.append ("Content-Description: "+content_description+"\n");

	//header.append ("Cache-Control: no-cache\nPragma: no-cache\n")

	output_buffer << header;
	output_buffer << std::endl;
}
/*}}}*/

void http_document::output (const QString &document, const QString &mime_type, const QString &charset)/*{{{*/
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

void http_document::output_redirect (QString target)/*{{{*/
{
	output_buffer << "Location: " << target << std::endl;
	output_buffer << std::endl;
}
/*}}}*/

void http_document::do_output (std::ostream &stream)/*{{{*/
{
	stream << output_buffer.str ();
	output_buffer.str ("");
}
/*}}}*/

