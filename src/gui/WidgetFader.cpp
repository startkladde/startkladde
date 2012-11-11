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

void WidgetFader::fadeOutAndClose (QWidget *widget, uint32_t milliseconds)
{
	// Create the opacity effect and assign it to the widget. The effect will
	// be deleted when the widget (which is the effect's parent) is deleted,
	// when a new effect is assigned to the widget, or after the animation ends.
	QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect (widget);
	widget->setGraphicsEffect (effect);

	// Create the animation for the opacity. The animation will be deleted when
	// the widget (which is the animation's parent) is is deleted, or after the
	// animation ends.
	QPropertyAnimation *animation = new QPropertyAnimation (effect, "opacity", widget);

	// Close the widget at the end of the animation. The widget may choose to
	// ignore the close event.
	QObject::connect (animation, SIGNAL (finished ()), widget, SLOT (close ()));

	// Delete the effect and the animation after the animation finishes.
	QObject::connect (animation, SIGNAL (finished ()), effect   , SLOT (deleteLater ()));
	QObject::connect (animation, SIGNAL (finished ()), animation, SLOT (deleteLater ()));

	// Setup and start the animation
	animation->setDuration (milliseconds);
	animation->setStartValue (1);
	animation->setEndValue (0);
	animation->start ();

// FIXME verify that the effect and the animation are destroyed => check for destroyed signal
}
