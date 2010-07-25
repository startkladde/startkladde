#include "Downloader.h"

//#include <iostream>

#include <QUrl>

#include "src/net/Network.h"
#include "src/util/qString.h"

/*
 * Improvements:
 *   - add a downloadProgress signal
 *   - add a downloadAborted signal
 */

Downloader::Downloader (QObject *parent):
	QObject (parent),
	reply (NULL), currentState (-1)
{
}

Downloader::~Downloader ()
{
	abort ();
}

/**
 * A convenience method to connect the necessary signals to corresponding
 * slots of a receiver
 *
 * The receiver must have the downloadSucceeded and downloadFailed slots.
 *
 * The signals can also be connected manually.
 *
 * @param receiver the receiver of the signals
 */
void Downloader::connectSignals (QObject *receiver)
{
	QObject::connect (
		this    , SIGNAL (downloadSucceeded (int, QNetworkReply *)),
		receiver, SLOT   (downloadSucceeded (int, QNetworkReply *))
		);

	QObject::connect (
		this    , SIGNAL (downloadFailed (int, QNetworkReply *, QNetworkReply::NetworkError)),
		receiver, SLOT   (downloadFailed (int, QNetworkReply *, QNetworkReply::NetworkError))
		);
}

/**
 * Starts a download
 *
 * @param state the state, passed back in the signals
 * @param url the URL to download
 */
void Downloader::startDownload (int state, const QUrl &url)
{
	// Cancel any running downloads
	abort ();

	// Get the manager
	QNetworkAccessManager *manager=Network::getNetworkAccessManager ();

	// Set up the request
	QNetworkRequest request;
	request.setUrl (QUrl (url));

	// Create the reply
	reply=manager->get (request);

	QObject::connect (
		reply, SIGNAL (     finished ()),
		this , SLOT   (replyFinished ())
		);
	QObject::connect (
		reply, SIGNAL (     error (QNetworkReply::NetworkError)),
		this , SLOT   (replyError (QNetworkReply::NetworkError))
		);

	// Set the state
	currentState=state;
}

/**
 * Like #startDownload(int, const QUrl &), but takes the URL as string
 */
void Downloader::startDownload (int state, const QString &url)
{
	startDownload (state, QUrl (url));
}

/**
 * Aborts the current download, if any
 */
void Downloader::abort ()
{
	if (reply)
	{
		// First disconnect, then abort. Otherwise, we may get an
		// "Object::disconnect: Unexpected null parameter" error
		reply->disconnect ();
		reply->abort ();
		reply->deleteLater ();
		reply=NULL;
	}
}

/**
 * Invoked when a download is finished
 */
void Downloader::replyFinished ()
{
	QNetworkReply *r=reply;

	// May happen if the download was aborted or restarted inbetween
	if (sender ()!=reply)
	{
		sender ()->deleteLater ();
		return;
	}

	sender ()->deleteLater ();
	reply=NULL;

	// It is important to first set the reply to NULL and then emit
	// downloadSucceeded because a slot connected to downloadSucceeded
	// may call startDownload.
	emit downloadSucceeded (currentState, r);
}

/**
 * Invoked when a download error occurs
 */
void Downloader::replyError (QNetworkReply::NetworkError code)
{
	QNetworkReply *r=reply;

	// May happen if the download was aborted or restarted inbetween
	if (sender ()!=reply)
	{
		sender ()->deleteLater ();
		return;
	}


	sender ()->deleteLater ();
	reply=NULL;

	emit downloadFailed (currentState, r, code);
}
