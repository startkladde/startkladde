#ifndef ACPIWIDGET_H
#define ACPIWIDGET_H

/**
  * wrapper class for libacpi
  * eggert.ehmke@berlin.de
  * 10.09.2008
  */

#include <qlabel.h>
#include <qtimer.h>

class acpiwidget: public QLabel
{
  Q_OBJECT

  public:
  acpiwidget (QWidget* parent);

  static bool valid ();

  private:
  QTimer* timer;
        
  private slots:
  void slotTimer();
};

#endif
