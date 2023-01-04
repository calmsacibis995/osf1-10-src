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
static char	*sccsid = "@(#)$RCSfile: putline.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:49:29 $";
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
 * FUNCTIONS: putline, flushline, xrm
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
/* putline.c 1.7 com/cmd/sccs/lib/comobj,3.1,9021 1/5/90 10:56:06"; */

# include	"defines.h"

/*
	Routine to write out either the current line in the packet
	(if newline is zero) or the line specified by newline.
	A line is actually written (and the x-file is only
	opened) if pkt->p_upd is non-zero.  When the current line from 
	the packet is written, pkt->p_wrttn is set non-zero, and
	further attempts to write it are ignored.  When a line is
	read into the packet, pkt->p_wrttn must be turned off.
*/

int	Xcreate;
FILE	*Xiop;


putline(pkt,newline)
register struct packet *pkt;
char *newline;
{
	static char obf[BUFSIZ];
	char *xf, *auxf();
	register char *p;
	FILE *fdfopen();

	char	*acl_get();
	char	*sptr;

	if(pkt->p_upd == 0) return;

	if(!Xcreate) {
		stat(pkt->p_file,&Statbuf);
		xf = auxf(pkt->p_file,'x');
                Xiop = xfcreat(xf,Statbuf.st_mode);
		setbuf(Xiop,obf);
		chown(xf,Statbuf.st_uid,Statbuf.st_gid);
	}
	if (newline)
		p = newline;
	else {
		if(!pkt->p_wrttn++)
			p = pkt->p_line;
		else
			p = 0;
	}
	if (p) {
		if(fputs(p,Xiop)==EOF)
			FAILPUT;
		if (Xcreate)
			while (*p)
				pkt->p_nhash += *p++;
	}
	Xcreate = 1;
}


flushline(pkt,stats)
register struct packet *pkt;
register struct stats *stats;
{
	register char *p;
	char ins[6], del[6], unc[6], hash[6];

	if (pkt->p_upd == 0)
		return;
	putline(pkt,(char *) 0);
	rewind(Xiop);

        if ( stats && (stats->s_ins > 99999 || stats->s_del > 99999 || stats->s_unc > 99999) ) {
                fprintf (stderr, MSGCO(LCNTWARN,"WARNING - The number of lines inserted, deleted or unchanged exceeds 99,999.\n"));
		if ( stats->s_ins > 99999 )
			stats->s_ins = 99999;
		if ( stats->s_del > 99999 )
			stats->s_del = 99999;
		if (stats->s_unc > 99999 )
			stats->s_unc = 99999;
        }

	if (stats) {
		sprintf(ins,"%.05u",stats->s_ins);
		sprintf(del,"%.05u",stats->s_del);
		sprintf(unc,"%.05u",stats->s_unc);
		for (p = ins; *p; p++)
			pkt->p_nhash += (*p - '0');
		for (p = del; *p; p++)
			pkt->p_nhash += (*p - '0');
		for (p = unc; *p; p++)
			pkt->p_nhash += (*p - '0');
	}

	sprintf(hash,"%5u",pkt->p_nhash&0xFFFF);
	zeropad(hash);
	fprintf(Xiop,"%c%c%s\n",CTLCHAR,HEAD,hash);
	if (stats)
		fprintf(Xiop,"%c%c %s/%s/%s\n",CTLCHAR,STATS,ins,del,unc);
	fclose(Xiop);
}


/*ARGSUSED*/
xrm(pkt)
struct packet *pkt;
{
	if (Xiop)
		fclose(Xiop);
	Xiop = 0;
	Xcreate = 0;
}
