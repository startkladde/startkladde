#ifndef _WhatNext_h
#define _WhatNext_h

/*
 * WhatNext
 * martin
 * 2005-01-02
 */

#include <QString>

#include "src/accessor.h"

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
enum WhatNextType
{
	wn_go_on,
	wn_output_error,
	wn_output_document,
	wn_change_state,
	wn_do_redirect,
	wn_end_program,
	wn_output_raw_document
};

class WhatNext
{
	public:
		static WhatNext go_on ();
		static WhatNext output_error (const QString &message, const QString &explanation="");
		static WhatNext output_document ();
		static WhatNext output_raw_document (const QString &document, const QString &mime_type, const QString &filename="", const QString &description="");
		static WhatNext go_to_state (const QString &new_state_label, const QString &_message="", bool _message_error=false);
		static WhatNext do_redirect (const QString &url);
		static WhatNext end_program ();

#define READ_ACCESS(TYPE, NAME, REALNAME)	\
		TYPE get_ ## NAME () const	\
		{	\
			return REALNAME;	\
		}

		READ_ACCESS (WhatNextType, next, next)	// *
		READ_ACCESS (QString, message, data)	// output_error go_to_state
		READ_ACCESS (QString, state_label, state_label)	// go_to_state
		READ_ACCESS (bool, message_error, flag)	// go_to_state
		READ_ACCESS (QString, url, data)	// do_redirect
		READ_ACCESS (QString, document, data)	// output_raw_document
		READ_ACCESS (QString, mime_type, mime_type)	// output_raw_document
		READ_ACCESS (QString, filename, filename)	// output_raw_document
		READ_ACCESS (QString, description, description)	// output_raw_document
		READ_ACCESS (QString, explanation, description)	// output_error
#undef READ_ACCESS

		RW_ACCESSOR_REF_DEF (WhatNext, bool, keep_title, true)

	private:
		WhatNext (WhatNextType _next):next (_next), flag (false), keep_title (false) {};

#define SETTABLE_MEMBER(TYPE, NAME)	\
		TYPE NAME;	\
		WhatNext &set_ ## NAME (const TYPE &_ ## NAME)	\
		{	\
			NAME=_ ## NAME;	\
			return *this;	\
		}
		SETTABLE_MEMBER (WhatNextType, next)
		SETTABLE_MEMBER (QString, data)
		SETTABLE_MEMBER (QString, state_label)
		SETTABLE_MEMBER (bool, flag)
		SETTABLE_MEMBER (QString, mime_type)
		SETTABLE_MEMBER (QString, filename)
		SETTABLE_MEMBER (QString, description)
#undef SETTABLE_MEMBER

		bool keep_title;
};

#endif

