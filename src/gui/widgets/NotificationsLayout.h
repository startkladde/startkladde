#ifndef NOTIFICATIONSLAYOUT_H_
#define NOTIFICATIONSLAYOUT_H_

#include <QHash>
#include <QPoint>

#include "src/gui/widgets/NotificationWidget.h"

class NotificationsLayout
{
	public:
		NotificationsLayout ();
		virtual ~NotificationsLayout ();

		void add (NotificationWidget *widget);
		void remove (const NotificationWidget *widget);

		void setArrowPosition (const NotificationWidget *widget, const QPoint &position);
		void setWidgetInvisible (const NotificationWidget *widget);

		void doLayout ();


	private:
		class Node
		{
			public:
				NotificationWidget *widget;
				bool visible;
				QPoint arrowPosition;
		};

		QHash<const NotificationWidget *, Node> _nodes;
};

#endif
