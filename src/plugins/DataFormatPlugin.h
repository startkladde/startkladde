#ifndef _DataFormatPlugin_h
#define _DataFormatPlugin_h

#include <QString>

#include "src/accessor.h"
#include "src/ObjectField.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/LaunchType.h"
#include "src/web/Argument.h"

// Not using qlibrary because it does strange things to the name (resulting in
// "liblibfoo.so.so" when copying) and has poor error reporting.

// Functions starting with "plugin" call the corresponding library function.

class DataFormatPlugin
{
	public:
		// Exceptions
		class exception: public std::exception
		{
			public:
				exception (const QString &_desc) :desc (_desc) {}
				~exception () throw () {}
				virtual QString description () { return desc; }
				QString desc;
		};
		class ex_file_not_found: public exception
		{
			public:
				ex_file_not_found (const QString &name=""):exception (name.isEmpty ()?"Plugin-Datei nicht gefunden":"Plugin-Datei "+name+" nicht gefunden") {};
		};
		class ex_load_error: public exception
		{
			public:
				ex_load_error (const QString &msg=""):exception (msg.isEmpty ()?"Fehler beim Laden der Bibliothek":"Fehler beim Laden der Bibliothek: "+msg) {};
		};
		class ex_symbol_not_found: public exception
		{
			public:
				ex_symbol_not_found (const QString &name):exception ("Symbol "+name+" nicht gefunden") {};
		};
		class ex_plugin_internal_error: public exception
		{
			public:
				ex_plugin_internal_error (const QString &msg):exception (msg), fatal (true) {};
				ex_plugin_internal_error &is_fatal (bool f=true) { fatal=f; return *this; }
				bool fatal;

		};
		class ex_plugin_invalid_format: public ex_plugin_internal_error
		{
			public:
				ex_plugin_invalid_format (const QString &fmt=""): ex_plugin_internal_error (fmt.isEmpty ()?"Ungültiges Format":"Ungültiges Format \""+fmt+"\"") { fatal=true; }
		};

		// Construction
		DataFormatPlugin (const QString &_filename);
		DataFormatPlugin (const DataFormatPlugin &p);
		~DataFormatPlugin ();

		// Library handling
		void load () const;
		RO_ACCESSOR (QString, filename)
		RO_ACCESSOR (QString, real_filename)
		RO_ACCESSOR (bool, loaded)
		QString get_display_filename () const { if (real_filename.isEmpty ()) return filename; else return real_filename; }

		// Metainformation
		ArgumentList plugin_list_formats () const;
		bool provides_format (const QString &label) const;
		// Unique names have the filename or something appended so they can be
		// distiguished from other plugins using the same format name.
		ArgumentList plugin_list_unique_formats () const;
		bool provides_unique_format (const QString &label) const;
		QString make_unique (const QString &label) const;
		QString make_non_unique (const QString &label) const;

		// Actual plugin functions
		void plugin_make_field_list (const QString &format, QList<ObjectField> &fields) const;
		void plugin_flight_to_fields (const QString &format, QList<ObjectField> &fields, const Flight &f, const sk_flug_data &flight_data, int &num, const QString &none_text="", const QString &error_text="") const;

	private:
		// Library handling
		QString filename;
		mutable bool loaded;
		mutable void *handle;
		mutable QString real_filename;

#define PLUGIN_SYMBOL(NAME, RETVAL, ARGLIST)	\
		typedef RETVAL (*type_ ## NAME) ARGLIST;	\
		mutable type_ ## NAME symbol_ ## NAME;

		PLUGIN_SYMBOL (list_formats, ArgumentList, ())
		PLUGIN_SYMBOL (make_field_list, void, (const QString &format, QList<ObjectField> &fields));
		PLUGIN_SYMBOL (flight_to_fields, void, (const QString &format, QList<ObjectField> &fields, const Flight &f, const sk_flug_data &flight_data, int &num, const QString &none_text, const QString &error_text));
#undef PLUGIN_SYMBOL

};

#endif

