#ifndef ACPIWIDGET_H_
#define ACPIWIDGET_H_

/**
* ACPI battery status display
* eggert.ehmke@berlin.de
* 10.09.2008
*
* Generalized: Martin Herrmann, 3/2010
*/

#include "src/gui/widgets/SkLabel.h"

class QTimer;

class AcpiWidget: public SkLabel
{
	Q_OBJECT

	public:
		AcpiWidget (QWidget* parent);

		static bool valid ();

	private:
		QTimer* timer;

	private slots:
		void slotTimer();
};

#endif
