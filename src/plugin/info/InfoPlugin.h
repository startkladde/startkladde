/*
 * InfoPlugin.h
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#ifndef INFOPLUGIN_H_
#define INFOPLUGIN_H_

#include "src/plugin/Plugin.h"
#include "src/accessor.h"

/**
 * A plugin which displays text in the info area of the main window
 *
 * Info plugins can be enabled and disabled (to avoid losing configuration by
 * deleting the plugin) and have a caption.
 */
class InfoPlugin: public Plugin
{
	Q_OBJECT

	public:
		// ***********
		// ** Types **
		// ***********

		class Descriptor
		{
			public:
				virtual InfoPlugin *create () const=0;
				virtual QString getId          () const=0;
				virtual QString getName        () const=0;
				virtual QString getDescription () const=0;
		};

		template<class T> class DefaultDescriptor: public Descriptor
		{
			public:
				virtual InfoPlugin *create () const { return new T (); }
				// FIXME read all at once, or only when required?
				virtual QString getId          () const { return T ().getId          (); }
				virtual QString getName        () const { return T ().getName        (); }
				virtual QString getDescription () const { return T ().getDescription (); }
		};


		// ******************
		// ** Construction **
		// ******************

		InfoPlugin ();
		virtual ~InfoPlugin ();


		// **********************
		// ** Property Acceess **
		// **********************

		value_accessor (QString, Caption, caption)
		bool_accessor (Enabled, enabled)


		// **************
		// ** Settings **
		// **************

		virtual PluginSettingsPane *createSettingsPane (QWidget *parent=NULL);

		/**
		 * Creates a PluginSettingsPane for editing the settings of this plugin
		 * instance
		 *
		 * The returned PluginSettingsPane must contain the fields for the
		 * actual settings of the info plugin, excluding settings common to all
		 * info plugins, like the caption.
		 *
		 * The caller takes ownership of the created PluginSettingsPane.
		 *
		 * @param parent the parent widget for the plugin pane
		 * @return a newly created PluginSettingsPane instance
		 */
		virtual PluginSettingsPane *infoPluginCreateSettingsPane (QWidget *parent=NULL)=0;

		virtual void readSettings (const QSettings &settings);
		virtual void writeSettings (QSettings &settings);

	signals:
		/**
		 * Output text to be displayed in the info area
		 */
		void textOutput (const QString &text, Qt::TextFormat format);

	protected:
		void outputText (const QString &text, Qt::TextFormat format=Qt::PlainText);

	private:
		/** The caption to be displayed for the info plugin */
		QString caption;

		/**
		 * Whether the plugin is enabled. If the plugin is disabled, it is not
		 * used, as if it were not configured, but its configuration is
		 * retained.
		 */
		bool enabled;
};

#endif
