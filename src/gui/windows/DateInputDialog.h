#ifndef DATEINPUTDIALOG_H
#define DATEINPUTDIALOG_H

#include <QDialog>

// TODO reduce dependencies
#include "ui_DateInputDialog.h"

class DateInputDialog : public QDialog
{
    Q_OBJECT

public:
    DateInputDialog (QWidget *parent = 0, Qt::WindowFlags f=0);
    ~DateInputDialog();

    static bool editDate (QWidget *parent, QDate *date, QTime *time, QString title, QString text, bool todayEnabled, bool yesterdayEnabled, bool specificSelected);

private:
    Ui::DateInputDialogClass ui;
};

#endif // DATEINPUTDIALOG_H
