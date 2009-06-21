#include "plugin_data_format.h"

#include <dlfcn.h>

#include "src/text.h"
#include "src/config/options.h"

// Construction
plugin_data_format::plugin_data_format (const string &_filename)/*{{{*/
	:filename (_filename+".so"), loaded (false), handle (NULL)
	// filename: without extension
{
	symbol_list_formats=NULL;
	symbol_make_field_list=NULL;
	symbol_flight_to_fields=NULL;
}/*}}}*/

plugin_data_format::plugin_data_format (const plugin_data_format &p)/*{{{*/
	:filename (p.filename), loaded (false), handle (NULL)
{
}
/*}}}*/

plugin_data_format::~plugin_data_format ()/*{{{*/
{
	if (handle) dlclose (handle);
}
/*}}}*/

/**
  *load plugin library
  */
void plugin_data_format::load () const/*{{{*/
{
	if (!loaded)
	{
		real_filename=opts.find_plugin_file (filename);
		if (real_filename.empty ()) {
			throw ex_file_not_found (filename);
		}

		handle=dlopen ((real_filename).c_str (), RTLD_NOW);

		if (handle)
		{
			loaded=true;
		}
		else
		{
			loaded=false;
			throw ex_load_error (string (dlerror ()));
		}
	}

	// Now we're either loaded or thrown.
#define USE_SYMBOL(NAME) symbol_ ## NAME=(type_ ## NAME)dlsym (handle, #NAME);
	USE_SYMBOL (list_formats);
	USE_SYMBOL (make_field_list);
	USE_SYMBOL (flight_to_fields);
#undef USE_SYMBOL
}

/*}}}*/

#define REQUIRE_SYMBOL(NAME) do { if (!symbol_ ## NAME) throw ex_symbol_not_found (#NAME); } while (false)

// Metainformation
argument_list plugin_data_format::plugin_list_formats () const/*{{{*/
{
	load ();
	REQUIRE_SYMBOL (list_formats);
	return symbol_list_formats ();
}
/*}}}*/

bool plugin_data_format::provides_format (const string &label) const/*{{{*/
{
	argument_list plugins=plugin_list_formats ();
	return plugins.has_argument (label);
}
/*}}}*/

argument_list plugin_data_format::plugin_list_unique_formats () const/*{{{*/
{
	load ();
	REQUIRE_SYMBOL (list_formats);
	argument_list formats=symbol_list_formats ();

	argument_list unique_formats;
	for (argument_list::const_iterator formats_end=formats.end (),
			format=formats.begin (); format!=formats_end; ++format)
		unique_formats.set_value (make_unique ((*format).get_name ()), (*format).get_value ());

	return unique_formats;
}
/*}}}*/

bool plugin_data_format::provides_unique_format (const string &unique_label) const/*{{{*/
{
	argument_list plugins=plugin_list_unique_formats ();
	return plugins.has_argument (unique_label);
}
/*}}}*/


string plugin_data_format::make_unique (const string &label) const/*{{{*/
{
	return real_filename+":"+label;
}
/*}}}*/

string plugin_data_format::make_non_unique (const string &label) const/*{{{*/
{
	string::size_type pos=label.rfind (':', label.length ());
	if (pos==string::npos)
		return label;
	else
		return label.substr (pos+1);
}
/*}}}*/



// Plugin functions
void plugin_data_format::plugin_make_field_list (const string &format, list<object_field> &fields) const/*{{{*/
{
	load ();
	REQUIRE_SYMBOL (make_field_list);
	return symbol_make_field_list (make_non_unique (format), fields);
}
/*}}}*/

void plugin_data_format::plugin_flight_to_fields (const string &format, list<object_field> &fields, const sk_flug &f, const sk_flug_data &flight_data, int &num, const string &none_text, const string &error_text) const/*{{{*/
{
	load ();
	REQUIRE_SYMBOL (flight_to_fields);
	return symbol_flight_to_fields (make_non_unique (format), fields, f, flight_data, num, none_text, error_text);
}
/*}}}*/



