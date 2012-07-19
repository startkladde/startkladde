#ifndef FLARMNETRECORD_H_
#define FLARMNETRECORD_H_

#include <QApplication>
#include <QString>
#include <QList>
#include <QMetaType>

#include "src/model/objectList/ObjectModel.h"
#include "src/model/Entity.h"

class FlarmNetRecord: public Entity
{
	public:

		class DefaultObjectModel: public ObjectModel<FlarmNetRecord>
		{
			public:
				virtual int columnCount () const;
				virtual QVariant displayHeaderData (int column) const;
				virtual QVariant displayData (const FlarmNetRecord &object, int column) const;
		};


		// *** Construction
		FlarmNetRecord ();
		FlarmNetRecord (dbId id);

		QString getFlarmId () const;
		QString getRegistration () const;
		QString getOwner () const;
		QString getType () const;
		QString getCallsign () const;
		QString getFrequency () const;

		void setFlarmId (const QString& flarmId);
		void setRegistration (const QString& registration);
		void setOwner (const QString& owner);
		void setType  (const QString& type);
		void setCallsign (const QString& callsign);
		void setFrequency (const QString& frequency);
		
		// *** Formatting
		virtual QString toString () const;
		virtual QString getDisplayName () const;

		// *** ObjectListWindow/ObjectEditorWindow helpers
		static QString objectTypeDescription () { return qApp->translate ("FlarmNetRecord", "flarmnet record"); }
		static QString objectTypeDescriptionDefinite () { return qApp->translate ("FlarmNetRecord", "the flarmnet record"); }
		static QString objectTypeDescriptionPlural () { return qApp->translate ("FlarmNetRecord", "flarmnet records"); }

		// SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static FlarmNetRecord createFromResult (const Result &result);
		static QString insertColumnList ();
		static QString insertPlaceholderList ();
		virtual void bindValues (Query &q) const;
		static QList<FlarmNetRecord> createListFromResult (Result &query);

	private:
		void initialize ();
		// *** Data
		QString flarmId;
		QString registration;
		QString owner;
		QString type;
		QString callsign;
		QString frequency;

};

Q_DECLARE_METATYPE (FlarmNetRecord);

#endif
