#ifndef what_next_h
#define what_next_h

/*
 * what_next
 * martin
 * 2005-01-02
 */

#include <string>
#include "accessor.h"

using namespace std;

/*
 * What to do next, basically the program state.
 *   - wn_go_on: no error, go on with whatever you were doing
 *   - wn_output_error: an error occured. Stop processing and output an error
 *     document.
 *   - wn_output_document: processing finished. Output the document.
 *   - wn_change_state: go to another state, possibly erasing output and
 *     starting over.
 *   - wn_do_redirect: redirect the user to a different state or web page.
 *   - wn_end_program: everything done. Exit.
 */
enum what_next_t
{
	wn_go_on,
	wn_output_error,
	wn_output_document,
	wn_change_state,
	wn_do_redirect,
	wn_end_program,
	wn_output_raw_document
};

class what_next
{
	public:
		static what_next go_on ();
		static what_next output_error (const string &message, const string &explanation="");
		static what_next output_document ();
		static what_next output_raw_document (const string &document, const string &mime_type, const string &filename="", const string &description="");
		static what_next go_to_state (const string &new_state_label, const string &_message="", bool _message_error=false);
		static what_next do_redirect (const string &url);
		static what_next end_program ();

#define READ_ACCESS(TYPE, NAME, REALNAME)	\
		TYPE get_ ## NAME () const	\
		{	\
			return REALNAME;	\
		}

		READ_ACCESS (what_next_t, next, next)	// *
		READ_ACCESS (string, message, data)	// output_error go_to_state
		READ_ACCESS (string, state_label, state_label)	// go_to_state
		READ_ACCESS (bool, message_error, flag)	// go_to_state
		READ_ACCESS (string, url, data)	// do_redirect
		READ_ACCESS (string, document, data)	// output_raw_document
		READ_ACCESS (string, mime_type, mime_type)	// output_raw_document
		READ_ACCESS (string, filename, filename)	// output_raw_document
		READ_ACCESS (string, description, description)	// output_raw_document
		READ_ACCESS (string, explanation, description)	// output_error
#undef READ_ACCESS

		RW_ACCESSOR_REF_DEF (what_next, bool, keep_title, true)

	private:
		what_next (what_next_t _next):next (_next), flag (false), keep_title (false) {};

#define SETTABLE_MEMBER(TYPE, NAME)	\
		TYPE NAME;	\
		what_next &set_ ## NAME (const TYPE &_ ## NAME)	\
		{	\
			NAME=_ ## NAME;	\
			return *this;	\
		}
		SETTABLE_MEMBER (what_next_t, next)
		SETTABLE_MEMBER (string, data)
		SETTABLE_MEMBER (string, state_label)
		SETTABLE_MEMBER (bool, flag)
		SETTABLE_MEMBER (string, mime_type)
		SETTABLE_MEMBER (string, filename)
		SETTABLE_MEMBER (string, description)
#undef SETTABLE_MEMBER

		bool keep_title;
};

#endif

