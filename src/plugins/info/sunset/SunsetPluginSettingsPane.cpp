#include "SunsetPluginSettingsPane.h"

#include <QFile>
#include <QFileDialog>

#include "src/util/qString.h"
#include "src/util/file.h"
#include "src/text.h"
#include "src/gui/dialogs.h"

SunsetPluginSettingsPane::SunsetPluginSettingsPane (SunsetPluginBase *plugin, QWidget *parent):
	PluginSettingsPane (parent),
	plugin (plugin)
{
	ui.setupUi (this);
}

SunsetPluginSettingsPane::~SunsetPluginSettingsPane()
{

}

void SunsetPluginSettingsPane::readSettings ()
{
	ui.filenameInput->setText (plugin->getFilename ());
	on_filenameInput_editingFinished ();
}

bool SunsetPluginSettingsPane::writeSettings ()
{
	plugin->setFilename (ui.filenameInput->text ());
	return true;
}

void SunsetPluginSettingsPane::on_filenameInput_editingFinished ()
{
	QString filename=ui.filenameInput->text ().trimmed ();

	if (blank (filename))
		ui.filenameLabel->setText ("-");
	else
	{
		QString resolved=plugin->resolveFilename (filename);

		if (resolved.isEmpty ())
		{
			ui.filenameLabel->setText ("nicht gefunden");
			ui.  sourceLabel->setText ("-");
		}
		else if (!QFile::exists (resolved))
		{
			ui.filenameLabel->setText ("existiert nicht");
			ui.  sourceLabel->setText ("-");
		}
		else
		{
			ui.filenameLabel->setText (QFileInfo (resolved).absoluteFilePath ());

			QRegExp re ("^Source: (.*)");
			try
			{
				if (findInFile (resolved, re))
					ui.sourceLabel->setText (re.cap (1));
				else
					ui.sourceLabel->setText ("unbekannt");
			}
			catch (FileOpenError &ex)
			{
				ui.sourceLabel->setText (QString ("Fehler: %1").arg (ex.errorString));
			}
		}
	}
}

void SunsetPluginSettingsPane::on_findFileButton_clicked ()
{
	QString currentFile=ui.filenameInput->text ();
	QString resolved=plugin->resolveFilename (currentFile);

	QString dir;
	if (resolved.isEmpty ())
		dir=".";
	else
		dir=QFileInfo (resolved).dir ().path ();

	QString filename=QFileDialog::getOpenFileName (
		this,
		utf8 ("Datei auswählen"),
		dir,
		"*.txt",
		NULL,
		0
		);

	if (!filename.isEmpty ())
	{
		ui.filenameInput->setText (filename);
		on_filenameInput_editingFinished ();
	}
}
