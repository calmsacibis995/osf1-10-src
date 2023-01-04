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
static char	*sccsid = "@(#)$RCSfile: sinit.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:49:38 $";
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
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: sinit
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* sinit.c 1.4 com/cmd/sccs/lib/comobj,3.1,9021 9/15/89 14:17:06"; */

# include	"defines.h"

/*
	Does initialization for sccs files and packet.
*/

sinit(pkt,file,openflag)
register struct packet *pkt;
register char *file;
{
	extern	char	*satoi();
	register char *p;
	FILE *fdfopen();
	char *getline();

	zero((char *)pkt,sizeof(*pkt));
	if (size(file) > FILESIZE)
		fatal(MSGCO(PATHTOOLNG, 
			    "\nThe maximum path name is 510 characters.(co7)\n"));
	if (!sccsfile(file))
		fatal(MSGCO(NOTSCCS, 
			    "\nSpecify an SCCS file with this command.(co1)\n"));
	copy(file,pkt->p_file);
	pkt->p_wrttn = 1;
	pkt->do_chksum = 1;	/* turn on checksum check for getline */
	if (openflag) {
		pkt->p_iop = xfopen(file,0);
		setbuf(pkt->p_iop,pkt->p_buf);
		fstat(fileno(pkt->p_iop),&Statbuf);
		if (openflag == 1 && Statbuf.st_nlink > 1)
			fatal(MSGCO(MRTHNONELNK, 
                         "\nSCCS files can have only one link or name.(co3)\n"));
		if ((p = getline(pkt)) == NULL || *p++ != CTLCHAR || *p++ != HEAD) {
			fclose(pkt->p_iop);
			fmterr(pkt);
		}
		p = satoi(p,&pkt->p_ihash);
		if (*p != '\n')
			fmterr(pkt);
	}
	pkt->p_chash = 0;
}
