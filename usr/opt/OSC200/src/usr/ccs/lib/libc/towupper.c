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
static char *rcsid = "@(#)$RCSfile: towupper.c,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/09/30 20:21:32 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: towupper
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/lib/c/chr/towupper.c, libcchr, bos320, 9130320 7/17/91 15:17:24
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak towupper = __towupper
#endif
#endif
#include <sys/localedef.h>
#include <ctype.h>
    
#undef towupper
#ifdef _NAME_SPACE_WEAK_STRONG
#define towupper __towupper
#endif

wint_t 
__towupper_std( wint_t wc, _LC_ctype_t *hdl)
{
    if(wc > hdl->max_upper)
	return(wc);
    return(hdl->_upper[wc]);
}

wint_t 
towupper(wint_t wc)
{
	if (METHOD(__lc_ctype,towupper) == NULL)
		return __towupper_std(wc, __lc_ctype);
	else
		return METHOD(__lc_ctype,towupper)( wc, __lc_ctype);
}

