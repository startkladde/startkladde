/*
 * Monitor.h
 *
 *  Created on: 28.02.2010
 *      Author: Martin Herrmann
 */

#ifndef MONITOR_H_
#define MONITOR_H_

#include <QObject>

#include "src/db/event/Event.h"

/**
 * Implements the listener pattern for signals with a Db::Event::Event
 * parameter.
 *
 * This is useful for template classes which are to react to database events
 * because template classes cannot be QObjects and thus cannot define slots.
 * An alternative solution may be to use a QObject base class to the template
 * class, but that usually involves diamond inheritance.
 */
namespace Db
{
	namespace Event
	{
		class Monitor: public QObject
		{
			Q_OBJECT

			public:
				// *** Types
				class Listener
				{
					public:
						virtual void dbEvent (Event event)=0;
				};

				// *** Construction
				Monitor (QObject &source, const char *signal, Listener &listener);
				virtual ~Monitor ();

			public slots:
				virtual void dbEvent (Db::Event::Event event);

			private:
				Listener &listener;
		};
	}
}

#endif
