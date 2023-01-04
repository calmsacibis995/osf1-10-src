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
static char	*sccsid = "@(#)$RCSfile: ltest1.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:45:55 $";
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
/* ltest1.c
 * Test file for tload2
 * Must be linked -A against tload2
 */

#include <stdio.h>

foo_entry()
{
	char	buf[512];
	int	val = 256;
	int	rc;

	printf("running in foo_entry\n");
	if (gets(buf) == NULL) {
		perror("gets error");
		return(0);
	}

	printf("current value is %d\n", val);

	if ((rc = sscanf(buf, "%d", &val)) != 1)
		printf("scanf didn't like %s, returned %d\n", buf, rc);

	printf("current value is %d\n", val);
	return(val);
}
