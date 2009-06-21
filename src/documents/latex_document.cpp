#include "latex_document.h"

#include <fstream>
#include <sstream>

#include "malloc.h"
#include "unistd.h"

#include "src/text.h"

string latex_document::package::make_use_clause () const/*{{{*/
{
	string r;
	r.append ("\\usepackage");
	if (!options.empty ()) r.append ("["+make_string (options)+"]");
	r.append ("{"+name+"}");

	return r;
}
/*}}}*/

latex_document::package::package (const string &_name, const string &opt0, const string &opt1, const string &opt2, const string &opt3)/*{{{*/
{
	name=_name;
	if (!opt0.empty ()) options.insert (opt0);
	if (!opt1.empty ()) options.insert (opt1);
	if (!opt2.empty ()) options.insert (opt2);
	if (!opt3.empty ()) options.insert (opt3);
}
/*}}}*/

latex_document::latex_document ()/*{{{*/
{
	document_class="scrartcl";
	document_options.insert ("a4paper");
	font_size=10;
	pagestyle="scrheadings";
	margin_top="2.5cm";
	margin_left="1.0cm";
	margin_right="1.0cm";
	margin_bottom="2.5cm";
	tabcolsep="0.4mm";
	landscape=false;
}/*}}}*/

void latex_document::write_header ()/*{{{*/
{
	// Set up document options
	set<string> docopts=document_options;
	docopts.insert (num_to_string (font_size)+"pt");
	if (landscape) docopts.insert ("landscape");

	// Set up packages
	// TOOD operate on a copy
	add_package (package ("babel", "ngerman"));
	add_package (package ("inputenc", "latin1"));
	add_package (package ("geometry", "dvips"));	// TODO dvips? pdflatex?
	add_package (package ("ae"));
	add_package (package ("fontenc", "T1"));
	add_package (package ("scrpage2", ""));
	add_package (package ("longtable", ""));
	add_package (package ("lastpage", ""));
	add_package (package ("ragged2e", "NewCommands"));
	add_package (package ("array", ""));
	add_package (package ("pstricks", ""));	// TODO required?

	// Output the document

	// Document class
	doc << "\\documentclass[" << make_string (docopts, ",") << "]{" << document_class << "}" << endl;

	// Packages
	list<package>::const_iterator end=packages.end ();
	for (list<package>::const_iterator it=packages.begin (); it!=end; ++it)
		doc << (*it).make_use_clause () << endl;

	doc << "\\pagestyle{" << pagestyle << "}" << endl;

	doc << "\\geometry{"
		<< "top=" << margin_top << ","
		<< "left=" << margin_left << ","
		<< "right=" << margin_right << ","
		<< "bottom=" << margin_bottom << "}" << endl;

	doc << "\\begin{document}" << endl;
	doc << endl;

	if (no_section_numbers)
	{
		doc << "\\setcounter{secnumdepth}{-2}" << endl;
		doc << endl;
	}

	doc << "\\newcommand{\\cliptext}[2]{\\psclip{\\psframe[linestyle=none](-1,-1)(#1,2)}{\\hbox{#2}}\\endpsclip}" << endl;
	doc << endl;

	doc << "\\cohead{" << head_co << "}" << endl;
	doc << "\\lohead{" << head_lo << "}" << endl;
	doc << "\\rohead{" << head_ro << "}" << endl;
	doc << "\\cofoot{" << foot_co << "}" << endl;
	doc << "\\lofoot{" << foot_lo << "}" << endl;
	doc << "\\rofoot{" << foot_ro << "}" << endl;
	doc << endl;

	doc << "\\setlength{\\tabcolsep}{" << tabcolsep << "}" << endl;
	doc << endl;
}
/*}}}*/

void latex_document::write_footer ()/*{{{*/
{
	doc << endl;
	doc << "\\end{document}" << endl;
}
/*}}}*/

void latex_document::add_package (const package &p)/*{{{*/
{
	list<package>::const_iterator end=packages.end ();
	for (list<package>::iterator it=packages.begin (); it!=end; ++it)
	{
		if ((*it).name==p.name)
		{
			// Already have this package
			(*it).options.insert (p.options.begin (), p.options.end ());

			return;
		}
	}

	packages.push_back (p);
}
/*}}}*/

string latex_document::get_string ()/*{{{*/
{
	// Delete the document buffer
	doc.str ();

	// Write the header, body and footer
	write_header ();
	doc << body.str ();
	write_footer ();

	// Return the document.
	return doc.str ();
}
/*}}}*/

latex_document &latex_document::write (const table &tab, const table_row &header, const list<float> &widths)/*{{{*/
{
	// This writes a table. It is a bit q'n'd, in lack of a proper data
	// structure. Only rows for which widths exist are written.

	table_row::const_iterator header_end=header.end ();
	list<float>::const_iterator widths_end=widths.end ();
	table_row::const_iterator column;
	list<float>::const_iterator width;

	// Write the columns declaration
	string columns_string;
	string header_string;

	// For the clipping, we have to add 0.5mm. I don't know of a better way to
	// do it, but this is why we don't use strings for widths as before.
	for (column=header.begin (), width=widths.begin (); column!=header_end && width!=widths_end; ++column, ++width)
	{
		columns_string.append (">{\\raggedright}p{"+num_to_string (*width)+"mm}| ");

		if (column!=header.begin ()) header_string.append (" & ");
		header_string.append ("\\cliptext{"+num_to_string ((*width)+(float)0.5)+"mm}{"+latex_escape (*column)+"}");
	}
	body << "\\begin{longtable}{|" << columns_string << "} \\hline" << endl;
	body << header_string << "\\tabularnewline\\hline\\hline\\endhead" << endl;

	table::const_iterator table_end=tab.end ();
	for (table::const_iterator row=tab.begin (); row!=table_end; ++row)
	{
		int col=0;
		table_row::const_iterator row_end=(*row).end ();
		for (column=(*row).begin (), width=widths.begin (); column!=row_end && width!=widths_end; ++column, ++width)
		{
			col++;
			if (column!=(*row).begin ()) body << " & ";
			body << "\\cliptext{" << num_to_string ((*width)+(float)0.5) << "mm}{" << latex_escape (*column) << "}";
		}

		body << "\\tabularnewline\\hline" << endl;
	}

	body << "\\end{longtable}" << endl;
	return *this;
}
/*}}}*/

latex_document &latex_document::write_text (const string &text)/*{{{*/
{
	body << latex_escape (text);
	return *this;
}
/*}}}*/

latex_document &latex_document::start_section (const string &caption)/*{{{*/
{
	body << "\\section{" << latex_escape (caption) << "}" << endl;
	return *this;
}
/*}}}*/

latex_document &latex_document::write_empty_line ()/*{{{*/
{
	body << endl;
	return *this;
}
/*}}}*/

// TODO auslagern
void latex_document::execute_command (const string &command, const string &location) throw (ex_generate_error)/*{{{*/
{
	// TODO this is a generic function
	// TODO better error checking: if there was an error, return the output of
	// the process. If there was no error, the process output should also made
	// available.
//	int ret=system (command.c_str ());

	FILE *subprocess=popen (command.c_str (), "r");
	string output;

	const int maxlen=1128;
	char buf[maxlen+1];

	while (!feof (subprocess))
	{
		size_t len=fread (buf, 1, maxlen, subprocess);
		output.append (buf, len);
	}

	int ret=pclose (subprocess);

	if (ret!=0) throw ex_command_failed ("Fehler "+num_to_string (ret)+" "+location, ret, command, output);
}
/*}}}*/

string latex_document::make_pdf ()/*{{{*/
	throw (sk_exception)
{
	// Make a temporary directory
	temp_dir dir ("sk_web.latex.");

	// Construct the names
	string dir_name=dir.get_name ();
	string base_name=dir_name+"/latex_document";
	string latex_file_name=base_name+".tex";
	string dvi_file_name=base_name+".dvi";
	string ps_file_name=base_name+".ps";
	string pdf_file_name=base_name+".pdf";

	// Save the document to a file in the temporary directory
	string doc_string=get_string ();

//throw ex_command_failed ("Test", 0, "-", doc_string);


	ofstream latex_file (latex_file_name.c_str ());
	if (!latex_file.is_open ()) throw ex_generate_error ("Fehler beim �ffnen der Ausgabedatei");

	latex_file << doc_string;
	latex_file.close ();

	char *old_pwd_buffer=getcwd (NULL, 0);
	string old_pwd (old_pwd_buffer);
	free (old_pwd_buffer);

	chdir (dir_name.c_str ());

	try
	{
		execute_command ("latex --interaction nonstopmode "+latex_file_name, "beim 1. Aufruf von LaTeX");
		execute_command ("latex --interaction nonstopmode "+latex_file_name, "beim 2. Aufruf von LaTeX");
		execute_command ("latex --interaction nonstopmode "+latex_file_name, "beim 3. Aufruf von LaTeX");
	}
	catch (ex_command_failed &e)
	{
		e.document=doc_string;
		throw e;
	}
	catch (...)
	{
		throw;
	}

	execute_command ("dvips "+dvi_file_name, "beim Aufruf von dvips");
	execute_command ("ps2pdf "+ps_file_name, "beim Aufruf von ps2pdf");

	ifstream pdf_file (pdf_file_name.c_str ());
	if (!pdf_file.is_open ()) throw ex_generate_error ("Fehler beim �ffnen der PDF-Datei");

	// Copy the file
	string r;
	string line;
	while (getline (pdf_file, line))
	{
		r.append (line);
		r.append ("\n");
	}
	pdf_file.close ();

	chdir (old_pwd.c_str ());

	return r;
}
/*}}}*/


