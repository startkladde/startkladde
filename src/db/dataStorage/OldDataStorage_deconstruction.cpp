/*
 * Multithreading TODO:
 * TODO: Potential problem: refresh, cancel, refresh. First refresh still
 *       running. Can the second one be started? May the first one hang even
 *       though the database comes back online?
 * TODO: when to delete tasks that take some time to cancel
 * TODO: add Task* parameter to signals (but connection to dialog?)
 * TODO: fortune example:
 *   - thread destructor: quit = true; cond.wakeOne(); wait();
 *   - invoking: if (!isRunning()) start(); else cond.wakeOne();
 * TODO: test case: port open, but connection not accepted
 * TODO: display error, db->get_last_error
 *
 * TODO: handle unusable database
 *
 * TODO: the reconnecting (1 second delay) and associated error handling is
 * done in several places
 * TODO: all database actions should retry until success or canceled.
 * TODO: on timeout in an operation (other then the idle ping), the state
 * should be updated immediately - same for a succeeded query
 */

/*
 * On multithreaded database access:
 *   - Some operations may take some time, especially over a slow network
 *     connection. Examples:
 *       - refresh cache, fetch flights for a given date (long operation even
 *         with local storage)
 *       - write to the database (create, update, delete)
 *   - Some operations cannot be interrupted, for example a MySQL call waiting
 *     for a timeout due to a non-working network connection
 *   - We want to be able to cancel such operations. For this, we must have a
 *     responsive GUI, so the GUI thread must be running
 *   - Typically, the function initiating a long operation will not continue
 *     (which also means that it will not return) until after the task is
 *     completed (or failed or aborted), as the next action may depend on the
 *     result of the task
 *
 * Implementation of multithreaded database access:
 *   - operation are performed by a Task
 *   - the DataStorage has a WorkerThread that performs Tasks sequentially on
 *     a background thread
 *   - the database access functionality is in DataStorage; the tasks call the
 *     corresponding method of DataStorage, passing a pointer the Task as an
 *     OperationMonitor
 *   - a dialog for monitoring and canceling Tasks is provided by
 *     TaskProgressDialog
 */


/*
 * TODO:
 *   - prepared flights
 *   - this must probably have some concept of an "active date", so we can also
 *     edit flights and create logs for past days.
 *   - use QSet where appropriate, but sort case insensitively
 *   - notify the main window about changes (old db_change mechanism?)
 *   - log an error if an invalid ID is passed to the get by ID functions
 *   - addObject error message when getX (maybe also getNewX) is called with an
 *     invalid ID, this should be checked by the caller
 *   - don't delete an entity that is still in use; potentially addObject a force
 *     flag
 *   - For retrieving flights of other days, we need a way to say "Get this",
 *     then wait until it finished, and have a way of aborting it or signalling
 *     error.
 *   - club list should not include ""/whitespace only; Locations dito
 */







