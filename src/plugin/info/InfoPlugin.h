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
				virtual QString getName             () const=0;
				virtual QString getShortDescription () const=0;
				virtual QString getLongDescription  () const=0;
		};

		template<class T> class DefaultDescriptor: public Descriptor
		{
			public:
				virtual InfoPlugin *create () const { return new T (); }
				virtual QString getName             () const { return T::getName (); }
				virtual QString getShortDescription () const { return T::getShortDescription (); }
				virtual QString getLongDescription  () const { return T::getLongDescription (); }
		};


		// ******************
		// ** Construction **
		// ******************

		InfoPlugin ();
		virtual ~InfoPlugin ();


		// **********************
		// ** Property Acceess **
		// **********************

		const QString &getTitle () const;
		void setTitle (const QString &title);

		const QString &getText () const;

	signals:
		void textOutput (const QString &);

	protected:
		void outputText (const QString &text);

	private:
		QString title;
		QString text;
};

#endif
