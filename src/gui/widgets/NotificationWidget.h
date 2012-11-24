#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QWidget>
#include <QPainterPath>

#include "src/util/math.h"

class QEvent;
class QCloseEvent;
class QMouseEvent;
class QPaintEvent;

class NotificationWidget: public QWidget
{
		Q_OBJECT

	public:
		// Construction
		explicit NotificationWidget (QWidget *parent, Qt::WindowFlags f=0);
		~NotificationWidget ();

		// Contents
		void setContents (QWidget *contents);
		QWidget *contents () const  { return _contents; }
		void setText (const QString &text);
		QString text () const;

		// Parameters
		void setArrowWidth   (int arrowWidth  );
		void setCornerRadius (int cornerRadius);
		void setMargin       (int margin      );

		int arrowWidth   () const { return _arrowWidth;   }
		int cornerRadius () const { return _cornerRadius; }
		int margin       () const { return _margin;       }

		// Position
		QPoint defaultBubblePosition (const QPoint &arrowTip) const;
		void moveTo (const QPoint &arrowTip, const QPoint &bubblePosition);
		void moveTo (const QPoint &arrowTip);

		// Layout
		virtual QSize sizeHint () const;

		// Geometry
		// Including the bubble margins
		QRect bubbleGeometry () const { return QRect (bubblePosition (), bubbleSizeHint ()); }
		QRect bubbleGeometryParent () const { return bubbleGeometry ().translated (pos ()); }

	signals:
		void closed ();
		void clicked ();

	protected:
		// Layout
		virtual void invalidate ();
		virtual void doLayout ();

		// Depends only on the parameters - in widget coordinates
		int top    () const { return ifPositive (-_arrowTipFromBubblePosition.y ()); }
		int left   () const { return ifPositive (-_arrowTipFromBubblePosition.x ()); }
		int bottom () const { return ifPositive ( _arrowTipFromBubblePosition.y ()); }
		int right  () const { return ifPositive ( _arrowTipFromBubblePosition.x ()); }
		QPoint arrowTip       () const { return QPoint (right (), bottom ()); }
		QPoint bubblePosition () const { return QPoint (left  (), top    ()); }
		QSize minimumBubbleSize () const { return QSize (2*_cornerRadius, 2*_cornerRadius+_arrowWidth); }

		// Depends only on the parameters and the contents
		QSize bubbleSizeHint () const;

		// Depends on the actual layout
		QPainterPath path ();

		// Qt events
		virtual bool event              (QEvent       *e);
		virtual void closeEvent         (QCloseEvent  *e);
		virtual void layoutRequestEvent ();
		virtual void mousePressEvent    (QMouseEvent  *e);
		virtual void paintEvent         (QPaintEvent  *e);


	private:
		// Contents
		QWidget *_contents;

		// Colors
		QColor bubbleColor;

		// Geometry parameters - set by the user
		int _arrowWidth;
		int _cornerRadius;
		int _margin;
		QPoint _arrowTipFromBubblePosition; // NB!

		// The actual geometry - known after doing the layout
		QPainterPath _path_;

};

#endif
