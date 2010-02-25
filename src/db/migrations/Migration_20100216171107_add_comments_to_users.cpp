#include "Migration_20100216171107_add_comments_to_users.h"

Migration_20100216171107_add_comments_to_users::Migration_20100216171107_add_comments_to_users (Db::Interface::DefaultInterface &interface):
	Migration (interface)
{
}

Migration_20100216171107_add_comments_to_users::~Migration_20100216171107_add_comments_to_users ()
{
}

void Migration_20100216171107_add_comments_to_users::up ()
{
	addColumn ("users", "comments", dataTypeString ());
}

void Migration_20100216171107_add_comments_to_users::down ()
{
	dropColumn ("users", "comments");
}
