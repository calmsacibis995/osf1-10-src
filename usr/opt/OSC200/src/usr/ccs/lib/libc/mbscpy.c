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
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak mbscpy = __mbscpy
#endif
#endif
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: mbscpy.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:27:44 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: mbscpy, wcscpy
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.7  com/lib/c/nls/mbscpy.c, libcnls, bos320, 9132320b 7/22/91 10:14:41
 */

/*
 * NAME: mbscpy
 *                                                                    
 * FUNCTION: Copy characters (code points) from one multibyte character 
 *	     string to another multibyte character string.
 *
 * PARAMETERS: 
 *	     char *s1 - overlaid string
 *	     char *s2 - copied string
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer equal to s1.
 */

char  *
mbscpy(char *s1, const char *s2)
{
	strcpy(s1,s2);
	return(s1);
}
