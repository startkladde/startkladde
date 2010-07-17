#include "SunsetPluginSettingsPane.h"

#include <QFile>
#include <QFileDialog>
#include <QDebug>

#include "src/plugins/info/sunset/SunsetPluginBase.h"
#include "src/util/qString.h"
#include "src/util/file.h"
#include "src/text.h"
#include "src/gui/dialogs.h"
#include "src/plugins/info/sunset/SunsetTimePlugin.h"

SunsetPluginSettingsPane::SunsetPluginSettingsPane (SunsetPluginBase *plugin, QWidget *parent):
	PluginSettingsPane (parent),
	plugin (plugin),
	fileSpecified (false), fileResolved (false), fileExists (false), fileOk (false),
	referenceLongitudeFound (false)
{
	ui.setupUi (this);

	SunsetTimePlugin *sunsetTimePlugin=dynamic_cast<SunsetTimePlugin *> (plugin);
	ui.timeZoneLabel->setVisible (sunsetTimePlugin!=NULL);
	ui.timeZoneInput->setVisible (sunsetTimePlugin!=NULL);
}

SunsetPluginSettingsPane::~SunsetPluginSettingsPane()
{

}

void SunsetPluginSettingsPane::readSettings ()
{
	ui.filenameInput->setText (plugin->getFilename ());
	ui.longitudeInput->setLongitude (plugin->getLongitude ());
	ui.longitudeCorrectionCheckbox->setChecked (plugin->getLongitudeCorrection ());

	SunsetTimePlugin *sunsetTimePlugin=dynamic_cast<SunsetTimePlugin *> (plugin);
	if (sunsetTimePlugin)
		ui.timeZoneInput->setCurrentIndex (sunsetTimePlugin->getDisplayUtc ()?0:1);

	on_filenameInput_editingFinished ();
}

bool SunsetPluginSettingsPane::writeSettings ()
{
	plugin->setFilename (ui.filenameInput->text ());
	plugin->setLongitude (ui.longitudeInput->getLongitude ());
	plugin->setLongitudeCorrection (ui.longitudeCorrectionCheckbox->isChecked ());

	SunsetTimePlugin *sunsetTimePlugin=dynamic_cast<SunsetTimePlugin *> (plugin);
	if (sunsetTimePlugin)
		sunsetTimePlugin->setDisplayUtc (ui.timeZoneInput->currentIndex ()==0);

	return true;
}

void SunsetPluginSettingsPane::on_longitudeCorrectionCheckbox_toggled ()
{
	updateReferenceLongitudeNoteLabel ();
}

void SunsetPluginSettingsPane::on_filenameInput_editingFinished ()
{
	QString filename=ui.filenameInput->text ().trimmed ();

	fileSpecified=false;
	fileResolved=false;
	fileExists=false;
	fileOk=false;

	referenceLongitude=Longitude ();

	if (!isBlank (filename))
	{
		fileSpecified=true;

		QString resolved=plugin->resolveFilename (filename, getEffectivePluginPaths ());

		if (!resolved.isEmpty ())
		{
			fileResolved=true;
			resolvedFilename=QFileInfo (resolved).absoluteFilePath ();

			if (QFile::exists (resolved))
			{
				fileExists=true;

				try
				{
					source=SunsetPluginBase::readSource (resolved);

					QString referenceLongitudeString=SunsetPluginBase::readReferenceLongitudeString (resolved);
					referenceLongitudeFound=!referenceLongitudeString.isEmpty ();
					referenceLongitude=Longitude::parse (referenceLongitudeString);

					fileOk=true;
				}
				catch (FileOpenError &ex)
				{
					fileError=ex.errorString;
				}
			}
		}
	}

	updateFilenameLabel ();
	updateSourceLabel ();
	updateReferenceLongitudeLabel ();
	updateReferenceLongitudeNoteLabel ();
}


void SunsetPluginSettingsPane::on_findFileButton_clicked ()
{
	QString currentFile=ui.filenameInput->text ();
	QString resolved=plugin->resolveFilename (currentFile, getEffectivePluginPaths ());

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


// ************
// ** Labels **
// ************

void SunsetPluginSettingsPane::updateFilenameLabel ()
{
	if (!fileSpecified)
		ui.filenameLabel->setTextAndDefaultColor ("-");
	else if (!fileResolved)
		ui.filenameLabel->setTextAndColor ("nicht gefunden", Qt::red);
	else if (!fileExists)
		ui.filenameLabel->setTextAndColor ("existiert nicht", Qt::red);
	else if (!fileOk)
		ui.filenameLabel->setTextAndColor (QString ("%1\nFehler: %2").arg (resolvedFilename, fileError), Qt::red);
	else
		ui.filenameLabel->setTextAndDefaultColor (resolvedFilename);
}

void SunsetPluginSettingsPane::updateSourceLabel ()
{
	if (!fileOk)
		ui.sourceLabel->setText ("-");
	else if (source.isEmpty ())
		ui.sourceLabel->setText ("unbekannt");
	else
		ui.sourceLabel->setText (source);
}

void SunsetPluginSettingsPane::updateReferenceLongitudeLabel ()
{
	if (!fileOk)
		ui.referenceLongitudeLabel->setText ("-");
	else if (!referenceLongitudeFound)
		ui.referenceLongitudeLabel->setText (utf8 ("unbekannt"));
	else if (!referenceLongitude.isValid ())
		ui.referenceLongitudeLabel->setText (utf8 ("ungültig"));
	else
		ui.referenceLongitudeLabel->setText (referenceLongitude.format ());
}

const QString referenceLongitudeNoteRegular=
	utf8 ("Längengradkorrektur ist nur dann möglich, wenn in\nder Datendatei ein Bezugslängengrad angegeben ist.");
const QString referenceLongitudeNoteError=
	utf8 ("Längengradkorrektur ist nicht möglich, weil in\nder Datendatei kein Bezugslängengrad angegeben ist.");

void SunsetPluginSettingsPane::updateReferenceLongitudeNoteLabel ()
{
	if (fileOk && !referenceLongitude.isValid ())
	{
		if (ui.longitudeCorrectionCheckbox->isChecked ())
			// Longitude correction activated - this is an error
			ui.referenceLongitudeNoteLabel->setTextAndColor (referenceLongitudeNoteError, Qt::red);
		else
			// Longitude correction not activated - this is ok
			ui.referenceLongitudeNoteLabel->setTextAndDefaultColor (referenceLongitudeNoteError);
	}
	else
	{
		ui.referenceLongitudeNoteLabel->setTextAndDefaultColor (referenceLongitudeNoteRegular);
	}
}

