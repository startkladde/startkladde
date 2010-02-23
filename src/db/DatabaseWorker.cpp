#include "DatabaseWorker.h"

#include <iostream>

#include "src/db/DatabaseTask.h"

DatabaseWorker::DatabaseWorker ()
{
}

DatabaseWorker::~DatabaseWorker ()
{
}

void DatabaseWorker::runTask (DatabaseTask *task)
{
	try
	{
		task->run ();
	}
	catch (...)
	{
		// Got an exception. Since this slot is called from the event loop, we
		// cannot do anything about the exception (letting it propagate to the
		// event loop does not make sense).
		std::cerr << "DatabaseWorker::runTask caught an exception" << std::endl;
	}
}
