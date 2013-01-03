#ifndef UTIL_DOUBLE_H_
#define UTIL_DOUBLE_H_

double floor (double value, double unit);

void toScientific (double value, double *mantissa, double *exponent, bool *sign);
double fromScientific (double mantissa, double exponent, bool sign);



//double logDiv (double y, double x);
//double logMod (double y, double x);


#endif
