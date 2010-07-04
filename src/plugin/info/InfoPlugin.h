/*
 * InfoPlugin.h
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#ifndef INFOPLUGIN_H_
#define INFOPLUGIN_H_

#include "src/plugin/Plugin.h"

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
				// FIXME read all at once
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

		const QString &getCaption () const;
		void setCaption (const QString &caption);

		const QString &getText () const;


		// **************
		// ** Settings **
		// **************

		virtual void loadSettings (const QSettings &settings);
		virtual void saveSettings (QSettings &settings);

	signals:
		void textOutput (const QString &);

	protected:
		void outputText (const QString &text);

	private:
		QString caption;
		QString text;
};

#endif
