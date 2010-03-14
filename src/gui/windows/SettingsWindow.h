#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QtGui/QDialog>
#include "ui_SettingsWindow.h"

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent = 0);
    ~SettingsWindow();

private:
    Ui::SettingsWindowClass ui;
};

#endif // SETTINGSWINDOW_H
