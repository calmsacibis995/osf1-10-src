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
**      demangle_string.h
**
**  ABSTRACT:
**
**	Definitions necessary for demangling a string.
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


#ifndef _DEMANGLE_STRING_H_
#define _DEMANGLE_STRING_H_

/*
**
**  DEMANGLER FLAGS
**
**  The following flags can be passed to the "demangle_string" function as
**  the "flags" argument.  To combine several flags, logically OR them together.
**  NOTE:  combining MLD_SHOW_MANGLED_NAME and MLD_SHOW_DEMANGLED_NAME will put
**         the mangled name in parens.
** 
*/

#define MLD_SHOW_MANGLED_NAME	0x01	/* mangled name */
#define MLD_SHOW_INFO		0x02	/* e.g. type signatures */
#define MLD_NO_SPACES		0x04	/* no space in output */
#define MLD_SHOW_DEMANGLED_NAME	0x08	/* demangled name */


/*
**
**  DEMANGLE STRING
**
**  The "demangle_string" function, given a string "ds" of size "ds_max" will
**  populate the string with a demangled version of the string "s".  The 
**  demangling of the string is influenced by the value of "flags" (see above).
**  If there are several mangled names, all of the mangled names are replaced
**  with demangled names.  If there are no a mangled names or an error occurs,
**  "ds" will be equal to "s".  If ds is not large enough to hold the demangled
**  name, an error will result (and consequently, as just mentioned, "ds" will
**  equal "s").
**
**  NOTE:  This function will return either MLD_ERROR or MLD_OK, depending
**         on if an error occurred.
** 
*/

#define MLD_SOMETHING_RECOGNIZED 	 1
#define MLD_NOTHING_RECOGNIZED 		 0
#define MLD_ERROR			-1
#define MLD_INADEQUATE_ARRAY_SIZE 	-2

int /* status */ 
MLD_demangle_string(char* s,	/* IN:  string to be demangled */
                    char* ds,	/* OUT: demangled string */
                    int ds_max,	/* IN:	maximum size of ds */
                    int flags);	/* IN:	flags to control string formation */


#endif /* _DEMANGLE_STRING_H_ */
/* DON'T ADD ANYTHING AFTER THIS #endif */
