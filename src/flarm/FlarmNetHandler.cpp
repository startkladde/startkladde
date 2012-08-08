#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include "src/text.h"
#include "src/concurrent/monitor/SignalOperationMonitor.h"
#include "src/db/DbManager.h"
#include "src/flarm/FlarmNetHandler.h"
#include "src/flarm/FlarmNetRecord.h"
#include "src/flarm/FlarmNetFile.h"
#include "src/gui/windows/MonitorDialog.h"


static const char *flarmNetFileUrl="http://www.flarmnet.org/files/data.fln";

// ****************************
// ** Construction/singleton **
// ****************************

FlarmNetHandler::FlarmNetHandler (DbManager &dbManager, QWidget *parent): QObject (parent),
	parent (parent), dbManager (dbManager), operationMonitorInterface (NULL)
{
	monitor=new SignalOperationMonitor ();
	downloader.connectSignals (this);

	connect (monitor, SIGNAL (canceled ()), this, SLOT (abort ()));
}

FlarmNetHandler::~FlarmNetHandler ()
{
	delete monitor;
	delete operationMonitorInterface;
}


// ***************
// ** Importing **
// ***************

void FlarmNetHandler::interactiveImport (QList<FlarmNetRecord> &records)
{
	// Delete all FlarmNet records
	// TODO there should be a DbManager::deleteAllObjects
	QList<dbId> idList = dbManager.getCache ().getFlarmNetRecordIds ();
	if (idList.count () > 0)
		dbManager.deleteObjects<FlarmNetRecord> (idList, parent);

	dbManager.createObjects (records, parent);
}

void FlarmNetHandler::interactiveImport (const QByteArray &data)
{
	int numGood=0, numBad=0;
	QList<FlarmNetRecord> records=FlarmNetFile::createRecordsFromFile (data, &numGood, &numBad);

	QString message;
	if (numBad==0)
		message=tr ("%1 FlarmNet record(s) were found.", NULL, numGood).arg (numGood)
			+tr (" Do you want to import the records into the database? This"
			" will remove all records and replace them with the new records.");
	else
		// FIXME test
		message=tr ("%1 FlarmNet record(s) were found.", NULL, numGood).arg (numGood)
			+tr ("Additionally, %1 invalid record(s) were found.", NULL, numBad).arg (numBad)
			+tr (" Do you want to import the records into the database? This"
			" will remove all records and replace them with the new records."
			" The invalid entries will be ignored.");

	QMessageBox::StandardButton button=QMessageBox::question (
		parent,
		tr ("Import FlarmNet database"), message,
		QMessageBox::Ok | QMessageBox::Cancel);

	if (button==QMessageBox::Ok)
	{
		interactiveImport (records);
	}
}

void FlarmNetHandler::interactiveImportFromFile ()
{
	QString fileName=QFileDialog::getOpenFileName (parent,
		tr ("Select .fln file"), ".", notr ("*.fln"), NULL, 0);

	if (!fileName.isEmpty ())
	{
		QFile file (fileName);
		if (file.open (QIODevice::ReadOnly))
		{
			// Read the whole file
			QByteArray data=file.readAll ();
			interactiveImport (data);
		}
		else
		{
			QMessageBox::warning (parent, tr ("Error opening file"), file.errorString ());
			return;
		}
	}
}

void FlarmNetHandler::interactiveImportFromWeb ()
{
	// Store a copy of the operation monitor interface locally
	operationMonitorInterface=new OperationMonitorInterface (monitor->interface ());
	operationMonitorInterface->progress (0, 0, tr ("Downloading"), true);

	// Start the download
	downloadSuccess=false; downloadData.clear ();
	downloadAborted=false;
	downloadFailure=false; downloadErrorString.clear ();

	downloader.startDownload (0, flarmNetFileUrl);

	// Open a monitor dialog. This call is blocking and executes a local event
	// loop. Execution will resume after the monitor has finished when the
	// download succeeds, fails or is aborted.
	MonitorDialog::monitor (*monitor, tr ("Importing FlarmNet database"), parent);

	if (downloadSuccess)
		interactiveImport (downloadData);
	else if (downloadFailure)
		QMessageBox::critical (parent, "Download error", downloadErrorString);
	else
		{} // Probably aborted, do nothing
}

void FlarmNetHandler::downloadSucceeded (int state, QNetworkReply *reply)
{
	Q_UNUSED (state);

	downloadSuccess=true;
	downloadData=reply->readAll ();

	finishProgress ();
}

void FlarmNetHandler::downloadFailed (int state, QNetworkReply *reply, QNetworkReply::NetworkError code)
{
	Q_UNUSED (state);
	Q_UNUSED (code);

	downloadFailure=true;
	downloadErrorString=reply->errorString ();

	finishProgress ();
}

void FlarmNetHandler::abort ()
{
	downloadAborted=true;

	downloader.abort ();
	finishProgress ();
}

void FlarmNetHandler::finishProgress ()
{
	// When the (last instance of the) interface is deleted, this is counted as
	// "ended" automatically.
	delete operationMonitorInterface;
	operationMonitorInterface=NULL;
}
