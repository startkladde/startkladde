#ifndef WIDGETFADER_H_
#define WIDGETFADER_H_

#include <stdint.h>

#include <QObject>

class QWidget;

class WidgetFader: public QObject
{
		Q_OBJECT

	public:
		virtual ~WidgetFader ();

		static void fadeOutAndDelete (QWidget *widget, uint32_t milliseconds);

	private:
		WidgetFader (QObject *parent);

};

#endif
