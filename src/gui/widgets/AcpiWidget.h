#ifndef _AcpiWidget_h
#define _AcpiWidget_h

/**
  * wrapper class for libacpi
  * eggert.ehmke@berlin.de
  * 10.09.2008
  */

#include <QLabel>
#include <QTimer>

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
