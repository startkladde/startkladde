#ifndef FLARMHANDLING_H_
#define FLARMHANDLING_H_

#include "src/db/dbId.h"

#include <QObject>

class QWidget;
class DbManager;

/**
 * For want of a better name - this class contains some methods for handling
 * flights and planes, as related to Flarm.
 */
class FlarmHandling//: public QObject
{
//		Q_OBJECT

	public:
		FlarmHandling ();
		virtual ~FlarmHandling ();

		static dbId interactiveIdentifyPlane (QWidget *parent, DbManager &dbManager, dbId flightId);



};

#endif
