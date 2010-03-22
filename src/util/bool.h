/*
 * bool.h
 *
 *  Created on: 21.02.2010
 *      Author: Martin Herrmann
 */

#ifndef BOOL_H_
#define BOOL_H_

#include <QString>

QString boolToString (bool val, const QString &true_value="Ja", const QString &false_value="Nein");
bool stringToBool (const QString &text);

#endif
