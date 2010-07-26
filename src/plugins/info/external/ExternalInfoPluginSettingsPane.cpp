#include "ExternalInfoPluginSettingsPane.h"

#include <QFileInfo>
#include <QFileDialog>

#include "src/util/qString.h"
#include "src/gui/dialogs.h"
#include "src/plugins/info/external/ExternalInfoPlugin.h"

// TODO: file resolving sunset plugin style

ExternalInfoPluginSettingsPane::ExternalInfoPluginSettingsPane (ExternalInfoPlugin *plugin, QWidget *parent):
	PluginSettingsPane (parent),
	plugin (plugin)
{
	ui.setupUi (this);
}

ExternalInfoPluginSettingsPane::~ExternalInfoPluginSettingsPane()
{

}

void ExternalInfoPluginSettingsPane::readSettings ()
{
	ui.commandInput    ->setText    (plugin->getCommand  ());
	ui.richTextCheckbox->setChecked (plugin->getRichText ());
}

bool ExternalInfoPluginSettingsPane::writeSettings ()
{
	plugin->setCommand  (ui.commandInput    ->text      ());
	plugin->setRichText (ui.richTextCheckbox->isChecked ());
	return true;
}

void ExternalInfoPluginSettingsPane::on_browseButton_clicked ()
{
	QString currentCommand=ui.commandInput->text ();
	QString resolved=plugin->resolveFilename (currentCommand, getEffectivePluginPaths ());

	QString dir;
	if (resolved.isEmpty ())
		dir=".";
	else
		dir=QFileInfo (resolved).dir ().path ();

	QString command=QFileDialog::getOpenFileName (
		this,
		utf8 ("Datei auswählen"),
		dir,
		"*",
		NULL,
		0
		);

	if (!command.isEmpty ())
	{
		ui.commandInput->setText (command);
//		on_filenameInput_editingFinished ();
	}
}
