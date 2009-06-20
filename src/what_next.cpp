#include "what_next.h"

what_next what_next::go_on ()/*{{{*/
{
	return what_next (wn_go_on);
}
/*}}}*/

what_next what_next::output_error (const string &message, const string &explanation)/*{{{*/
{
	return what_next (wn_output_error)
		.set_data (message)
		.set_description (explanation)
		;
}
/*}}}*/

what_next what_next::output_document ()/*{{{*/
{
	return what_next (wn_output_document);
}
/*}}}*/

what_next what_next::output_raw_document (const string &document, const string &mime_type, const string &filename, const string &description)/*{{{*/
{
	return what_next (wn_output_raw_document)
		.set_data (document)
		.set_mime_type (mime_type)
		.set_filename (filename)
		.set_description (description)
		;
}
/*}}}*/

what_next what_next::go_to_state (const string &new_state_label, const string &_message, bool _message_error)/*{{{*/
{
	return what_next (wn_change_state)
		.set_state_label (new_state_label)
		.set_data (_message)
		.set_flag (_message_error);
}
/*}}}*/

what_next what_next::do_redirect (const string &url)/*{{{*/
{
	return what_next (wn_do_redirect)
		.set_data (url);
}
/*}}}*/

what_next what_next::end_program ()/*{{{*/
{
	return what_next (wn_end_program);
}
/*}}}*/



