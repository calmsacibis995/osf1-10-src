/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */

/*
 * HISTORY
 */

/*
**++
**  PRODUCT:
***
**	DEC C++ Demangler
**
**  FILE:  
**
**      mangled_name.h
**
**  ABSTRACT:
**
**	This file contains info necessary for demangling a single name.
**
**  ENVIRONMENT:
**
**	User Mode
**
**  AUTHORS:
**
**
**  MODIFICATION HISTORY:
**
**
**--
**/


#ifndef _MANGLED_NAME_H_
#define _MANGLED_NAME_H_

/*
*** 
*** Misc.
***
*/

#define	MLD_ERROR	-1
#define MLD_SOMETHING_RECOGNIZED 1
#define MLD_NOTHING_RECOGNIZED 0


/*
*** 
*** Predeclarations
***
*/

struct MLD_complex;
struct MLD_pointer;
struct MLD_reference;
struct MLD_array;
struct MLD_pointer_to_member;
struct MLD_function;
struct MLD_qualification;
struct MLD_type_modifiers;
struct MLD_unmodified_type;
struct MLD_type;
struct MLD_mangled_function_name;
struct MLD_mangled_template_name;
struct MLD_mangled_variable_name;
struct MLD_mangled_name;


/*
*** 
*** Definition of the type structure
***
*/

struct MLD_type {

    /* type modifier list:
       This actually isn't a list anymore...it's really a structure
       with a boolean for each of the type modifiers (const, volatile,
       signed, unsigned). */
    struct MLD_type_modifiers* tm_list;
    
    /* unmodified type:
       This is a pointer to an unmodified type.  An unmodified
       type would be a type (pointer, int, char, pointer to member, etc.)
       that doesn't have type modifiers associated with it. */
    struct MLD_unmodified_type* ut;
    
    /* next:
       This is a pointer to the next type.  For instance, let's say that
       you had the mangled name, "foo__Xiii", then there would be 
       three type structures with the first's next field pointing to the
       second and so on. */
	struct MLD_type* next;
};


/*
*** 
*** Definition of the unmodified type structure
***
*/

/* Type of type:
   This is the kind of type that is stored in the unmodified type structure. */
enum MLD_type_type {
        MLD_NO_TYPE,				/* no type defined yet */
        MLD_BASIC_TYPE,				/* int, float, char, double, void, ... */
        MLD_COMPLEX_TYPE,			/* a named type, like "pointer" below */
        MLD_POINTER_TYPE,			/* a pointer */
        MLD_REFERENCE_TYPE,			/* a reference */
        MLD_ARRAY_TYPE,				/* an array */
        MLD_FUNCTION_TYPE,			/* a function */
        MLD_POINTER_TO_MEMBER_TYPE,	/* a pointer to a member */
        MLD_CONSTANT_TYPE,			/* a constant (used only for templates) */
        MLD_ADDRESS_TYPE			/* an address (used only for templates) */
};

struct MLD_unmodified_type {
	int type_of_type; /* see the enumeration above for more info */
	union {
			char* type_name; /* used for CONSTANT_TYPE and ADDRESS_TYPE */
			int basic;       /* integer representing a basic type:  see below */
			struct MLD_qualification* complex; /* used for complex */
			struct MLD_pointer* pointer;       /* used for pointer */
			struct MLD_reference* reference;   /* used for reference */
			struct MLD_array* array;           /* used for array */
			struct MLD_function* function;     /* used for function */
			struct MLD_pointer_to_member* ptr_to_member; /* used for pointer to member */
	} the_type; /* storage for the type in question (determined by "type_of_type" */
};

/*
*** 
*** Definitions for the various "types of types"
***
*/

/* Basic types:
   A basic type is represented as an integer.  This enumeration allows you to
   interpret the basic type integer stored in the unmodified type structure.
   Note that ellipses is one of the basic types. */
enum basic_types {
        MLD_VOID_BASIC_TYPE,		/* void */
        MLD_CHAR_BASIC_TYPE,		/* char */
        MLD_SHORT_BASIC_TYPE,		/* short */
        MLD_INT_BASIC_TYPE,			/* int */
        MLD_LONG_BASIC_TYPE,		/* long */
        MLD_FLOAT_BASIC_TYPE,		/* float */
        MLD_DOUBLE_BASIC_TYPE,		/* double */
        MLD_LONG_DOUBLE_BASIC_TYPE,	/* long double */
        MLD_ELLIPSES_BASIC_TYPE		/* ellipses "..." */
};

/* Pointer type:
   A pointer type has one argument--the type modified.  The type
   modified can be any of the "types of types" including pointer. */
struct MLD_pointer {
	struct MLD_type*	type_modified;
};

/* Reference type:
   A reference type has one argument--the type modified.  The type
   modified can be any of the "types of types" including reference. */
struct MLD_reference {
	struct MLD_type*	type_modified;
};

/* Array type:
   An array type has two arguments--a dimension represented as an integer and
   a type modified which is similar to the type modified part of pointer */
struct MLD_array {
	int 			dimension;
	struct MLD_type*	type_modified;
};


/* Pointer to member type:
   A pointer to member type has two arguments--a list of qualifications
   (see the explanation of the qualifications data structure for more info)
   and a type modified portion which is similar to what's been described above. */
struct MLD_pointer_to_member {
	struct MLD_qualification*	qualifications;
	struct MLD_type*			type_modified;
};

/* Qualification type:
   The term qualification is not a good term for this structure.  This
   structure should actually be called something like, "fully qualified
   class or struct name."  Each of the classes or structures that are
   involved with qualifying the final class or struct name is represented
   as a "complex" (see below).  The next field is a pointer to the next
   qualification.  A full qualification consists of a linked list of
   qualification nodes--the last of which is the actual class or struct
   name. */
struct MLD_qualification {
	struct MLD_complex*			q; /* a qualification is a complex
								  type */
	struct MLD_qualification*	next;
};

/* complex:
   a complex is a single class or structure name.  Since a 
   class or structure can be a template, a complex can also be a template */
struct MLD_complex {
	int mangled_template_name; /* boolean:  1 means it's a mangled template name
								   			0 means that it's just a structure name. */
	union {
    	char*							structure_name; /* actual text 
                                                           representing structure
                                                           name */
		struct MLD_mangled_template_name*	template_name;  /* a template structure */
	} name; /* either a structure name or a template name */
};

/* function type:
   A function type has two arguments--a series of arguments and a return type. */
struct MLD_function {
	struct MLD_type*	args;
	struct MLD_type*	return_type;
};


/*
*** 
*** Definition of type modifier structure
***
*/

/* The type modifiers structure consists of a set of integers 
   each of which is a boolean for one of the types of type modifiers */
struct MLD_type_modifiers {
	int const_tm;		/* boolean:  1 if const, 0 if not */
	int signed_tm;		/* boolean:  1 if signed, 0 if not */
	int unsigned_tm;	/* boolean:  1 if unsigned, 0 if not */
	int volatile_tm;	/* boolean:  1 if volatile, 0 if not */
};


/*
*** 
*** Definition of the mangled function name structure (and other associated structures)
***
*/

enum MLD_function_name_types {
	MLD_NO_FUNCTION_NAME,
	MLD_NORMAL_FUNCTION_NAME,
	MLD_OPERATOR_FUNCTION_NAME,
	MLD_CONSTRUCTOR_FUNCTION_NAME,
	MLD_DESTRUCTOR_FUNCTION_NAME,
	MLD_CONVERSION_OPERATOR_FUNCTION_NAME
};

struct MLD_mangled_function_name {
	enum MLD_function_name_types function_name_type; /* the function name type (see above) */
	union {
		char* identifier; /* the identifier text */
		struct MLD_type* conversion_operator_type; /* a type structure--if a conversion op */
	} id;
	struct MLD_type_modifiers* tm_list; /* type modifiers */
	struct MLD_qualification*	qualifications; /* qualifications, if define within a class */
	struct MLD_type* args; /* arguments of the function */
};


/*
*** 
*** Definition of the mangled template name structure (and other associated structures)
***
*/

struct MLD_mangled_template_name {
	char* identifier; /* the identifier for the template */
	struct MLD_type* args; /* arguments for the template */
};


/*
*** 
*** Definition of the mangled variable name structure (and other associated structures)
***
*/

struct MLD_mangled_variable_name {
	char* identifier; /* identifier for the variable */
	struct MLD_qualification*	qualifications; /* qualifications, if defined within a class */
};


/*
*** 
*** Definition of the mangled name structure (and other associated structures)
***
*/


enum MLD_mangled_name_types {
	MLD_NO_MANGLED_NAME,
	MLD_FUNCTION_MANGLED_NAME,
	MLD_TEMPLATE_MANGLED_NAME,
	MLD_VARIABLE_MANGLED_NAME,
	MLD_VIRTUAL_TABLE_NAME /* uses "info" item in name union, below */
};

struct MLD_mangled_name {
	enum MLD_mangled_name_types mangled_name_type; /* type from above enum */
	union {
		struct MLD_mangled_function_name* funct_name;
		struct MLD_mangled_template_name* templ_name;
		struct MLD_mangled_variable_name* var_name;
		char* info; /* used for VIRTUAL_TABLE_NAME */
	} name;
};


/*
*** 
*** Definition of the mangled name function
***
*/

int /* recognition status */
MLD_parse_mangled_name
			(char* s,					/* IN:  string to be parsed */
             int start,					/* IN:  index of first unparsed character */
             int* end,					/* OUT: index of last parsed character */
             struct MLD_mangled_name** mn,	/* OUT: structure containing mangled name */
             char* str_before_name,		/* OUT: string before mangled name */
             char* str_replaced_by_name,/* OUT: string replaced by mangled name */
             char* str_after_name,		/* OUT: string after mangled name */
             int size_of_string);		/* IN:	max size of the strings listed above */

void
MLD_delete_mangled_name(struct MLD_mangled_name* mn);

void
MLD_concat_mangled_name(struct MLD_mangled_name* mn,
                     char* s,
                     int max_s,
                     int flags);

void
MLD_concat_mangled_function_name_identifier
                   (struct MLD_mangled_function_name* mfn,
                    char* s,
                    int max_s,
                    int flags);

void
MLD_concat_mangled_template_name
                   (struct MLD_mangled_template_name* mtn,
                    char* s,
                    int max_s,
                    int flags);


#endif /* _MANGLED_NAME_H_ */
/* DON'T ADD ANYTHING AFTER THIS #endif */
