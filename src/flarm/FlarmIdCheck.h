#ifndef FLARMIDCHECK_H_
#define FLARMIDCHECK_H_

#include <QObject>
#include <QList>
#include <QString>

#include "src/model/Plane.h"

class DbManager;
class QWidget;

class FlarmIdCheck: public QObject
{
		Q_OBJECT

	public:
		FlarmIdCheck (DbManager &dbManager, QWidget *parent);
		virtual ~FlarmIdCheck ();

		bool interactiveCheck (const QString &newFlarmId, const QString &oldFlarmId);
		bool interactiveApply (QString *flightFlarmId);

		bool keepOldFlarmId ();

	protected:
		// The reaction to a conflict
		enum Reaction
		{
			clear,  // Set the other plane's Flarm ID to ""
			swap,   // Set the other plane's Flarm ID this plane's old Flarm ID
			ignore, // Ignore the conflict, both planes will have the same Flarm ID
			keep,   // Keep this plane's old Flarm ID (cancel the Flarm ID update)
			cancel  // Cancel the whole operation
		};

		// A textual representation of a (possible) reaction for displaying to
		// the user.
		static QString Reaction_getText (Reaction reaction, const Plane &conflictingPlane, const QString &newFlarmId, const QString &oldFlarmId);


		// A set of reactions presented to a user in case of a conflict. The set
		// of reactions as wells as the default (preselected) reaction depend on
		// the type of conflict.
		class Options
		{
			public:
				QList<Reaction> reactions;
				Reaction defaultReaction;
		};

		Options getOptions ();
		Reaction showChoiceDialog (const Options &options);

	private:
		DbManager &dbManager;
		QWidget *parent;

		// Input data
		QString newFlarmId;
		QString oldFlarmId;

		// Status
		bool conflict;             // Is there a conflict at all
		Plane conflictingPlane;    // The conflicting plane if there is a conflict
		Reaction selectedReaction; // The reaction chosen by the user

};

#endif
