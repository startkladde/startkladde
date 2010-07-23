/*
 * Downloader.h
 *
 *  Created on: 23.07.2010
 *      Author: martin
 */

#ifndef DOWNLOADER_H_
#define DOWNLOADER_H_

#include <QObject>
#include <QNetworkReply> // Required for QNetworkReply::NetworkError

class QString;

/**
 * You need the following slots:
 *    void downloadSucceeded (int state, QNetworkReply *reply);
 *    void downloadFailed    (int state, QNetworkReply *reply, QNetworkReply::NetworkError code);
 */
class Downloader: public QObject
{
		Q_OBJECT

	public:
		Downloader (QObject *parent=NULL);
		virtual ~Downloader ();

		void connect (QObject *receiver);

	public slots:
		void startDownload (int state, const QString &url);
		void abort ();

	signals:
		void downloadSucceeded (int state, QNetworkReply *reply);
		void downloadFailed    (int state, QNetworkReply *reply, QNetworkReply::NetworkError code);

	private slots:
		void replyFinished ();
		void replyError (QNetworkReply::NetworkError code);

	private:
		QNetworkReply *reply;
		int currentState;
};

#endif
