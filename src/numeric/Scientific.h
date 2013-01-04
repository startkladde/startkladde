#ifndef SCIENTIFIC_H_
#define SCIENTIFIC_H_

/**
 * Scientific number representation
 */
class Scientific
{
	public:
		Scientific (double mantissa, int exponent, bool sign);
		Scientific (double value);
		virtual ~Scientific ();

		double mantissa () { return _mantissa; }
		int    exponent () { return _exponent; }
		bool   sign     () { return _sign;     }

		void setMantissa (double mantissa) { _mantissa=mantissa; }
		void setExponent (int    exponent) { _exponent=exponent; }
		void setSign     (bool   sign    ) { _sign    =sign;     }

		void setValue (double value);
		double toValue ();

	private:
		double _mantissa;
		int _exponent;
		bool _sign;
};

#endif
