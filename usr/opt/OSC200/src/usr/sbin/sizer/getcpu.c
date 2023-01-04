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
static char     *sccsid = "@(#)$RCSfile: getcpu.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:35:40 $";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1987 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/


/************************************************************************
 *
 * Name: getcpu.c
 *
 * Modification History
 * 
 * Jun 5, 1991 - jaa
 *	ported to OSF/1.  modified to use new bus/controller/device
 *	kernel structures.  in porting, we don't use /dev/kmem anymore.
 *	getsysinfo(3) was modified to return the needed information.
 *	NOTE: ifdef TODO's in this code will have to be done as the 
 *	pieces are added (i.e. floating csr space, workstation specifics)
 *
 * July 6, 1989 - Alan Frechette
 *	Added check for a VAXSTAR cputype.
 *
 * May 02, 1989 - Alan Frechette
 *	Changes to deal with new unique "cpu" handling for both
 *	vax and mips architectures.
 *
 * Feb 12, 1989 - Alan Frechette
 *	New sizer code which supports multiple architectures.
 *	Restructured this code and cleaned it up considerably.
 *	Based on the original V3.0 sizer by Tungning Cherng.
 *
 ***********************************************************************/

#include "sizer.h"

/****************************************************************
*    getcpu							*
*    								*
*    Get cpu type that this program is running on.		*
****************************************************************/
getcpu(displayflag)
	int displayflag;
{
	int cputype, index;

	if(getsysinfo(GSI_CPU, &cputype, sizeof(cputype), 0, 0) == -1) 
		quitonerror(-4);

	/* Find cpu type in the "cputbl" table */
	for(index = 0; index < Cputblsize - 1; index++) {
		if((Cpu = cputbl[index].cputype) == cputype)
			break;
	}

	if(displayflag == DISPLAY) 
		fprintf(stdout, "cpu\t\t\"%s\"\n", cputbl[index].cpuname);
	else 
		fprintf(Fp, "cpu\t\t\"%s\"\n", cputbl[index].cpuname);
	return(index);
}

/****************************************************************
*    getmaxcpu							*
*								*
*    Get the maximum number of cpu's in the system.		*
****************************************************************/
getmaxcpu()
{

	if(getsysinfo(GSI_MAX_CPU, &Maxcpu, sizeof(Maxcpu), 0, 0) == -1)
		quitonerror(-4);

	if(Maxcpu == 0 || Maxcpu > 62)
		Maxcpu = 1;
}
