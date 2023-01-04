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
static char	*sccsid = "@(#)$RCSfile: outp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:27:51 $";
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
#ifndef lint
#ifndef _NOIDENT

#endif
#endif
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */
/*
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */ 

/*
 * COMPONENT_NAME: (CMDTEXT) Text Formatting Services
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26
 *
 * OBJECT CODE ONLY SOURCE MATERIALS
 *
 */

#include <stdio.h>
#include <ctype.h>
#include "style.h"
#include "names.h"
#include "conp.h"
char           *names[] = {
			   "noun", "verb", "interjection", "adjective", "adverb", "conjunction", "possessive",
			   "pronoun", "article", "preposition", "auxiliary", "tobe", "", "subordinate conjunction", "expletive"
};
extern int      barebones;
outp ()
{
    struct ss      *st;
    char           *ssp;
    char           *spart,
                    ff;
    int             index,
                    lverbc;
    int             nn,
                    sc,
                    f,
                    kk,
                    comp,
                    begsc;
    int             conjf,
                    verbf,
                    lpas,
                    bflg,
                    lexp,
                    olvb;
    int             nom;
    int             infinf,
                    ovflg;
    int             lvowel,
                    nlet;
    int             imper;
    float           rd;
    extern FILE    *deb;
    extern int      nosave;

    if (barebones)
    {
	for (sentp = sent; sentp -> cc != END; sentp++)
	    printf ("%s %c %c\n", sentp -> sp, sentp -> ic, sentp -> cc);
	printf ("%s %c %c\n", sentp -> sp, sentp -> ic, sentp -> cc);
	return;
    }
    if (topic)
    {
	for (sentp = sent; sentp -> cc != END; sentp++)
	{
	    if (sentp -> cc == ADJ && (sentp + 1) -> cc == NOUN)
	    {
		printf ("%s ", sentp -> sp);
		sentp++;
		printf ("%s\n", sentp -> sp);
	    } else
	    if (sentp -> cc == NOUN)
		printf ("%s\n", sentp -> sp);
	}
	return;
    }
    if (style)
    {
	nn = kk = 0;
	for (sentp = sent; sentp -> cc != END; sentp++)
	{
	    if (sentp -> cc != ',' && sentp -> cc != '"')
		nn++;
	    if (sentp -> cc == VERB || sentp -> cc == BE || sentp -> cc == AUX)
		kk++;
	}
	if (nn < 4 && kk == 0)
	    return;
    }
    imper = lexp = lpas = index = lverbc = nom = 0;
    conjf = verbf = kk = nn = sc = comp = begsc = 0;
    bflg = olvb = infinf = ovflg = 0;
    nlet = 0;
    f = 1;
    sentp = sent;
    while (sentp -> cc != END)
    {
	/* printf("%c:",sentp->ic);	 */
	if (sentp -> cc == ';')
	    comp++;
	else
	{
	    if ((sentp -> cc != ',') && (sentp -> cc != '"'))
	    {
		if (*sentp -> sp != 'x')
		{
		    nn++;
		    nlet += sentp -> leng;
		}
		kk++;
	    }
	}
	switch (sentp -> cc)
	{
	case NOUN:
	    spart = "noun";
	    if (f)
		index = 0;
	    if ((sentp -> ic == NOM) || (sentp -> ic == PNOUN && islower (*(sentp -> sp))))
	    {
		sentp -> ic = NOM;
		nom++;
		if (nosave && (deb != NULL))	/* SAVE NOM */
		    fprintf (deb, "%s\n", sentp -> sp);
	    }
	    if (*sentp -> sp != 'x')
	    {
		noun++;
		numnonf++;
		letnonf += sentp -> leng;
	    }
	    bflg = infinf = ovflg = 0;
	    break;
	case VERB:
	    spart = "verb";
	    if (f)
		index = 1;
	    if (sentp -> ic == TO)
	    {
		infin++;
		infinf = 1;
		lverbc++;
	    } else
	    {
		if (f)
		    imper = 1;
		if (ovflg == 0 && infinf == 0)
		{
		    ovflg = 1;
		    lverbc++;
		    olvb++;
		}
		numnonf++;
		letnonf += sentp -> leng;
		if (infinf == 0)
		{
		    if (verbf == 0)
			verbf++;
		    else
		    if (conjf)
			comp++;
		}
		if (bflg && sentp -> ic == ED)
		{
		    lpas++;
		    ++passive;
		}
	    }
	    break;
	case INTER:
	    spart = "interj";
	    if (f)
		index = 2;
	    bflg = infinf = ovflg = 0;
	    break;
	case ADJ:
	    spart = "adj";
	    if (f)
		index = 3;
	    adj++;
	    numnonf++;
	    if (sentp -> ic == NOM)
	    {
		nom++;
		if (nosave && (deb != NULL))	/* SAVE NOM */
		    fprintf (deb, "%s\n", sentp -> sp);
	    }
	    letnonf += sentp -> leng;
	    bflg = infinf = ovflg = 0;
	    break;
	case ADV:
	    spart = "adv";
	    if (f)
		index = 4;
	    adv++;
	    numnonf++;
	    letnonf += sentp -> leng;
	    break;
	case CONJ:
	    spart = "conj";
	    conjc++;
	    if (f)
		index = 5;
	    if (infinf && (sentp + 1) -> cc == VERB);
	    else
	    {
		if (verbf)
		    conjf++;
		bflg = infinf = ovflg = 0;
	    }
	    break;
	case POS:
	    spart = "pos";
	    if (f)
		index = 6;
	    bflg = infinf = ovflg = 0;
	    break;
	case PRONS:
	case PRONP:
	    spart = "pron";
	    pron++;
	    if (f)
	    {
		index = 7;
		if ((sentp + 1) -> cc == BE)
		{
		    if (sentp -> leng == 5 && *(sentp -> sp) == 't' && *((sentp -> sp) + 3) == 'r')
		    {
			index = 14;
			lexp = 1;
		    } else
		    if (sentp -> leng == 2 && *(sentp -> sp) == 'i')
		    {
			index = 14;
			lexp = 1;
		    }
		}
	    }
	    bflg = infinf = ovflg = 0;
	    if (sentp -> ic == THAT || sentp -> ic == WHO)
		sc++;
	    break;
	case ART:
	    spart = "art";
	    if (f)
		index = 8;
	    bflg = infinf = ovflg = 0;
	    break;
	case PREP:
	    spart = "prep";
	    if (f)
		index = 9;
	    prepc++;
	    bflg = infinf = ovflg = 0;
	    break;
	case AUXX:
	    spart = "aux";
	    if (ovflg == 0 && infinf == 0)
	    {
		ovflg = 1;
		lverbc++;
		olvb++;
		aux++;
	    }
	    if (f)
		index = 10;
	    break;
	case BE:
	    if (ovflg == 0 && infinf == 0)
	    {
		ovflg = 1;
		lverbc++;
		olvb++;
	    }
	    spart = "be";
	    if (f)
		index = 11;
	    tobe++;
	    bflg = 1;
	    if (verbf == 0)
		verbf++;
	    else
	    if (conjf)
		comp++;
	    break;
	case SUBCONJ:
	    spart = "subcj";
	    if (f)
	    {
		index = 13;
		begsc++;
	    }
	    sc++;
	    if ((sentp - 1) -> cc != CONJ)
		verbf = conjf = 0;
	    bflg = infinf = ovflg = 0;
	    break;
	default:
	    if (sentp -> cc == ',')
	    {
		if (begsc)
		    conjf = verbf = 0;
	    }
	    spart = sentp -> sp;
	}
	if (part)
	{
	    printf ("%s	%s\n", spart, sentp -> sp);
	}
	if (style)
	{
	    ssp = sentp -> sp;
	    lvowel = 0;
	    while (*ssp != '\0')
	    {
		if (*ssp >= '0' && *ssp <= '9')
		{
		    lvowel = 0;
		    break;
		}
		switch (*(ssp++))
		{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
		case 'y':
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
		case 'Y':
		    lvowel++;
		    continue;
		}
	    }
	    if (lvowel != 0)
	    {
		vowel += lvowel;
		twds++;
	    }
	}
	if (f)
	{
	    ff = sentp -> cc;
	    f = 0;
	}
	sentp++;
    }
    if (part)
    {
	printf (".	%s\n", sentp -> sp);
	if (sentno < MAXPAR && nn > 0)
	    leng[sentno++] = nn;
    }
    if (nn == 0)
	return;
    numsent++;
    numlet += nlet;
    tverbc += lverbc;
    verbc += olvb;
    if (*(sentp -> sp) == '?')
    {
	if (sc > 0)
	    sc -= 1;
	qcount++;
    } else
    if (*(sentp -> sp) == '/')
	icount++;
    else
    if (imper)
	icount++;
    if (rstyle || pstyle)
	rd = 4.71 * ((float) (nlet) / (float) (nn)) +.5 * (float) (nn) - 21.43;
    if (pstyle ||
	(rstyle && rd >= rthresh) || (lstyle && nn >= lthresh) || (pastyle && lpas) || (estyle && lexp)
	|| (nstyle && (nom > 1 || (nom && lpas))) || (Nstyle && nom))
    {
	if (!part)
	{
	    for (st = sent, kk = 0; st -> cc != END; st++)
	    {
		if (st -> ic == NOM)
		    printf ("*%s* ", st -> sp);
		else
		    printf ("%s ", st -> sp);
		if (kk++ >= 15)
		{
		    kk = 0;
		    printf ("\n");
		}
	    }
	}
	kk = 1;
    } else
	kk = 0;
    if (pstyle || kk)
    {
	if (!part)
	    printf ("%s\n", sentp -> sp);
	printf (" sentence length: %d ", nn);
	if (sc == 0)
	{
	    if (comp == 0)
		printf ("SIMPLE ");
	    else
		printf ("COMPOUND ");
	} else
	if (comp == 0)
	    printf ("COMPLEX ");
	else
	    printf ("COMPOUND-COMPLEX ");
	if (index == 14)
	    printf (":expletive:");
	if (lpas)
	    printf (":passive:");
	if (rstyle || pstyle)
	    printf (" readability %4.2f ", rd);
	printf (": begins with %s\n\n", names[index]);
    }
    if (index < 15)
	beg[index]++;
    if (nn > maxsent)
    {
	maxsent = nn;
	maxindex = numsent;
    }
    if (nn < minsent)
    {
	minsent = nn;
	minindex = numsent;
    }
    numwds += nn;
    if (nn > 49)
	nn = 49;
    sleng[nn]++;
    if (sc == 0)
    {
	if (comp == 0)
	    simple++;
	else
	    compound++;
    } else
    if (comp == 0)
	complex++;
    else
	compdx++;
}
