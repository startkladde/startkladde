#ifndef ACCESSOR_H_
#define ACCESSOR_H_

// FIXME clean up

// Really old accessors

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


// Old accessors

#define attr_accessor(type, capitalName, name) \
        attr_reader (type, capitalName, name) \
        attr_writer (type, capitalName, name)

#define const_attr_accessor(type, capitalName, name) \
        const_attr_reader (type, capitalName, name) \
        attr_writer (type, capitalName, name)

#define virtual_attr_accessor(type, capitalName, name) \
        virtual_attr_reader (type, capitalName, name) \
        virtual attr_writer (type, capitalName, name)

#define attr_reader(      type, capitalName, name) type       get ## capitalName ()                { return name; }
#define const_attr_reader(type, capitalName, name) const type get ## capitalName ()          const { return name; }
#define attr_writer(      type, capitalName, name) void       set ## capitalName (type name)       { this->name=name; }

#define virtual_attr_reader(      type, capitalName, name) virtual attr_reader       (type, capitalName, name)
#define virtual_const_attr_reader(type, capitalName, name) virtual const_attr_reader (type, capitalName, name)
#define virtual_attr_writer(      type, capitalName, name) virtual attr_writer       (type, capitalName, name)


// New accessors
#define value_reader(type, capitalName, name) type get ## capitalName () const           { return name;     }
#define value_writer(type, capitalName, name) void set ## capitalName (const type &name) { this->name=name; }
#define value_accessor(type, capitalName, name) \
        value_reader (type, capitalName, name) \
        value_writer (type, capitalName, name)

#define bool_reader(capitalName, name) bool is  ## capitalName () const    { return name;     }
#define bool_writer(capitalName, name) void set ## capitalName (bool name) { this->name=name; }
#define bool_accessor(capitalName, name) \
        bool_reader (capitalName, name) \
        bool_writer (capitalName, name)

// TODO: read value as const &




#endif

