#ifndef MATH_H_
#define MATH_H_

// FIXME qMax returns a reference, can we call that with a constant?
template <typename T> inline const T ifPositive (const T &value)
{
	if (value>0)
		return value;
	else
		return 0;
}

#endif
