#ifndef _AcpiWidget_h
#define _AcpiWidget_h

/**
  * wrapper class for libacpi
  * eggert.ehmke@berlin.de
  * 10.09.2008
  */

#include <qlabel.h>
#include <qtimer.h>

class AcpiWidget: public QLabel
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
