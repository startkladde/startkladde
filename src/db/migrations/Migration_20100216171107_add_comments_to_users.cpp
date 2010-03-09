#include "Migration_20100216171107_add_comments_to_users.h"

Migration_20100216171107_add_comments_to_users::Migration_20100216171107_add_comments_to_users (Interface &interface):
	Migration (interface)
{
}

Migration_20100216171107_add_comments_to_users::~Migration_20100216171107_add_comments_to_users ()
{
}

void Migration_20100216171107_add_comments_to_users::up (OperationMonitorInterface monitor)
{
	addColumn ("users", "comments", dataTypeString ());
}

void Migration_20100216171107_add_comments_to_users::down (OperationMonitorInterface monitor)
{
	dropColumn ("users", "comments");
}
