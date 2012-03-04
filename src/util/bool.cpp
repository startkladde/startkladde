#include "bool.h"

//QString boolToString (bool val, const QString &true_value, const QString &false_value)
//{
//	if (val)
//		return true_value;
//	else
//		return false_value;
//}

bool stringToBool (const QString &text)
{
	if (text.toInt ()==0)
		return false;
	else
		return true;
}
