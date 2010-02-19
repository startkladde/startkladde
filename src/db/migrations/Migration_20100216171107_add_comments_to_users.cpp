#include "Migration_20100216171107_add_comments_to_users.h"

#include "src/db/Database.h"

Migration_20100216171107_add_comments_to_users::Migration_20100216171107_add_comments_to_users (Database &database):
	Migration (database)
{
}

Migration_20100216171107_add_comments_to_users::~Migration_20100216171107_add_comments_to_users ()
{
}

void Migration_20100216171107_add_comments_to_users::up ()
{
	database.addColumn ("users", "comments", Database::dataTypeString);
}

void Migration_20100216171107_add_comments_to_users::down ()
{
	database.dropColumn ("users", "comments");
}
