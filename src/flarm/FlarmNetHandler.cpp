#include <QtNetwork/QNetworkAccessManager>
#include <QtGui/QMessageBox>
#include <QFile>
#include <QFileDialog>

#include "src/flarm/FlarmNetHandler.h"
#include "src/flarm/FlarmNetRecord.h"
#include "src/flarm/FlarmNetFile.h"
#include "src/db/DbManager.h"
#include "src/text.h"


// ****************************
// ** Construction/singleton **
// ****************************

FlarmNetHandler::FlarmNetHandler (DbManager &dbManager, QWidget *parent): QObject (parent),
	parent (parent), dbManager (dbManager)
{
}

FlarmNetHandler::~FlarmNetHandler ()
{
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
		message=tr ("%1 FlarmNet record(s) were read.", NULL, numGood).arg (numGood)
			+tr (" Do you want to import the records into the database? This"
			" will remove all records and replace them with the new records.");
	else
		// FIXME test
		message=tr ("%1 FlarmNet record(s) were read.", NULL, numGood).arg (numGood)
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
	OperationMonitor monitor ();



//  QApplication::setOverrideCursor(Qt::WaitCursor);
//  QNetworkAccessManager* network = new QNetworkAccessManager (this);
//  network_reply= network->get(QNetworkRequest(QUrl("http://www.flarmnet.org/files/data.fln")));
//  connect (network_reply, SIGNAL(finished()), this, SLOT (downloadFinished()));
}

//void FlarmNetHandler::downloadFinished ()
//{
//	if (network_reply->error() == QNetworkReply::NoError)
//	{
//		// Network connection to www.flarmnet.org is ok
//
//		// Delete old table
//	//    QList<dbId>idList = dbManager->getCache().getFlarmNetRecordIds ();
//	//    if (idList.count() > 0) {
//	//      dbManager->deleteObjects<FlarmNetRecord> (idList, (QWidget*)parent());
//	//      qDebug () << "cache contains elems: " << idList.count() << endl;
//	//    }
//
//		while (!network_reply->atEnd())
//		{
//			QString rawLine=network_reply->readLine().trimmed ();
//			QString decodedLine=FlarmNetFile::decodeLine (rawLine);
//
//			bool ok=false;
//			FlarmNetRecord record=FlarmNetFile::createRecord (decodedLine, &ok);
//
////			if (record.getFlarmId().length () == 6 && record.getRegistration().length () > 0)
////			{
////				//qDebug () << record.toString() << endl;
////				bool success = idValid (dbManager->createObject (record, (QWidget*)parent()));
////
////				//if (success)
////				//  qDebug () << "insert ok" << endl;
////			}
////			else
////			{
////				qDebug () << "invalid record" << endl;
////			}
//		}
//		QMessageBox::information (0, "Download", "FlarmNet Datenbank erfolgreich importiert");
//	}
//	else
//	{
//		QMessageBox::warning (0, "Download", "Fehler beim Importieren der FlarmNet Datenbank: " + network_reply->errorString());
//	}
//}
