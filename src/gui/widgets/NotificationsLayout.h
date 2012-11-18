#ifndef NOTIFICATIONSLAYOUT_H_
#define NOTIFICATIONSLAYOUT_H_

#include <QHash>
#include <QPointF>

#include "src/gui/widgets/NotificationWidget.h"

class NotificationsLayout
{
	public:
		NotificationsLayout ();
		virtual ~NotificationsLayout ();

		void add (NotificationWidget *widget);
		void remove (const NotificationWidget *widget);

		void setWidgetPosition (const NotificationWidget *widget, const QPointF &position);
		void setWidgetInvisible (const NotificationWidget *widget);

		void layout ();


	private:
		class Node
		{
			public:
				NotificationWidget *widget;
				bool visible;
				QPointF arrowPosition;
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
