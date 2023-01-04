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
static char	*sccsid = "@(#)$RCSfile: locinfo.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:40:23 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * When adding a locale to libc, the following steps must be performed:
 * 
 * 1)  The collating table (ctab) information for the locale is transferred 
 *     into binary form using the ctab utility.
 * 2)  The locale is added to the libc Makefile.  This causes the environment
 *     information for the locale and the binary form of the ctab information
 *     to be used by the libloc utility to create a C-language source module.
 *     The libloc utility will take as an argument the name of the locale.
 *     This "name" is traditionally one or two upper case characters (such
 *     as DA for Danish).  If name is, for example, DA, then 'DA' (without
 *     the quotes) should be added to the libc Makefile, the binary form of
 *     the ctab information for this locale should reside in /etc/nls/DA,
 *     and the environment information for the locale should reside in 
 *     /etc/nls/DA.en.  The source module produced by the libloc utility
 *     will be compiled and archived into the library.
 * 3)  The locale's name (DA in the example above) is placed in the _locnms
 *     array below, surrounded by double quotes and followed by a comma. 
 *     The array must be NULL terminated.
 * 4)  The locale's name is added to the _locadr array below IN THE SAME 
 *     POSITION AS IT WAS ADDED TO _locnms!  i.e. if DA were made the 
 *     second element of _locnms, then '&DA,' (without the quotes) must 
 *     be the second element of _locadr.
 * 5)  The library is rebuilt.
 */

#include <NLctype.h>

char *_locnms[] = {
	"C",
	NULL
};

extern loc_t C;

loc_t *_locadr[] = {
	&C,
	NULL
};
