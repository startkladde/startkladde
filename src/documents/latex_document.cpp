#include "latex_document.h"

#include <fstream>
#include <sstream>

#include "malloc.h"
#include "unistd.h"

#include "src/text.h"

QString latex_document::package::make_use_clause () const
{
	QString r;
	r.append ("\\usepackage");
	if (!options.empty ()) r.append ("["+make_string (options)+"]");
	r.append ("{"+name+"}");

	return r;
}

latex_document::package::package (const QString &_name, const QString &opt0, const QString &opt1, const QString &opt2, const QString &opt3)
{
	name=_name;
	if (!opt0.isEmpty ()) options.insert (opt0);
	if (!opt1.isEmpty ()) options.insert (opt1);
	if (!opt2.isEmpty ()) options.insert (opt2);
	if (!opt3.isEmpty ()) options.insert (opt3);
}

latex_document::latex_document ()
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
}

void latex_document::write_header ()
{
	// Set up document options
	QSet<QString> docopts=document_options;
	docopts.insert (QString::number(font_size)+"pt");
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
	doc << "\\documentclass[" << make_string (docopts, ",") << "]{" << document_class << "}" << std::endl;

	// Packages
	QList<package>::const_iterator end=packages.end ();
	for (QList<package>::const_iterator it=packages.begin (); it!=end; ++it)
		doc << (*it).make_use_clause () << std::endl;

	doc << "\\pagestyle{" << pagestyle << "}" << std::endl;

	doc << "\\geometry{"
		<< "top=" << margin_top << ","
		<< "left=" << margin_left << ","
		<< "right=" << margin_right << ","
		<< "bottom=" << margin_bottom << "}" << std::endl;

	doc << "\\begin{document}" << std::endl;
	doc << std::endl;

	if (no_section_numbers)
	{
		doc << "\\setcounter{secnumdepth}{-2}" << std::endl;
		doc << std::endl;
	}

	doc << "\\newcommand{\\cliptext}[2]{\\psclip{\\psframe[linestyle=none](-1,-1)(#1,2)}{\\hbox{#2}}\\endpsclip}" << std::endl;
	doc << std::endl;

	doc << "\\cohead{" << head_co << "}" << std::endl;
	doc << "\\lohead{" << head_lo << "}" << std::endl;
	doc << "\\rohead{" << head_ro << "}" << std::endl;
	doc << "\\cofoot{" << foot_co << "}" << std::endl;
	doc << "\\lofoot{" << foot_lo << "}" << std::endl;
	doc << "\\rofoot{" << foot_ro << "}" << std::endl;
	doc << std::endl;

	doc << "\\setlength{\\tabcolsep}{" << tabcolsep << "}" << std::endl;
	doc << std::endl;
}

void latex_document::write_footer ()
{
	doc << std::endl;
	doc << "\\end{document}" << std::endl;
}

void latex_document::add_package (const package &p)
{
	QMutableListIterator<package> it (packages);
	while (it.hasNext ())
	{
		package &currentPackage=it.next ();
		if (currentPackage.name==p.name)
		{
			// Already have this package
			foreach (QString option, p.options)
				currentPackage.options << option;

			return;
		}
	}

	packages.push_back (p);
}

QString latex_document::get_string ()
{
	// Delete the document buffer
	doc.str ();

	// Write the header, body and footer
	write_header ();
	doc << body.str ();
	write_footer ();

	// Return the document.
	return std2q (doc.str ());
}

latex_document &latex_document::write (const table &tab, const table_row &header, const QList<float> &widths)
{
	// This writes a table. It is a bit q'n'd, in lack of a proper data
	// structure. Only rows for which widths exist are written.

	table_row::const_iterator header_end=header.end ();
	QList<float>::const_iterator widths_end=widths.end ();
	table_row::const_iterator column;
	QList<float>::const_iterator width;

	// Write the columns declaration
	QString columns_string;
	QString header_string;

	// For the clipping, we have to add 0.5mm. I don't know of a better way to
	// do it, but this is why we don't use strings for widths as before.
	for (column=header.begin (), width=widths.begin (); column!=header_end && width!=widths_end; ++column, ++width)
	{
		columns_string.append (">{\\raggedright}p{"+QString::number (*width)+"mm}| ");

		if (column!=header.begin ()) header_string.append (" & ");
		header_string.append ("\\cliptext{"+QString::number ((*width)+(float)0.5)+"mm}{"+latex_escape (*column)+"}");
	}
	body << "\\begin{longtable}{|" << columns_string << "} \\hline" << std::endl;
	body << header_string << "\\tabularnewline\\hline\\hline\\endhead" << std::endl;

	table::const_iterator table_end=tab.end ();
	for (table::const_iterator row=tab.begin (); row!=table_end; ++row)
	{
		int col=0;
		table_row::const_iterator row_end=(*row).end ();
		for (column=(*row).begin (), width=widths.begin (); column!=row_end && width!=widths_end; ++column, ++width)
		{
			col++;
			if (column!=(*row).begin ()) body << " & ";
			body << "\\cliptext{" << QString::number ((*width)+(float)0.5) << "mm}{" << latex_escape (*column) << "}";
		}

		body << "\\tabularnewline\\hline" << std::endl;
	}

	body << "\\end{longtable}" << std::endl;
	return *this;
}

latex_document &latex_document::write_text (const QString &text)
{
	body << latex_escape (text);
	return *this;
}

latex_document &latex_document::start_section (const QString &caption)
{
	body << "\\section{" << latex_escape (caption) << "}" << std::endl;
	return *this;
}

latex_document &latex_document::write_empty_line ()
{
	body << std::endl;
	return *this;
}

// TODO auslagern
void latex_document::execute_command (const QString &command, const QString &location) throw (ex_generate_error)
{
	// TODO this is a generic function
	// TODO better error checking: if there was an error, return the output of
	// the process. If there was no error, the process output should also made
	// available.
//	int ret=system (command.c_str ());

	// TODO Qt-ize
	FILE *subprocess=popen (command.latin1(), "r");
	QString output;

	const int maxlen=1128;
	char buf[maxlen+1];

	while (!feof (subprocess))
	{
		size_t len=fread (buf, 1, maxlen, subprocess);
		output.append (QString::fromLatin1 (buf, len));
	}

	int ret=pclose (subprocess);

	if (ret!=0) throw ex_command_failed ("Fehler "+QString::number (ret)+" "+location, ret, command, output);
}

QString latex_document::make_pdf ()
	throw (sk_exception)
{
	// Make a temporary directory
	temp_dir dir ("sk_web.latex.");

	// Construct the names
	QString dir_name=dir.get_name ();
	QString base_name=dir_name+"/latex_document";
	QString latex_file_name=base_name+".tex";
	QString dvi_file_name=base_name+".dvi";
	QString ps_file_name=base_name+".ps";
	QString pdf_file_name=base_name+".pdf";

	// Save the document to a file in the temporary directory
	QString doc_string=get_string ();

//throw ex_command_failed ("Test", 0, "-", doc_string);


	std::ofstream latex_file (latex_file_name.latin1());
	if (!latex_file.is_open ()) throw ex_generate_error ("Fehler beim �ffnen der Ausgabedatei");

	latex_file << doc_string;
	latex_file.close ();

	char *old_pwd_buffer=getcwd (NULL, 0);
	QString old_pwd (old_pwd_buffer);
	free (old_pwd_buffer);

	chdir (dir_name.latin1());

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

	std::ifstream pdf_file (pdf_file_name.latin1());
	if (!pdf_file.is_open ()) throw ex_generate_error ("Fehler beim �ffnen der PDF-Datei");

	// Copy the file
	// TODO Qt-ize
	QString r;
	std::string line;
	while (getline (pdf_file, line))
	{
		r.append (std2q (line));
		r.append ("\n");
	}
	pdf_file.close ();

	chdir (old_pwd.latin1());

	return r;
}


