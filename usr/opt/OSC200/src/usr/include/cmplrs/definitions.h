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
**      definitions.c
**
**  ABSTRACT:
**
**	Header file containing definitions for tokens used in mangling.
**
**  ENVIRONMENT:
**
**	User Mode
**
**  AUTHORS:
**
**  MODIFICATION HISTORY:
**
**
**--
**/


#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

/* 
***
*** tokens for basic types 
***
*/

#define	NUMBER_OF_BASIC_TYPES 9

#define	VOID_TOKEN			'v'
#define	CHAR_TOKEN			'c'
#define	SHORT_TOKEN			's'
#define	INT_TOKEN			'i'
#define	LONG_TOKEN			'l'
#define	FLOAT_TOKEN			'f'
#define	DOUBLE_TOKEN		'd'
#define	LONG_DOUBLE_TOKEN	'r'
#define	ELLIPSES_TOKEN		'e'

#define BASIC_TYPE_TOKENS \
		{	VOID_TOKEN, CHAR_TOKEN, SHORT_TOKEN, INT_TOKEN, LONG_TOKEN,  \
			FLOAT_TOKEN, DOUBLE_TOKEN, LONG_DOUBLE_TOKEN, ELLIPSES_TOKEN }


/* 
***
*** strings for basic types 
***
*/

#define	VOID_STRING			"void"
#define	CHAR_STRING			"char"
#define	SHORT_STRING		"short"
#define	INT_STRING			"int"
#define	LONG_STRING			"long"
#define	FLOAT_STRING		"float"
#define	DOUBLE_STRING		"double"
#define	LONG_DOUBLE_STRING	"long double"
#define	ELLIPSES_STRING		"..."

#define BASIC_TYPE_STRINGS \
		{	VOID_STRING, CHAR_STRING, SHORT_STRING, INT_STRING, LONG_STRING, \
			FLOAT_STRING, DOUBLE_STRING, LONG_DOUBLE_STRING, ELLIPSES_STRING }


/* 
***
*** Token indicating that something is qualified 
***
*/

#define	QUALIFIED_TOKEN	'Q'


/* 
***
*** Token used to represent template
***
*/

#define	TEMPLATE_TOKEN	'T'


/* 
***
*** Token for type modifiers 
***
*/

#define	NUMBER_OF_TYPE_MODIFIERS 4

#define	CONST_TOKEN		'C'
#define	SIGNED_TOKEN	'S'
#define	UNSIGNED_TOKEN	'U'
#define	VOLATILE_TOKEN	'V'

#define TYPE_MODIFIER_TOKENS \
		{	UNSIGNED_TOKEN, CONST_TOKEN, VOLATILE_TOKEN, SIGNED_TOKEN }


/* 
***
*** Strings for type modifiers 
***
*/

#define	UNSIGNED_STRING	"unsigned"
#define	CONST_STRING	"const"
#define	VOLATILE_STRING	"volatile"
#define	SIGNED_STRING	"signed"

#define TYPE_MODIFIER_STRINGS \
		{	UNSIGNED_STRING, CONST_STRING, VOLATILE_STRING, SIGNED_STRING }


/* 
***
*** Tokens for type declarators 
***
*/

#define	POINTER_TOKEN			'P'
#define	REFERENCE_TOKEN			'R'
#define	ARRAY_TOKEN				'A'
#define	FUNCTION_TOKEN			'X'
#define	POINTER_TO_MEMBER_TOKEN	'M'


/* 
***
*** Strings for type declarators 
***
*/

#define	POINTER_STRING			"*"
#define	REFERENCE_STRING		"&"
#define	ARRAY_STRING			"[]"
#define	FUNTION_STRING			"()"
#define	POINTER_TO_STRING		"::*"


/* 
***
*** Tokens (in string form as opposed to character form) and strings 
*** for basic operator functions
***
*/
	 
#define NUMBER_OF_BASIC_OPERATORS 40
#define BASIC_OPERATOR_TOKEN 0
#define BASIC_OPERATOR_STRING 1

#define BASIC_OPERATORS {									\
	{	"__ml"		,		"*"			},					\
	{	"__dv"		,		"/"			},					\
	{	"__md"		,		"%"			},					\
	{	"__pl"		,		"+"			},					\
	{	"__mi"		,		"-"			},					\
	{	"__ls"		,		"<<"		},					\
	{	"__rs"		,		">>"		},					\
	{	"__eq"		,		"=="		},					\
	{	"__ne"		,		"!="		},					\
	{	"__lt"		,		"<"			},					\
	{	"__gt"		,		">"			},					\
	{	"__le"		,		"<="		},					\
	{	"__ge"		,		">="		},					\
	{	"__ad"		,		"&"			},					\
	{	"__or"		,		"|"			},					\
	{	"__er"		,		"^"			},					\
	{	"__aa"		,		"&&"		},					\
	{	"__oo"		,		"||"		},					\
	{	"__nt"		,		"!"			},					\
	{	"__co"		,		"~"			},					\
	{	"__pp"		,		"++"		},					\
	{	"__mm"		,		"--"		},					\
	{	"__as"		,		"="			},					\
	{	"__rf"		,		"->"		},					\
	{	"__apl"		,		"+="		},					\
	{	"__ami"		,		"-="		},					\
	{	"__amu"		,		"*="		},					\
	{	"__adv"		,		"/="		},					\
	{	"__amd"		,		"%="		},					\
	{	"__als"		,		"<<="		},					\
	{	"__ars"		,		">>="		},					\
	{	"__aad"		,		"&="		},					\
	{	"__aor"		,		"|="		},					\
	{	"__aer"		,		"^="		},					\
	{	"__cm"		,		","			},					\
	{	"__rm"		,		"->*"		},					\
	{	"__cl"		,		"()"		},					\
	{	"__vc"		,		"[]"		},					\
	{	"__nw"		,		"new"		},					\
	{	"__dl"		,		"delete"	}}


/* 
***
*** Tokens for operator functions (in string form)
***
*/

#define	FUNCTION_CALL_TOKEN			"__cl"
#define	SUBSCRIPTING_TOKEN			"__vc"

#define	CONSTRUCTOR_TOKEN			"__ct"
#define	DESTRUCTOR_TOKEN			"__dt"

#define	OPERATOR_NEW_TOKEN			"__nw"
#define	OPERATOR_DELETE_TOKEN		"__dl"

#define	OPERATOR_CONVERSION_TOKEN	"__op"


/* 
***
*** Strings for operator functions
***
*/

#define	FUNCTION_CALL_STRING		"()"
#define	SUBSCRIPTING_STRING			"[]"
#define	CONSTRUCTOR_STRING			""
#define	DESTRUCTOR_STRING			"~"
#define	OPERATOR_NEW_STRING			""
#define	OPERATOR_DELETE_STRING		""
#define	OPERATOR_CONVERSION_STRING	""


/* 
***
*** Tokens for shortening the encoding of types
***
*/

#define	NTH_TYPE_TOKEN									'T'
#define	NEXT_N_TYPES_ARE_SAME_AS_NTH_TYPE_TOKEN			'N'

#endif /* _DEFINITIONS_H_ */
/* DON'T ADD ANYTHING AFTER THIS #endif */

