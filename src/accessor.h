#ifndef accessor_h
#define accessor_h


#define RO_ACCESSOR(type, var)	\
		inline type get_ ## var () const { return var; }

#define RW_ACCESSOR(type, var)	\
		inline type get_ ## var () const { return var; }	\
		inline void set_ ## var (const type &_ ## var) { var=_ ## var; }

#define RW_ACCESSOR_REF_DEF(thistype, type, var, def)	\
		inline type get_ ## var () const { return var; }	\
		inline thistype &set_ ## var (const type &_ ## var=def) { var=_ ## var; return *this; }

#define RO_P_ACCESSOR(type, var)	\
		inline type *get_ ## var () const { return var; }

#define RW_P_ACCESSOR(type, var)	\
		inline type *get_ ## var () const { return var; }	\
		inline void set_ ## var (type * const _ ## var) { var=_ ## var; }

#endif

