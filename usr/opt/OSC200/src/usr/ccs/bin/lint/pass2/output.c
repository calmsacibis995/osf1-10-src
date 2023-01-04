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
static char	*sccsid = "@(#)$RCSfile: output.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/11/05 19:47:29 $";
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
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: LERROR, PrintSymbol, tprint
 *
 * ORIGINS: 00 03 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * output.c	1.4  com/cmd/prog/lint/pass2,3.1,9013 12/22/89 14:31:48";
 */

#include "lint_msg.h"
#define         MSGSTR(Num, Str) catgets(catd, MS_LINT, Num, Str)
nl_catd catd;

#include "mfile1.h"
#include "lint2.h"

/*
** Lint pass2 standard error message.
*/
LERROR(mode, s, p, use)
	int mode, use;
	char *s;
	SMTAB *p;
{
	char *pn, *in;
	short l;
#ifdef LINT
        /* if migration testing allow ONLY error when the print flag is on */
        if (AL_MIGCHK && (!AL_PRINTIT))
	  return;
	AL_PRINTIT = 0;
#endif

	if (mode) {
		switch (use) {
		case CRUSE:
			pn = curPFname;
			in = curIFname;
			l = curRLine;
			break;

		case CDUSE:
			pn = curPFname;
			in = curIFname;
			l = curDLine;
			break;

		default:
			use = RUSE; /* FIx case where use is bad.. */
		case DUSE:
		case RUSE:
			/* If the pfname for use is NULL then use the other one */
			if (!p->rd[use].pfname) {
			  use = !use;
			}
			pn = p->rd[use].pfname;
			in = p->rd[use].ifname;
			l = p->rd[use].line;
			break;
		}
		if (!in || !strcmp(pn, in))
			printf(MSGSTR(M_MSG_263, "\"%s\", line %d: warning: "), pn, l);
		else
			printf(MSGSTR(M_MSG_264, "\"%s\", line %d (\"%s\"): warning: "),
				in, l, pn);
		printf(s, p->sname);
		printf("\n");
	}
}

#ifndef	DEBUG
/*
** Debug mode symbolic diagnostic.
*/
PrintSymbol(s, p)
	char *s;
	SMTAB *p;
{
	register MBTAB *m;
	static char done = 0;

	if (!done) {
		printf("DPF (DIF, DL)/RPF (RIF, RL): Symbol <Type> Usage\n");
		printf("\tMember <Type> Tagname...\n");
		printf("------------------------------------------\n");
		++done;
	}

	/* Print message, if there is one. */
	if (s && *s)
		printf("%s\n", s);

	/* Print symbol information. */
	if (!p->dpf)
	  printf("       .../");
	else if (!p->dif || !strcmp(p->dpf, p->dif))
		printf("%s (%d)/", p->dpf, p->dl);
	else
		printf("%s (%s, %d)/", p->dpf, p->dif, p->dl);

	if (!p->rpf)
	  printf("       ...<");
	else if (!p->rif || !strcmp(p->rpf, p->rif))
	  printf("%s (%d): %s <", p->rpf, p->rl, p->sname);
	else
	  printf("%s (%s, %d): %s <", p->rpf, p->rif, p->rl, p->sname);
	tprint(p->type); printf("> 0%o\n", p->usage);

	/* Print each member, if any. */
	if (p->nmbrs) {
		m = p->mbrs;
		while (m) {
			printf("\t%s <", m->mname);
			tprint(m->type); printf("> %s\n", m->tagname);
			m = m->next;
		}
	}
}

/*
** Output a description of the type t.  This function must remain
** consistent with the ordering in pcc/m_ind/mfile1.h .  The same
** function exists in m_ind/treewalk.h .
*/
tprint(t)
	TPTR t;
{
	register PPTR p;
	TWORD bt;
/* The ordering of this array corresponds to that in m_ind/manifest.h */
char * tnames[NBTYPES] = {
  "null",
  "ellipsis",
  "farg",
  "moety",
  "SIGNED",
  "undef",
  "VOID",
  "CHAR",
  "SIGNED CHAR",
  "SHORT",
  "INT",
  "LONG",
  "LONG LONG",
  "FLOAT",
  "DOUBLE",
  "LONG DOUBLE",
  "STRTY",
  "UNIONTY",
  "ENUMTY",
  "UNSIGNED CHAR",
  "UNSIGNED SHORT",
  "UNSIGNED",
  "UNSIGNED LONG",
  "UNSIGNED LONG LONG"
  };

	for( ;; t = DECREF(t) ){

		if( ISCONST(t) ) printf( "const " );
		if( ISVOLATILE(t) ) printf( "volatile " );

		if( ISPTR(t) ) printf( "PTR " );
 		else if( ISFTN(t) ){
 			printf( "FTN (" );
			if( ( p = t->ftn_parm ) != PNIL ){
			  if (p < (PPTR)100L) {
			    printf("Value of p == 0X%LX ", p);
			  } else
				for( ;; ){
					tprint( p->type );
					if( ( p = p->next ) == PNIL ) break;
					printf( ", " );
				}
 			}
			printf( ") " );
 		}
		else if( ISARY(t) ) printf( "ARY[%.0d] ", t->ary_size );
		else {
			if( ISTSIGNED(t) ) printf( "<signed> " );
			if( HASCONST(t) ) printf( "<HASCONST> " );
			if( HASVOLATILE(t) ) printf( "<HASVOLATILE> " );
			if ((bt = TOPTYPE(t)) >= NBTYPES) {
			  printf("Invalid type\n");
			} else {
			  printf( tnames[bt = TOPTYPE(t)] );
			  printf( "(0%o)", t->typ_size );
			}
			return;
		}
	}
}
#endif

