
// ******************
// ** Construction **
// ******************

DataStorage::~DataStorage ()
{
	// This causes the program to hang on quit when the server is unresponsive
//	worker->stop (true);

	// This is not nice, but it will have to do for now.
	// TODO this should not be in the destructor, a terminate method would be
	// better, so we can terminate all threads before waiting
	worker->setTerminationEnabled (true);
	worker->terminate ();
	std::cout << "Wating for data storage worker to terminate..."; std::cout.flush ();
	if (worker->wait (1000))
		std::cout << "done" << std::endl;
	else
		std::cout << "timeout" << std::endl;

	delete worker;
	delete flightsToday;
	delete flightsOther;
	delete preparedFlights;
}

// **********************
// ** Database reading **
// **********************


// FIXME
bool retryOnResult (int result)
{
//	if (result==db_err_timeout) return true;
//	if (result==db_err_connection_failed) return true;
	return false;
}





// **********************
// ** Database writing **
// **********************

// These methods are a bit ugly because of the interface to Database
// TODO Database is better now

// TODO:
//  - also addObject planeTypes, clubs, but unique => QSet?
//  - for flights: only if really today
//  - error check for getObject
//  - when a database operation fails (due to a lost connection) after the
//    query has been executed, the cache may be inconsistent.
//  - should success and message be a method of monitor?
template<class T> bool DataStorage::addObject (OperationMonitor *monitor, const T &object, dbId *id, bool *success, QString *message)
{
//	(void)monitor;
//	// TODO addObject duplicate check?
//
//	// The copy will have the same id as the original, but the id will be
//	// overwritten.
//	T copy (object);
//
//	// Write the object by using the database method
//	QMutexLocker dbLock (&databaseMutex);
//	// TODO error check
//	dbId newId=db.createObject (copy);
//	dbLock.unlock ();
//
//	// TODO when can we cancel? When the object has been added, the cache
//	// must be updated.
//	//	if (monitor->isCanceled ()) return false;
//
//
//	// If adding succeeded, add the object to the cache and emit a event
//	if (idValid (newId))
//	{
//		objectAdded (copy);
//	}
//
//	// Task completed
//	if (id) *id=newId;
//	if (success) *success=(idValid (newId));
//	if (message) *message=db.lastError ().text ();
//	return true;
}

template<class T> bool DataStorage::deleteObject (OperationMonitor *monitor, dbId id, bool *success, QString *message)
{
//	(void)monitor;
//
//	// TODO error handling
//	QMutexLocker dbLock (&databaseMutex);
//	int affectedRows=db.deleteObject<T> (id);
//	dbLock.unlock ();
//
//	// TODO when can we cancel? When the object has been deleted, the cache
//	// must be updated.
//	//	if (monitor->isCanceled ()) return false;
//
//	bool ok=(affectedRows>0);
//
//	if (success) *success=ok;
//	if (message) *message=db.lastError ().text ();
//
//	if (ok)
//	{
//		objectDeleted<T> (id);
//	}
//
//	// Task completed
//	return true;
}

template<class T> bool DataStorage::updateObject (OperationMonitor *monitor, const T &object, bool *success, QString *message)
{
//	(void)monitor;
//
//	if (idInvalid (object.getId ())) return true; // TODO signal error
//	T copy (object);
//
//	QMutexLocker dbLock (&databaseMutex);
//	dbId result=db.updateObject (copy);
//	dbLock.unlock ();
//
//
//	// TODO when can we cancel? When the object has been updated, the cache
//	// must be updated.
//	//	if (monitor->isCanceled ()) return false;
//
//	if (success) *success=idValid (result);
//	if (message) *message=db.lastError ().text ();
//
//	if (idValid (result))
//		objectUpdated (copy);
//
//	return true;
}



// ***************************
// ** Connection management **
// ***************************

void DataStorage::setState (DataStorage::State state)
{
	QMutexLocker lock (&dataMutex);
	bool changed=(state!=currentState);
	currentState=state;
	lock.unlock ();

	if (changed)
		emit stateChanged (state);
}

//void DataStorage::setAlive (bool alive)
//{
//	QMutexLocker lock (&dataMutex);
//	bool changed=(alive!=isAlive);
//	isAlive=alive;
//	lock.unlock ();
//
//	if (changed)
//	{
//		if (alive)
//			emit status ("OK", false);
//		else
//			emit status ("Verbindung unterbrochen", true);
//	}
//}

void DataStorage::connect ()
{
	worker->scheduleConnect ();
}

void DataStorage::disconnect ()
{
	worker->scheduleDisconnect ();
}


void DataStorage::doConnect ()
{
//	setState (stateConnecting);
//
//	bool connected=false; // Whether a connection is established at the moment
//	bool unusable=false; // Whether the database has been found to be unusable
//
//	// TODO allow cancelling
//	while (!connected && !unusable)
//	{
////		try
////		{
//			QMutexLocker dbLock (&databaseMutex);
//			db.open ();
//			connected=true;
//			dbLock.unlock ();
////		}
//		// FIXME
////		catch (OldDatabase::ex_access_denied      ) { unusable=true; }
////		catch (OldDatabase::ex_connection_failed  ) { /* Keep trying */ }
//		// TODO other exceptions
//
//		// If we are not connected, wait one second before retrying
//		DefaultQThread::sleep (1);
//	}
//
//	// FIXME
////	// If the connection succeeded (i. e. we are connected now), try to use the
////	// database
////	bool checked=false;
////	while (!checked && !unusable)
////	{
////		if (connected)
////		{
////			try
////			{
////				QMutexLocker dbLock (&databaseMutex);
////				db.use_db (opts.database);
////				db.check_usability ();
////				checked=true;
////			}
////			catch (OldDatabase::ex_access_denied          ) { unusable=true; }
////			catch (OldDatabase::ex_database_not_found     ) { unusable=true; }
////			catch (OldDatabase::ex_database_not_accessible) { unusable=true; }
////			catch (OldDatabase::ex_insufficient_access    ) { unusable=true; }
////			catch (OldDatabase::ex_unusable               ) { unusable=true; }
////			catch (OldDatabase::ex_query_failed           ) { /* Keep trying */ }
////			catch (OldDatabase::ex_timeout                ) { /* Keep trying */ }
////			catch (OldDatabase::ex_connection_failed      ) { /* Keep trying */ }
////			// TODO other exceptions
////		}
////
////		// If checking failed, wait one second before retrying
////		DefaultQThread::sleep (1);
////	}
//
//	if (unusable)
//	{
//		if (connected)
//			db.close ();
//
//		setState (stateUnusable);
//	}
//	else if (connected)
//	{
//		emit status ("Daten werden gelesen...", false);
//		// TODO: pass an operation monitor to display the status in the main window
//		SimpleOperationMonitor monitor;
//
//		refreshAll (&monitor);
//
//		setState (stateConnected);
//	}
//	else
//	{
//		// Canceled
//	}
}


void DataStorage::doDisconnect ()
{
//	emit status ("Verbindung zur Datenbank wird getrennt...", false);
//
//	QMutexLocker dbLock (&databaseMutex);
//	db.close ();
//	dbLock.unlock ();
//
//	setState (stateOffline);
}

QString DataStorage::stateGetText (DataStorage::State state)
{
	switch (state)
	{
		case stateOffline: return "Nicht verbunden";
		case stateConnecting: return "Verbindungsaufbau...";
		case stateConnected: return "OK";
		case stateUnusable: return "Unbenutzbar";
		case stateLost: return "Verbindung unterbrochen";
	}

	assert (false);
	return "";
}

bool DataStorage::stateIsError (DataStorage::State state)
{
	switch (state)
	{
		case stateOffline: return false;
		case stateConnecting: return false;
		case stateConnected: return false;
		case stateUnusable: return true;
		case stateLost: return true;
	}

	assert (false);
	return false;
}

bool DataStorage::ping ()
{
	QMutexLocker dbLock (&databaseMutex);
	// FIXME
//	int result=db.ping ();
	int result=0;
	dbLock.unlock ();

	return result==0;
}

bool DataStorage::isConnectionEstablished () const
{
	State state=getState ();

	return (state==stateConnected || state==stateLost);
}

bool DataStorage::isConnectionAlive () const
{
	return getState ()==stateConnected;
}

// **********
// ** Test **
// **********

bool DataStorage::sleep (OperationMonitor &monitor, int seconds)
{
	std::cout << "DataStorage waiting for " << seconds << " seconds on " << (isGuiThread ()?"the GUI thread":"a background thread") << std::endl;

	int i;
	for (i=0; i<seconds && !monitor.isCanceled (); ++i)
	{
		monitor.progress (i, seconds);
		DefaultQThread::msleep (1000);
	}

	return (i==seconds);
}

// ***************************
// ** Background processing **
// ***************************

void DataStorage::addTask (Task *task)
{
	worker->addTask (task);
}
