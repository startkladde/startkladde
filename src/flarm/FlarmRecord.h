#ifndef FLARM_RECORD_H
#define FLARM_RECORD_H

#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QQueue>

#include "src/model/Plane.h"

class PflaaSentence;

/**
 * Stores Flarm related data for one plane
 *
 * Note that all numeric data (relative altitude, positions etc.), even data
 * that is received in integer precision from the Flarm, is stored in double
 * precision. This is because most calculations will be performed in double
 * precision and automatic conversion is not possible in all cases. For example,
 * a QVector<QPoint> can not be converted to a QVector<QPointF> automatically.
 * The performance impact of using double instead if integer precision is
 * negligible; for the QVector example, it might even be faster if we can avoid
 * copying the vector.
 */
class FlarmRecord: public QObject
{

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
		QString getFlarmId          () const { return flarmId; }
		double  getRelativeAltitude () const { return relativeAltitude; }
		double  getGroundSpeed      () const { return groundSpeed; }
		double  getClimbRate        () const { return climbRate; }
		QPointF getRelativePosition () const { return relativePosition; }

		QList<QPointF> getPreviousRelativePositions () const { return previousRelativePositions; }

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
		// Primary data (received from Flarm)
		QString flarmId;
		QPointF relativePosition; // <East, North> in meters
		QQueue<QPointF> previousRelativePositions;
		double relativeAltitude, lastRelativeAltitude; // In meters
		double groundSpeed     , lastGroundSpeed;      // In meters per second
		double climbRate       , lastClimbRate;        // In meters per second

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
