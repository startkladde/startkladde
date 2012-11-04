#ifndef FLARMIDUPDATE_H_
#define FLARMIDUPDATE_H_

#include <QMessageBox>

#include "src/db/dbId.h"

class DbManager;
class QWidget;
class Flight;
class Plane;

class FlarmIdUpdate
{
	public:
		FlarmIdUpdate (DbManager &dbManager, QWidget *parent);
		virtual ~FlarmIdUpdate ();

		bool interactiveUpdateFlarmId (const Flight &flight, bool manualOperation, dbId oldPlaneId);

	protected:
		enum UpdateAction { update, dontUpdate, cancel };

		void notCreatedAutomaticallyMessage ();
		void noPlaneMessage ();
		void currentMessage ();

		UpdateAction queryUpdateFlarmId (const Plane &plane, const Flight &flight);

		bool canUpdateSilently (const Plane &plane, const Flight &flight);
		bool checkAndUpdate (Plane &plane, const Flight &flight);


	private:
		DbManager &dbManager;
		QWidget *parent;

		bool manualOperation;
		dbId oldPlaneId;
};

#endif
