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
static char     *sccsid = "@(#)$RCSfile: getroot.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/06/04 09:43:55 $";
#endif lint
/************************************************************************
 *									*
 *			Copyright(c) 1987 by				*
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
 * Name: getroot.c
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
 * May 26, 1989 - Tim Burke
 *	Allow ra disks to occupy a range of major numbers to support
 *	larger numbers of disks.
 *
 * May 10, 1989 - Alan Frechette
 *	Figure out network boot device if booting from network.
 *
 * Feb 12, 1989 - Alan Frechette
 *	New sizer code which supports multiple architectures.
 *	Restructured this code and cleaned it up considerably.
 *	Based on the original V3.0 sizer by Tungning Cherng.
 *
 ***********************************************************************/

#include "sizer.h"

/****************************************************************
*    getroot							*
*								*
*    Get the root, swap, and dump devices.			*
****************************************************************/
getroot(displayflag)
	int displayflag;
{
	int pt, majornum, unitnum, root;
	int on_network;
	char dname[10];
	static char part[] = {'a','b','c','d','e','f','g','h'};

	if(getsysinfo(GSI_ROOTDEV, &root, sizeof(root), 0, 0) == -1)
		quitonerror(-5);
	
	/* Pick off "majornum" number, partition and "unitnum" number */
	majornum = major(root);
	pt = GETDEVS(root);
	on_network = 0;

	/*
	 * We should be using the devices file but it
	 * hasn't been updated yet.  For now we're safe
	 * since we only support rz's for booting.
	 */
	switch(majornum) {
	      /* SCSI */
	      case 8:
		unitnum = GETCAMUNIT(root);
		sprintf(dname, "rz%d", unitnum);
		break;
	      /* MSCP */
	      case 23:
		unitnum = GETUNIT(root);
		sprintf(dname, "ra%d", unitnum);
		break;
	      default:
		quitonerror(-5);
		break;
	}

	if(displayflag == NODISPLAY) {
		fprintf(Fp, "config\t\tvmunix\t");
#ifdef TODO
		/*
		 * the OSF/1 kernel currently ignores what's in the
		 * config file.  When that's changed, someone'll have
		 * to fix this
		 */
		if(!getconfig_string("root")) {
			fprintf(Fp, "root on %s", dname);
			fprintf(Fp, "%c  ", on_network ? "" : part[pt]);
		}
#endif /* TODO */
	} else 
		fprintf(stdout, "%s\n", dname);

}

getswap(displayflag)
	int displayflag;
{
	int pt, majornum, unitnum, swap;
	int on_network;
	char dname[10];
	static char part[] = {'a','b','c','d','e','f','g','h'};

#ifndef TODO
	/*
	 * the OSF/1 kernel currently ignores what's in the
	 * config file.  When that's changed, someone'll have
	 * to fix this
	 */
	fprintf(Fp, "swap generic");
#else /* TODO */
	if(getsysinfo(GSI_SWAPDEV, &swap, sizeof(swap), 0, 0) == -1)
		quitonerror(-5);
	majornum = major(swap);
	pt = GETDEVS(swap);
	on_network = 0;

	switch(majornum) {
	      /* SCSI */
	      case 8:
		unitnum = GETCAMUNIT(swap);
		sprintf(dname, "rz%d", unitnum);
		break;
	      /* MSCP */
	      case 23:
		unitnum = GETUNIT(swap);
		sprintf(dname, "ra%d", unitnum);
		break;
	      default:
		quitonerror(-5);
		break;
	}

	if(displayflag == NODISPLAY) {
		if(!getconfig_string("swap")) {
			fprintf(Fp, "swap on %s", dname);
			fprintf(Fp, "%c  ", on_network ? "" : part[pt]);
		}
	} else 
		fprintf(stdout, "%s\n", dname);
#endif /* TODO */
}

#ifdef notdef
/****************************************************************
*    getonnetwork						*
*								*
*    Get the network boot device name.				*
****************************************************************/
getonnetwork(rootname)
char *rootname;
{
	int roottype;
	long offset;

	/***
	 *** this has to go into getsysinfo 
	 ***/
    	if(nl[NL_roottype].n_type == N_UNDF)
		quitonerror(-14);
	offset = reset_anythg(NL_roottype);
	lseek(Kmem,offset,0);
	read(Kmem,&roottype,sizeof(roottype));
	strcpy(rootname, (roottype == GT_NFS ? "se0" : "boot"));
}
#endif /* notdef */
