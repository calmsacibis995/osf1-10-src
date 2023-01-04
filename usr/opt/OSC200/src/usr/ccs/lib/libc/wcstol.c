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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: wcstol.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:09:20 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: LIBCCNV wcstol
 *
 * FUNCTIONS: wcstol
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/lib/c/cnv/wcstol.c, libccnv, bos320, bosarea.9125 6/14/91 13:06:20
 */

/*
 * NAME: wcstol
 *                                                                    
 * FUNCTION: Converts a wide character string (in process code
 *           format) to an integer
 *                                                                    
 * NOTES:
 *
 * The function wcstol() converts the initial portion of the wide
 * character string pointed to by pwcs to a signed long integer
 * representation.  To do this, it parses the wide character string
 * pointed to by pwcs to obtain a valid string (i.e., subject string)
 * for the purpose of conversion to an unsigned long integer.  It 
 * points the endptr to the position where an unrecognized character
 * including the terminating null is found.
 *
 * The base can take the following values: 0 thru 9, a (or A) through
 * z (or Z).  This means that there can be potentially 36 values for
 * the base.  If the base is zero, the expected form of the subject
 * string is that of an unsigned integer constant as per the ANSI C
 * specifications with the option of a plus or minus sign, but not 
 * including the integer suffix.  If the base value is between 2 and
 * 38, the expected for of the subject sequence is a sequence of letters
 * and digits representing an integer with the radix specified by the base,
 * optionally preceeded by a plus or minus sign, but not including an
 * integer suffix.  The letters a (or A) throught z (or Z) are ascribed
 * the values 10 to 35; only letters whose values are less than that
 * of the base are permitted.  If the value of base is 16, the characters
 * 0x or 0X may optionally precede the sequence of letters or digits,
 * following the sign, if present.
 *
 * If the subject sequence has the expected form, it is interpreted as
 * an integer constant in the appropriate base.  A pointer to the final
 * string is stored in endptr, if the endptr is not a null character.
 * If the subject sequence is empty or does not have a valid form, no
 * conversion is done, the value of pwcs is stored in endptr, if endptr 
 * is not a null pointer.
 *
 * RETURN VALUES:
 *
 * Returns the converted value of long integer if the expected form is
 * found.
 *
 * If no conversion could be performed, zero is returned.
 *
 * If the converted value is outside the range of representable values,
 * LONG_MAX or LONG_MIN is returned (according to the sign of the value),
 * and the value of the variable errno is set to ERANGE.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak wcstol = __wcstol
#endif
#endif
#if !defined(_NOIDENT)
#define _NOIDENT
#endif
#define WCHAR
#include "strtol.c"
