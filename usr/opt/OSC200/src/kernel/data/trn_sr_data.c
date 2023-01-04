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
static char *rcsid = "@(#)$RCSfile: trn_sr_data.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/03 22:25:26 $";
#endif

#include "sys/errno.h"
#include "tra.h"
#include "trsrcf.h"


#if NTRA || TRSRCF 
extern int sr_search();
extern void srtab_update();
extern int sr_ioctl();
extern void sr_init();
	int (*sr_search_func)() = sr_search;
	void (*srtab_update_func)() = srtab_update;
	int (*sr_ioctl_func)() = sr_ioctl;
	void (*sr_init_func)() = sr_init;
#else
extern int nulldev();
void nullvoid();
int sr_ioctl_nulldev();
	int (*sr_search_func)() = nulldev;
	void (*srtab_update_func)() = nullvoid;
	int (*sr_ioctl_func)() = sr_ioctl_nulldev;
	void (*sr_init_func)() = nullvoid;

void
nullvoid()
{
	return;
}

int
sr_ioctl_nulldev()
{
	return(ENODEV);
}
#endif
