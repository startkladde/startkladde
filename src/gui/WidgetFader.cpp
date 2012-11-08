#include <src/gui/WidgetFader.h>

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QWidget>

WidgetFader::WidgetFader (QObject *parent): QObject (parent)
{
}

WidgetFader::~WidgetFader ()
{
}

void WidgetFader::fadeOutAndDelete (QWidget *widget, uint32_t milliseconds)
{
	QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect (widget);
	widget->setGraphicsEffect (effect);

	QPropertyAnimation *animation = new QPropertyAnimation (effect, "opacity", widget);
	QObject::connect (animation, SIGNAL (finished ()), widget, SLOT (deleteLater ()));

	animation->setDuration (milliseconds);
	animation->setStartValue (1);
	animation->setEndValue (0);
	animation->start ();

// FIXME make sure the effect and animation are destroyed => check for destroyed signal
}
