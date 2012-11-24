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

		void setWidgetPosition (const NotificationWidget *widget, const QPoint &position);
		void setWidgetInvisible (const NotificationWidget *widget);

		void layout ();


	private:
		class Node
		{
			public:
				NotificationWidget *widget;
				bool visible;
				QPoint arrowPosition;
		};

		class LayoutNode
		{
			public:
				Node *node;
				double y;
				double h;
				bool operator< (const LayoutNode &other) const { return y<other.y; }
		};

		double overlapValue (QList<LayoutNode> &nodes, int index, int spacing);
		int largestOverlap (QList<LayoutNode> &nodes, int spacing);
		void doLayout (QList<LayoutNode> &nodes);

		QHash<const NotificationWidget *, Node> _nodes;
};

#endif
