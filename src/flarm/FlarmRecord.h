#ifndef FLARM_RECORD_H
#define FLARM_RECORD_H

#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QQueue>

#include "src/model/Plane.h"

class PflaaSentence;

class FlarmRecord: public QObject {

		Q_OBJECT

	public:
		// Types
		enum flarmState {stateUnknown, stateOnGround, stateStarting, stateFlying, stateFlyingFar, stateLanding};
		enum FlightSituation {groundSituation, lowSituation, flyingSituation};
		enum FlightAction {departure, landing, goAround};

		// Construction
		FlarmRecord (QObject* parent, const QString &flarmId);
		virtual ~FlarmRecord ();

		// Properties
		QString getFlarmId     () const { return flarmId; }
		int     getAlt         () const { return altitude; }
		int     getGroundSpeed () const { return groundSpeed; }
		double  getClimb       () const { return climbRate; }
		int     getNorth       () const { return north; }
		int     getEast        () const { return east; }

		QList<QPointF> getPreviousPositions () const { return previousPositions; }

		flarmState getState () const { return state; }

		QString getRegistration () { return registration; }

		void setRegistration (const QString &registration);
		void setCategory (Plane::Category category);


		// Misc
		QString getStateText () const;
		bool isPlausible () const;
		void processPflaaSentence (const PflaaSentence &sentence);
		static QString flightActionToString (FlightAction action);

	protected:
		FlightSituation getSituation () const;

	private:
		// Primary data
		QString flarmId;
		int    altitude   , lastAltitude;
		int    groundSpeed, lastGroundSpeed;
		double climbRate  , lastClimbRate;
		int north, east;

		// History
		QQueue<QPointF> previousPositions;

		// Derived data
		flarmState state;

		// Database data
		QString registration;
		Plane::Category category;
		QString frequency;

		QTimer* keepaliveTimer;
		QTimer* landingTimer;

		void setState (flarmState state);

	private slots:
		void keepaliveTimeout ();
		void landingTimeout ();

	signals:
	void actionDetected (const QString& id, FlarmRecord::FlightAction);
};

#endif
