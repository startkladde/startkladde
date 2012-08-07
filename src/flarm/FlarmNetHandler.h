#ifndef FLARMNETHANDLER_H
#define FLARMNETHANDLER_H

#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>

class parent;
class DbManager;
class FlarmNetRecord;
class QByteArray;

class FlarmNetHandler: public QObject
{
	Q_OBJECT

	public:
		// *** Construction
		FlarmNetHandler (DbManager &dbManager, QWidget *parent);
		~FlarmNetHandler ();

		// *** Importing
		void interactiveImportFromFile ();
		void interactiveImportFromWeb ();

	protected:
		void interactiveImport (const QByteArray &data);
		void interactiveImport (QList<FlarmNetRecord> &records);

	private:
		QWidget *parent;
		DbManager &dbManager;

//		QNetworkReply* network_reply;
//
//	private slots:
//		void downloadFinished ();
};

#endif
