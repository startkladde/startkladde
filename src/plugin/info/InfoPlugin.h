/*
 * InfoPlugin.h
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#ifndef INFOPLUGIN_H_
#define INFOPLUGIN_H_

#include "src/plugin/Plugin.h"
#include "src/accessor.h"

class InfoPlugin: public Plugin
{
	Q_OBJECT

	public:
		// ***********
		// ** Types **
		// ***********

		class Descriptor
		{
			public:
				virtual InfoPlugin *create () const=0;
				virtual QString getId          () const=0;
				virtual QString getName        () const=0;
				virtual QString getDescription () const=0;
		};

		template<class T> class DefaultDescriptor: public Descriptor
		{
			public:
				virtual InfoPlugin *create () const { return new T (); }
				// FIXME read all at once, or only when required?
				virtual QString getId          () const { return T ().getId          (); }
				virtual QString getName        () const { return T ().getName        (); }
				virtual QString getDescription () const { return T ().getDescription (); }
		};


		// ******************
		// ** Construction **
		// ******************

		InfoPlugin ();
		virtual ~InfoPlugin ();


		// **********************
		// ** Property Acceess **
		// **********************

		value_accessor (QString, Caption, caption)
		bool_accessor (Enabled, enabled)


		// **************
		// ** Settings **
		// **************

		virtual void readSettings (const QSettings &settings);
		virtual void writeSettings (QSettings &settings);

	signals:
		void textOutput (const QString &);

	protected:
		void outputText (const QString &text);

	private:
		QString caption;
		bool enabled;
};

#endif
