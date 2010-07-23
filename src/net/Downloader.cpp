#include "Downloader.h"

#include "src/net/Network.h"
#include "src/util/qString.h"

// FIXME cleanup

Downloader::Downloader (QObject *parent):
	QObject (parent),
	reply (NULL), currentState (-1)
{
}

Downloader::~Downloader ()
{
	abort ();
}

void Downloader::connect (QObject *receiver)
{
	QObject::connect (
		this    , SIGNAL (downloadFailed (int, QNetworkReply *, QNetworkReply::NetworkError)),
		receiver, SLOT   (downloadFailed (int, QNetworkReply *, QNetworkReply::NetworkError))
		);

	QObject::connect (
		this    , SIGNAL (downloadSucceeded (int, QNetworkReply *)),
		receiver, SLOT   (downloadSucceeded (int, QNetworkReply *))
		);
}

void Downloader::startDownload (int state, const QString &url)
{
	std::cout << "Start download from " << url << std::endl;

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

	std::cout << "reply is now " << reply << std::endl;
}


void Downloader::abort ()
{
	if (reply)
	{
		reply->abort ();
		reply->deleteLater ();
		reply=NULL;
	}
}

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
