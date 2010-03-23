#include "AcpiWidget.h"

/**
  * AcpiWidget dummy implementation (for systems without libacpi)
  */

AcpiWidget::AcpiWidget (QWidget* parent):
	QLabel (parent)
{
}

bool AcpiWidget::valid ()
{
	return false;
}

void AcpiWidget::slotTimer()
{
}
