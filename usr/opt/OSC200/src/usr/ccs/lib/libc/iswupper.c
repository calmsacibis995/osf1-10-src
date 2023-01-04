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
static char *rcsid = "@(#)$RCSfile: iswupper.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 23:19:14 $";
#endif
/*/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: iswupper
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/lib/c/chr/iswupper.c, libcchr, bos320, 9132320 7/23/91 18:20:18
 */
/*
 *
 * FUNCTION: Determines if the process code, pc, is uppercase
 *	    
 *
 * PARAMETERS: pc  -- character to be classified
 *
 *
 * RETURN VALUES: 0 -- if pc is not uppercase
 *                >0 - If c is uppercase
 *
 *
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak iswupper = __iswupper
#endif
#endif
#include <sys/localedef.h>
#include <ctype.h>

#ifdef iswupper
#undef iswupper
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define iswupper __iswupper
#endif

int 
iswupper(wint_t pc)
{
	if (METHOD(__lc_ctype, iswctype) == NULL)
		return __iswctype_sb(pc, _ISUPPER, __lc_ctype);
	else
		return METHOD(__lc_ctype, iswctype)( 
			pc, _ISUPPER, __lc_ctype);
}
