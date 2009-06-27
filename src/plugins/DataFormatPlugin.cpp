#include "DataFormatPlugin.h"

#include <dlfcn.h>

#include "src/text.h"
#include "src/config/Options.h"

// Construction
DataFormatPlugin::DataFormatPlugin (const QString &_filename)
	:filename (_filename+".so"), loaded (false), handle (NULL)
	// filename: without extension
{
	symbol_list_formats=NULL;
	symbol_make_field_list=NULL;
	symbol_flight_to_fields=NULL;
}

DataFormatPlugin::DataFormatPlugin (const DataFormatPlugin &p)
	:filename (p.filename), loaded (false), handle (NULL)
{
}

DataFormatPlugin::~DataFormatPlugin ()
{
	if (handle) dlclose (handle);
}

/**
  *load plugin library
  */
void DataFormatPlugin::load () const
{
	if (!loaded)
	{
		real_filename=opts.find_plugin_file (filename);
		if (real_filename.isEmpty ()) {
			throw ex_file_not_found (filename);
		}

		handle=dlopen (real_filename.latin1(), RTLD_NOW);

		if (handle)
		{
			loaded=true;
		}
		else
		{
			loaded=false;
			throw ex_load_error (QString (dlerror ()));
		}
	}

	// Now we're either loaded or thrown.
#define USE_SYMBOL(NAME) symbol_ ## NAME=(type_ ## NAME)dlsym (handle, #NAME);
	USE_SYMBOL (list_formats);
	USE_SYMBOL (make_field_list);
	USE_SYMBOL (flight_to_fields);
#undef USE_SYMBOL
}


#define REQUIRE_SYMBOL(NAME) do { if (!symbol_ ## NAME) throw ex_symbol_not_found (#NAME); } while (false)

// Metainformation
ArgumentList DataFormatPlugin::plugin_list_formats () const
{
	load ();
	REQUIRE_SYMBOL (list_formats);
	return symbol_list_formats ();
}

bool DataFormatPlugin::provides_format (const QString &label) const
{
	ArgumentList plugins=plugin_list_formats ();
	return plugins.has_argument (label);
}

ArgumentList DataFormatPlugin::plugin_list_unique_formats () const
{
	load ();
	REQUIRE_SYMBOL (list_formats);
	ArgumentList formats=symbol_list_formats ();

	ArgumentList unique_formats;
	for (ArgumentList::const_iterator formats_end=formats.end (),
			format=formats.begin (); format!=formats_end; ++format)
		unique_formats.set_value (make_unique ((*format).get_name ()), (*format).get_value ());

	return unique_formats;
}

bool DataFormatPlugin::provides_unique_format (const QString &unique_label) const
{
	ArgumentList plugins=plugin_list_unique_formats ();
	return plugins.has_argument (unique_label);
}


QString DataFormatPlugin::make_unique (const QString &label) const
{
	return real_filename+":"+label;
}

QString DataFormatPlugin::make_non_unique (const QString &label) const
{
	int pos=label.lastIndexOf (':');
	if (pos<0)
		return label;
	else
		return label.mid (pos+1);
}



// Plugin functions
void DataFormatPlugin::plugin_make_field_list (const QString &format, QList<ObjectField> &fields) const
{
	load ();
	REQUIRE_SYMBOL (make_field_list);
	return symbol_make_field_list (make_non_unique (format), fields);
}

void DataFormatPlugin::plugin_flight_to_fields (const QString &format, QList<ObjectField> &fields, const Flight &f, const sk_flug_data &flight_data, int &num, const QString &none_text, const QString &error_text) const
{
	load ();
	REQUIRE_SYMBOL (flight_to_fields);
	return symbol_flight_to_fields (make_non_unique (format), fields, f, flight_data, num, none_text, error_text);
}



