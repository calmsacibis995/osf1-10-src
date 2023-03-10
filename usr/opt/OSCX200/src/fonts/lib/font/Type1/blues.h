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
/* $XConsortium: blues.h,v 1.2 91/10/10 11:17:52 rws Exp $ */
/* Copyright International Business Machines, Corp. 1991
 * All Rights Reserved
 * Copyright Lexmark International, Inc. 1991
 * All Rights Reserved
 * Portions Copyright (c) 1990 Adobe Systems Incorporated.
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM or Lexmark or Adobe
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * IBM, LEXMARK, AND ADOBE PROVIDE THIS SOFTWARE "AS IS", WITHOUT ANY
 * WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  THE
 * ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE SOFTWARE, INCLUDING
 * ANY DUTY TO SUPPORT OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY
 * PORTION OF THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM,
 * LEXMARK, OR ADOBE) ASSUMES THE ENTIRE COST OF ALL SERVICING, REPAIR AND
 * CORRECTION.  IN NO EVENT SHALL IBM, LEXMARK, OR ADOBE BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
extern psobj *GetType1CharString();
 
#define TOPLEFT 1
#define BOTTOMRIGHT 2
 
#define NUMBLUEVALUES 14
#define NUMOTHERBLUES 10
#define NUMFAMILYBLUES 14
#define NUMFAMILYOTHERBLUES 10
#define NUMSTEMSNAPH 12
#define NUMSTEMSNAPV 12
#define NUMSTDHW 1
#define NUMSTDVW 1
 
#define DEFAULTBOLDSTEMWIDTH 2.0
 
#define MAXALIGNMENTZONES ((NUMBLUEVALUES+NUMOTHERBLUES)/2)
#define DEFAULTBLUESCALE 0.039625
#define DEFAULTBLUESHIFT 7
#define DEFAULTBLUEFUZZ 1
#define DEFAULTSTDHW 0
#define DEFAULTSTDVW 0
#define DEFAULTFORCEBOLD FALSE
#define DEFAULTLANGUAGEGROUP 0
#define DEFAULTRNDSTEMUP FALSE
#define DEFAULTLENIV 4
#define DEFAULTEXPANSIONFACTOR 0.06
 
/* see Type 1 Font Format book for explanations of these values */
/* Note that we're currently doing nothing for minfeature and password. */
struct blues_struct {
        struct blues_struct *next;   /* ptr to next Blues structure in list */
        int numBlueValues;   /* # of BlueValues in following array */
        int BlueValues[NUMBLUEVALUES];
        int numOtherBlues;   /* # of OtherBlues values in following array */
        int OtherBlues[NUMOTHERBLUES];
        int numFamilyBlues;   /* # of FamilyBlues values in following array */
        int FamilyBlues[NUMFAMILYBLUES];
        int numFamilyOtherBlues; /* # of FamilyOtherBlues values in  */
        int FamilyOtherBlues[NUMFAMILYOTHERBLUES]; /* this array */
        double BlueScale;
        int BlueShift;
        int BlueFuzz;
        double StdHW;
        double StdVW;
        int numStemSnapH;   /* # of StemSnapH values in following array */
        double StemSnapH[NUMSTEMSNAPH];
        int numStemSnapV;   /* # of StemSnapV values in following array */
        double StemSnapV[NUMSTEMSNAPV];
        int ForceBold;
        int LanguageGroup;
        int RndStemUp;
        int lenIV;
        double ExpansionFactor;
};
 
/* the alignment zone structure -- somewhat similar to the stem structure */
/* see Adobe Type1 Font Format book about the terms used in this structure */
struct alignmentzone {
        int topzone;        /* TRUE if a topzone, FALSE if a bottom zone */
        double bottomy, topy;       /* interval of this alignment zone */
};
