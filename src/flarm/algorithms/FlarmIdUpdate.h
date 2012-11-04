#ifndef FLARMIDUPDATE_H_
#define FLARMIDUPDATE_H_

#include <QMessageBox>

class DbManager;
class QWidget;
class Flight;
class Plane;

class FlarmIdUpdate
{
	public:
		FlarmIdUpdate (DbManager &dbManager, bool manualOperation, QWidget *parent);
		virtual ~FlarmIdUpdate ();

		bool interactiveUpdateFlarmId (const Flight &flight);

	protected:
		void notCreatedAutomaticallyMessage ();
		void noPlaneMessage ();
		void currentMessage ();

		QMessageBox::StandardButton queryUpdateFlarmId (const Plane &plane, const Flight &flight);

		bool checkAndUpdate (Plane &plane, const Flight &flight);


	private:
		DbManager &dbManager;
		bool manualOperation;
		QWidget *parent;

};

#endif
