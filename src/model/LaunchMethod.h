#ifndef LAUNCHMETHOD_H_
#define LAUNCHMETHOD_H_

#include <QString>
#include <QStringList>

#include "src/model/Entity.h"
#include "src/model/objectList/ObjectModel.h"

class QSqlQuery;

class LaunchMethod: public Entity
{
	public:
		// *** Types
		enum Type { typeWinch, typeAirtow, typeSelf, typeOther };

		class DefaultObjectModel: public ObjectModel<LaunchMethod>
		{
			virtual int columnCount () const;
			virtual QVariant displayHeaderData (int column) const;
			virtual QVariant displayData (const LaunchMethod &object, int column) const;
		};


		// *** Construction
		LaunchMethod ();
		LaunchMethod (dbId id);
		static LaunchMethod parseConfigLine (QString line);


		// *** Data
		QString name; // The name displayed in the GUI
		QString shortName; // The name displayed in tables
		QString logString; // The value for the pilot log
		QString keyboardShortcut; // Should be unique
		Type type;
		QString towplaneRegistration; // For type==typeAirtow
		bool personRequired;


		// *** Property access
		virtual bool isAirtow () const { return type==typeAirtow; }
		virtual bool towplaneKnown () const { return !towplaneRegistration.isEmpty (); }


		// *** Formatting
		virtual QString toString () const;
		virtual QString nameWithShortcut () const;
		static bool nameLessThan (const LaunchMethod &l1, const LaunchMethod &l2) { return l1.name<l2.name; }


		// *** Type methods
		static QString typeString (Type type);
		static QList<Type> listTypes ();


		// *** EntitySelectWindow helpers
		virtual QString get_selector_value (int column_number) const;
		static QString get_selector_caption (int column_number);


		// *** ObjectListWindow/ObjectEditorWindow helpers
		static QString objectTypeDescription () { return "Startart"; }
		static QString objectTypeDescriptionDefinite () { return "die Startart"; }
		static QString objectTypeDescriptionPlural () { return "Startarten"; }


		// *** SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static LaunchMethod createFromQuery (const QSqlQuery &query);
		static QString insertValueList ();
		static QString updateValueList ();
		virtual void bindValues (QSqlQuery &q) const;
		static QList<LaunchMethod> createListFromQuery (QSqlQuery &query);
		// Enum mappers
		static QString typeToDb   (Type    type);
		static Type    typeFromDb (QString type);

	private:
		void initialize ();
};

#endif
