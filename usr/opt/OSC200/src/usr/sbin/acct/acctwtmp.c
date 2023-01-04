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
static char	*sccsid = "@(#)$RCSfile: acctwtmp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:00:02 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *      acctwtmp reason >> /var/adm/wtmp
 *	writes utmp.h record (with current time) to end of std. output
 *      acctwtmp `uname` >> /var/adm/wtmp as part of startup
 *      acctwtmp pm >> /var/adm/wtmp  (taken down for pm, for example)
 */

#include <stdio.h>
#include "acctdef.h"
#include <sys/acct.h>
#include <sys/types.h>

#include <locale.h>
#include "acct_msg.h"
#define	MSGSTR(Num, Str)	NLgetamsg(MF_ACCT, MS_ACCT, Num, Str)


static struct	utmp	wb;

main(int argc, char **argv)
{
	int i;

	(void) setlocale (LC_ALL,"");
	if(argc < 2) {
		(void)fprintf(stderr,MSGSTR( USAGEWTMP, 
			"Usage: %s 'Reason'\n"), argv[0]);
		exit(1);
	}

	(void)strncpy(wb.ut_line, argv[1], LSZ);
	for( i = LSZ-1; i >= 0 && wb.ut_line[i] == ' '; i-- )
		wb.ut_line[i] = '\0';
	wb.ut_type = ACCOUNTING;
	(void)time(&wb.ut_time);
	(void)fseek(stdout, 0L, 2);
	(void)fwrite(&wb, sizeof(wb), 1, stdout);

	return(0);
}
