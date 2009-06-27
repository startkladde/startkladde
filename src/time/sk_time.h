#ifndef _sk_time_h
#define _sk_time_h

#include <QDateTime>
#include <QString>

class sk_time:public QTime
{
	public:
		sk_time ();
		sk_time (int, int, int s=0, int ms=0);
		sk_time (const QTime &);

		QString table_string (bool gelandet);
		QString table_string (const char *);
};

#endif

