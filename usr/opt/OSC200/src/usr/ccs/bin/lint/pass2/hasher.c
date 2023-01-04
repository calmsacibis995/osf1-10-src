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
static char	*sccsid = "@(#)$RCSfile: hasher.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/11/05 19:46:20 $";
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
 * FUNCTIONS: AddFtnUsage, ChangeSymbol, CheckSymbols, FtnRefSymbol,
	      LookupSymbol, StoreSymbol
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
 * hasher.c	1.3  com/cmd/prog/lint/pass2,3.1,9013 9/12/89 11:49:26";
 */

#include "lint_msg.h"
#define         MSGSTR(Num, Str) catgets(catd, MS_LINT, Num, Str)
nl_catd catd;

#include "mfile1.h"
#include "lint2.h"

/*
** The segmented hash bucket definitions.
*/
struct ht {
	SMTAB	*htabLo;	/* low address of hash bucket */
	SMTAB	*htabHi;	/* high address of hash bucket */
	int	htabCnt;	/* number of entries in hash bucket */
};
static struct ht htab[MAXHASH];	/* array of hash buckets */

/*
** Hash and store name in permanent storage area. Mode = 1 causes insertion.
*/
#include <string.h>
SMTAB *
LookupSymbol(p, stat)
	SMTAB *p;
	int stat;
{
  register struct ht *htp;
  register SMTAB *h;
  register int i;
  char *cp;
  int offset;
  
  /* Hash the complete name. */
  cp = p->sname;
  i = 0;
  while (*cp)
    i = (i << 1) + *cp++;
  offset = ((i < 0) ? -i : i) % HASHBLK;
  cp = p->sname;
  
  /* Look through each hash bucket for name. */
  for (htp = htab; htp < &htab[MAXHASH]; htp++) {
    
    /* Allocate hash bucket, if needed. */
    if (htp->htabLo == 0) {
      if (stat == STORE) {
	htp->htabLo = (SMTAB *) calloc(sizeof(SMTAB), HASHBLK);
	if (htp->htabLo == 0)
	  cerror(MSGSTR(M_MSG_258,
			"no memory for hash table"));
	htp->htabHi = htp->htabLo + HASHBLK;
      } else	/* Have a new table - return not found */
	return NULL;
    }

    h = htp->htabLo + offset;
    
    /* Use quadratic re-hash. */
    i = 1;
    do {
      if (h->sname == 0) {
	/* High-water mark set at 3/4 full. */
	if(htp->htabCnt > ((HASHBLK * 3) >> 2))
	  break;
	/* Symbol insertion. */
	if (stat == STORE) {
	  htp->htabCnt++;
	  StoreSymbol(h, p);
	  return (h);
	}
	return (0);
      }
      
      /* Symbol lookup. */
      if (!strcmp(h->sname, cp) &&
	  ((h->usage&SNSPACE) == (p->usage&SNSPACE)))
	return (h);
      
      /* Collision resolution. */
      h += i;
      i += 2;
      if (h >= htp->htabHi)
	h -= HASHBLK;
    } while (i < HASHBLK);
  }
  cerror(MSGSTR(M_MSG_259, "Ran out of hash tables"));
}

/*
** Store the symbol into a hashed slot.  Determine proper
** reference/definition context.
*/
StoreSymbol(h, p)
	SMTAB *h, *p;
{
	p->sname = StoreSName(p->sname);
	if (p->usage & LINTDEF) {
		p->dpf = curPFname;
		p->dif = curIFname;
		p->dl = curDLine;
	}
	if (p->usage & (LINTREF|LINTDCL)) {
		p->rpf = curPFname;
		p->rif = curIFname;
		p->rl = curRLine;
	}
	memcpy((char *) h, (char *) p, sizeof(SMTAB));
#ifndef	DEBUG
	if (debug)
		PrintSymbol("INSERT SYMBOL", h);
#endif
}

/*
** Change the reference/definition context of an existing symbol.
*/
ChangeSymbol(h, p)
	SMTAB *h, *p;
{
    /* Set usage bits except set only */
	h->usage |= (p->usage & ~LINTSTO);

	/* Turn off set only flag if new usage does not contain the flag */
	if (!(p->usage & LINTSTO))
	  h->usage &= ~LINTSTO;

	if (p->usage & LINTDEF) {
		h->dpf = curPFname;
		h->dif = curIFname;
		h->dl = curDLine;
		h->type = p->type;
	}
	if (p->usage & (LINTREF|LINTDCL)) {
		h->rpf = curPFname;
		h->rif = curIFname;
		h->rl = curRLine;
	}
#ifndef	DEBUG
	if (debug)
		PrintSymbol("CHANGE SYMBOL", h);
#endif
}

/*
** Reference function call.
*/
FtnRefSymbol(h, p)
	SMTAB *h, *p;
{
	if (p->usage & LINTREF) {
		h->usage |= LINTREF;
		h->rpf = curPFname;
		h->rif = curIFname;
		h->rl = curRLine;
	}
#ifndef	DEBUG
	if (debug)
		PrintSymbol("FTN REF SYMBOL", h);
#endif
}

/*
** Add function usage to existing function symbol.
*/
AddFtnUsage(h, p)
	SMTAB *h, *p;
{
	h->usage |= p->usage;
#ifndef	DEBUG
	if (debug)
		PrintSymbol("ADDFTN SYMBOL", h);
#endif
}

/*
** Examine each symbol for:
**	RefDefSymbol() - proper reference/definitions usage
**	FtnUsage() - consistent function usage
*/
CheckSymbols()
{
	register struct ht *htp;
	register SMTAB *h;

	/* Search each hash bucket. */
	for (htp = htab; htp < &htab[MAXHASH]; htp++) {
		if (htp->htabLo == 0)
			continue;

		/* Examine each symbol. */
		for (h = htp->htabLo; h < htp->htabHi; h++) {
			if (h->sname) {
				RefDefSymbol(h);
				if (ISFTN(h->type))
					FtnUsage(h);
			}
		}
	}
}
