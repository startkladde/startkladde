#ifndef FLARMNETHANDLER_H
#define FLARMNETHANDLER_H

#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>

#include "src/net/Downloader.h"

class DbManager;
class FlarmNetRecord;
class QByteArray;
class SignalOperationMonitor;
class MonitorDialog;
class OperationMonitorInterface;

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

	protected slots:
		void downloadSucceeded (int state, QNetworkReply *reply);
		void downloadFailed    (int state, QNetworkReply *reply, QNetworkReply::NetworkError code);
		void abort ();

	private:
		void finishProgress ();

		QWidget *parent;
		DbManager &dbManager;
		Downloader downloader;

		SignalOperationMonitor *monitor; // FIXME need this?
		OperationMonitorInterface *operationMonitorInterface;

		bool downloadSuccess;
		bool downloadAborted;
		bool downloadFailure;
		QByteArray downloadData;
		QString downloadErrorString;
};

#endif
