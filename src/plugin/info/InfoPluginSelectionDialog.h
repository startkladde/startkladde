#ifndef INFOPLUGINSELECTIONDIALOG_H
#define INFOPLUGINSELECTIONDIALOG_H

#include <QtGui/QDialog>
#include "ui_InfoPluginSelectionDialog.h"

#include "src/plugin/info/InfoPlugin.h"

template<class T> class QList;

class InfoPluginSelectionDialog: public QDialog
{
		Q_OBJECT

	public:
		InfoPluginSelectionDialog (const QList<InfoPlugin::Descriptor *> &plugins, QWidget *parent=NULL);
		~InfoPluginSelectionDialog ();

		void setup ();
		const InfoPlugin::Descriptor *getCurrentPluginDescriptor ();

		static const InfoPlugin::Descriptor *select (const QList<InfoPlugin::Descriptor *> &plugins, QWidget *parent=NULL);

	private:
		Ui::InfoPluginSelectionDialogClass ui;

		QList<InfoPlugin::Descriptor *> plugins;
};

#endif
