#ifndef GPRMCSENTENCE_H_
#define GPRMCSENTENCE_H_

#include <QString>
#include <QDateTime>

#include "src/numeric/GeoPosition.h"

class GprmcSentence
{
	public:
		GprmcSentence (const QString &sentence);
		virtual ~GprmcSentence ();

		QString sentence;
		bool isValid;

		QDateTime timestamp;
		bool status;
		GeoPosition position;
};

#endif
