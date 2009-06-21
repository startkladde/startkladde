#include "AcpiWidget.h"

#include <qdebug.h>
#include <qpalette.h>
#include <qcolor.h>

extern "C" {
#include <libacpi.h>
}

/**
  * wrapper class for libacpi
  * eggert.ehmke@berlin.de
  * 10.09.2008
  */


/**
  * the global structure is _the_ acpi structure here
  */
static global_t global_acpi;

AcpiWidget::AcpiWidget (QWidget* parent)
	:QLabel (parent)
{
	setAutoFillBackground (true);
	timer = new QTimer (this);
	// 10 second period
	timer->start (10000);
	connect (timer, SIGNAL(timeout()), this, SLOT (slotTimer()));
	// show widget immediatly
	slotTimer();
}

bool AcpiWidget::valid ()
{
	if(check_acpi_support() == NOT_SUPPORTED){
		qDebug () << "No acpi support for your system?" << endl;
		return false;
	}
	if (init_acpi_batt(&global_acpi) != SUCCESS && init_acpi_acadapt(&global_acpi) != SUCCESS)
		return false;
	return true;
}

void AcpiWidget::slotTimer()
{
	adapter_t *ac = &global_acpi.adapt;

	QString message;
	QPalette palette;

	if(check_acpi_support() == NOT_SUPPORTED){
		qDebug () << "No acpi support for your system?" << endl;
		return;
	}

	/* initialize battery, thermal zones, fans and ac state */
	int battstate = init_acpi_batt(&global_acpi);
	int acstate = init_acpi_acadapt(&global_acpi);

	if(acstate == SUCCESS && ac->ac_state == P_BATT) {
		message = "Batterie: ";
		palette.setColor(QPalette::Window, Qt::red);
	}
	else if(acstate == SUCCESS && ac->ac_state == P_AC) {
		message = "Extern; ";
		palette.setColor (QPalette::Window, Qt::white);
	}
	else {
		message = "Nicht bekannt; ";
		palette.setColor(QPalette::Window, Qt::red);
	}
	setPalette(palette);

	if(battstate == SUCCESS){
		for(int i=0;i<global_acpi.batt_count;i++){
			battery_t *binfo = &batteries[i];
			/* read current battery values */
			read_acpi_batt(i);

			if(binfo->present)
				if (ac->ac_state == P_BATT)
					message += QString ("%1% %2:%3").arg(binfo->percentage).arg(binfo->remaining_time/60).arg(binfo->remaining_time%60, 2, 10, QChar('0'));
				else
					message += QString ("%1%").arg(binfo->percentage);
				QString tooltip = QString("%1: %2mAh\n").arg(binfo->name).arg(binfo->design_cap);
				tooltip += QString("%1mAh / %2mAh\n").arg(binfo->last_full_cap).arg(binfo->remaining_cap);
				tooltip += QString("%1mV / %2mV\n").arg(binfo->design_voltage).arg(binfo->present_voltage);
/*
				printf("\n%s:\tpresent: %d\n"
						"\tdesign capacity: %d\n"
						"\tlast full capacity: %d\n"
						"\tdesign voltage: %d\n"
						"\tpresent rate: %d\n"
						"\tremaining capacity: %d\n"
						"\tpresent voltage: %d\n"
						"\tcharge state: %d\n"
						"\tbattery state: %d\n"
						"\tpercentage: %d%%\n"
						"\tremaining charge time: %02d:%02d h\n"
						"\tremaining time: %02d:%02d h\n",
						binfo->name, binfo->present, binfo->design_cap,
						binfo->last_full_cap, binfo->design_voltage,
						binfo->present_rate, binfo->remaining_cap,
						binfo->present_voltage, binfo->charge_state,
						binfo->batt_state, binfo->percentage,
						binfo->charge_time / 60, binfo->charge_time % 60,
						binfo->remaining_time / 60, binfo->remaining_time % 60);
*/
				setToolTip (tooltip);
		}
	}
	else
		message += "nicht bekannt; ";

	setText (message);
	//update ();
}
