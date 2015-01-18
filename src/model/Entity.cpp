#include "Entity.h"

#include <QStringBuilder>
#include <QRegExp>

// ******************
// ** Construction **
// ******************

Entity::Entity ()
{
	id=0;
}

Entity::Entity (dbId id)
{
	this->id=id;
}

Entity::~Entity ()
{
}


void Entity::setField(const char* name, const QString& value)
{
    QString comment= value.isEmpty() ? QString() : QString("%1='%2'").arg(name, value);

    if ( comments.isEmpty() ) {
        comments= comment;
        return;
    }

    QString pattern= QString("\\s*%1\\s*=\\s*'.+'\\s*").arg(name);
    QRegExp re(pattern);

    int pos( re.indexIn(comments) );

    if(pos == -1){
        comments+= QString("; %1").arg(comment);
    }
    else{
        QStringRef before(&comments, 0, pos);
        QStringRef after(&comments, pos + re.matchedLength(), comments.length() - pos - re.matchedLength() );

        comments= before % comment % after;
    }
}


QString Entity::getField(const char* name) const
{
    if ( comments.isEmpty() )
        return QString();

    QString pattern= QString("%1\\s*=\\s*'(.+)'").arg(name);
    QRegExp re(pattern);

    if( re.indexIn(comments) == -1 )
        return QString();
    else
        return re.cap(1);
}
