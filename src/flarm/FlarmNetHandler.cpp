#include <QtNetwork/QNetworkAccessManager>
#include <QtGui/QMessageBox>
#include "FlarmNetHandler.h"
#include "FlarmNetRecord.h"

// ****************************
// ** Construction/singleton **
// ****************************

FlarmNetHandler* FlarmNetHandler::instance = NULL;

FlarmNetHandler::FlarmNetHandler (QObject* parent): QObject (parent)
{
}

FlarmNetHandler::~FlarmNetHandler ()
{
}

FlarmNetHandler* FlarmNetHandler::getInstance ()
{
	if (!instance)
		instance = new FlarmNetHandler (NULL);
        return instance;
}


// ****************
// ** Properties **
// ****************

void FlarmNetHandler::setDatabase (DbManager *db)
{
	dbManager = db;
}

void FlarmNetHandler::importFlarmNetDb () {
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QNetworkAccessManager* network = new QNetworkAccessManager (this);
  network_reply= network->get(QNetworkRequest(QUrl("http://www.flarmnet.org/files/data.fln")));
  connect (network_reply, SIGNAL(finished()), this, SLOT (downloadFinished()));
}

void FlarmNetHandler::downloadFinished () {
  if (network_reply->error() == QNetworkReply::NoError) {
    qDebug () << "QNetworkReply::NoError" << endl;
    // Network connection to www.flarmnet.org is ok

    // Delete old table
    QList<dbId>idList = dbManager->getCache().getFlarmNetRecordIds ();
    if (idList.count() > 0) {
      dbManager->deleteObjects<FlarmNetRecord> (idList, (QWidget*)parent());
      qDebug () << "cache contains elems: " << idList.count() << endl;
    }

    // Einlesen der neuen Datenbank direkt vom Server
    while (!network_reply->atEnd()) {
      QByteArray iline = network_reply->readLine();
      //qDebug () << "iline: " << iline << endl;
      QString oline = "";
      while (!iline.size() == 0) {
        QString byte = iline.left (2);
        iline.remove (0, 2);
        bool ok;
        int character = byte.toInt (&ok, 16);
        if (ok)
          oline += QChar (character);
      }
      //qDebug () << "oline: " << oline << endl;
      FlarmNetRecord record;
      record.setFlarmId      (oline.left(6));
      record.setOwner        (oline.mid (6, 20));
      record.setType         (oline.mid (48, 20));
      record.setRegistration (oline.mid (69, 7));
      record.setCallsign     (oline.mid (76, 3));
      record.setFrequency    (oline.mid (79, 7));
      if (record.getFlarmId().length () == 6 && record.getRegistration().length () > 0) {
        //qDebug () << record.toString() << endl;
        bool success = idValid (dbManager->createObject (record, (QWidget*)parent()));
        
        //if (success) 
        //  qDebug () << "insert ok" << endl;
      }
      else {
        qDebug () << "invalid record" << endl;
      }
    }
    QApplication::restoreOverrideCursor();
    QMessageBox::information (0, "Download", "FlarmNet Datenbank erfolgreich importiert");
  }
  else {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning (0, "Download", "Fehler beim Importieren der FlarmNet Datenbank: " + network_reply->errorString());
  }
}
