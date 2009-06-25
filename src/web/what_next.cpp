#include "what_next.h"

what_next what_next::go_on ()
{
	return what_next (wn_go_on);
}

what_next what_next::output_error (const QString &message, const QString &explanation)
{
	return what_next (wn_output_error)
		.set_data (message)
		.set_description (explanation)
		;
}

what_next what_next::output_document ()
{
	return what_next (wn_output_document);
}

what_next what_next::output_raw_document (const QString &document, const QString &mime_type, const QString &filename, const QString &description)
{
	return what_next (wn_output_raw_document)
		.set_data (document)
		.set_mime_type (mime_type)
		.set_filename (filename)
		.set_description (description)
		;
}

what_next what_next::go_to_state (const QString &new_state_label, const QString &_message, bool _message_error)
{
	return what_next (wn_change_state)
		.set_state_label (new_state_label)
		.set_data (_message)
		.set_flag (_message_error);
}

what_next what_next::do_redirect (const QString &url)
{
	return what_next (wn_do_redirect)
		.set_data (url);
}

what_next what_next::end_program ()
{
	return what_next (wn_end_program);
}



