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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: gename.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:05:16 $";
#endif
/* 
 * COMPONENT_NAME: UUCP gename.c
 * 
 * FUNCTIONS: gename, getseq, initSeq, sysseq 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
gename.c	1.5  com/cmd/uucp,3.1,9013 11/30/89 13:40:20";
*/
/*	/sccs/src/cmd/uucp/s.gename.c
	gename.c	1.1	7/29/85 16:32:57
*/
#include "uucp.h"

/* VERSION( gename.c	5.2 -  -  ); */

static struct {
	char	sys[NAMESIZE];
	int	job;
	int	subjob;
} syslst[30];		/* no more than 30 systems per job */

static int nsys = 0;
static int sysseq();

 /* generate file name
  *	pre	-> file prefix
  *	sys	-> system name
  *	grade	-> service grade 
  *	file	-> buffer to return filename must be of size DIRSIZ+1
  * return:
  *	none
  */
gename(pre, sys, grade, file)
char pre, *sys, grade, *file;
{
	int	n;

	DEBUG(9, "gename(%c, ", pre);
	DEBUG(9, "%s, ", sys);
	DEBUG(9, "%c)\n", grade);
	if (*sys == '\0') {
		sys = Myname;
		DEBUG(9, MSGSTR(MSG_GE1, "null sys -> %s\n"), sys);
	}
	n = sysseq(sys);
	if (pre == CMDPRE || pre == XQTPRE) {
		(void) sprintf(file, "%c.%.*s%c%.4x",
			pre, SYSNSIZE, sys, grade,syslst[n].job); 
	} else
		(void) sprintf(file, "%c.%.5s%.4x%.3x",
			pre, sys, syslst[n].job & 0xffff,
				++syslst[n].subjob & 0xfff); 
	DEBUG(4, MSGSTR(MSG_GE2, "file - %s\n"), file);
}


#define SLOCKTIME 10
#define SLOCKTRIES 5
#define SEQLEN 4

 /*
  * get next sequence number
  * returns:  
  *	number between 1 and 0xffff
  *
  * sequence number 0 is reserved for polling
  */
static int
getseq(sys, in_syslst)
char	*sys;
int     in_syslst;
{
	register FILE *fp;
	register int i;
	int n;
	int	seed;
	char seqlock[MAXFULLNAME], seqfile[MAXFULLNAME];

	ASSERT(nsys < sizeof (syslst)/ sizeof (syslst[0]),
	    MSGSTR(MSG_GE3, "SYSLST OVERFLOW"), "", sizeof (syslst));

	(void) time((time_t *)&seed);	/* crank up the sequence initializer */
	srand(seed);

	(void) sprintf(seqlock, "%s%s", SEQLOCK, sys);
	BASENAME(seqlock, '/')[MAXBASENAME] = '\0';
	for (i = 1; i < SLOCKTRIES; i++) {
		if (!ulockf(seqlock, (time_t) SLOCKTIME))
			break;
		sleep(5);
	}

	ASSERT(i < SLOCKTRIES, Ct_LOCK, seqlock, 0);

	(void) sprintf(seqfile, "%s/%s", SEQDIR, sys);
	if ((fp = fopen(seqfile, "r")) != NULL) {
		/* read sequence number file */
		if (fscanf(fp, "%4x", &n) != 1) {
		    n = rand();
		    clearerr(fp);
		}
		fp = freopen(seqfile, "w", fp);
		ASSERT(fp != NULL, Ct_OPEN, seqfile, errno);
		(void) chmod(seqfile, 0666);
	} else {
		/* can not read file - create a new one */
		ASSERT((fp = fopen(seqfile, "w")) != NULL,
		    Ct_CREATE, seqfile, errno);
		(void) chmod(seqfile, 0666);
		n = rand();
	}

	n++;
	n &= 0xffff;	/* 4 byte sequence numbers */
	(void) fprintf(fp, "%.4x\n", n);
	ASSERT(ferror(fp) == 0, Ct_WRITE, seqfile, errno);
	(void) fclose(fp);
	ASSERT(ferror(fp) == 0, Ct_CLOSE, seqfile, errno);
	rmlock(seqlock);
	DEBUG(6, MSGSTR(MSG_GE4, "%s seq "), sys); DEBUG(6, 
		 MSGSTR(MSG_GE5, "now %x\n"), n);

	if( in_syslst ) {
	  syslst[in_syslst].job = n;
	  syslst[in_syslst].subjob = rand() & 0xfff; /* random initial value */
	  return(in_syslst);
	}

	(void) strcpy(syslst[nsys].sys, sys);
	syslst[nsys].job = n;
	syslst[nsys].subjob = rand() & 0xfff;	/* random initial value */
	return(nsys++);
}

static int
sysseq(sys)
char	*sys;
{
	int	i;
	int     in_syslst = 0;

	for (i = 0; i < nsys; i++)
		if (strncmp(syslst[i].sys, sys, SYSNSIZE) == SAME) {
		  syslst[i].job = 0;
		  syslst[i].subjob = 0;
		  in_syslst = i;
		}

	return(getseq(sys,in_syslst));
}

/*
 *	initSeq() exists because it is sometimes important to forget any
 *	cached work files.  for example, when processing a bunch of spooled X.
 *	files, we must not re-use any C. files used to send back output.
 */
void
initSeq()
{
	nsys = 0;
}
