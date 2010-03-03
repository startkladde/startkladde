#include "OperationMonitor.h"

/*
 * TODO:
 *   - rename to Monitor
 *   - integrate Monitor with Returner so we only need to pass one object
 *     - NB: returner is a template, and we want to emit (progress, status) and
 *       receive (cancel) signals
 *     - Maybe use a Listener
 *     - Look at QFuture
 *   - make Returner copyable so we can write Retuner returner=operation ()...
 */

OperationMonitor::OperationMonitor ()
{
}

OperationMonitor::~OperationMonitor ()
{
}
