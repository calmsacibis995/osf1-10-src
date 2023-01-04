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
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmResCvt.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:12:10 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void AddWmResourceConverters ();
extern void WmCvtStringToCFocus ();
extern void WmCvtStringToCDecor ();
extern void WmCvtStringToCFunc ();
extern void WmCvtStringToIDecor ();
extern void WmCvtStringToIPlace ();
extern void WmCvtStringToKFocus ();
extern void WmCvtStringToSize ();
extern void WmCvtStringToShowFeedback ();
extern void WmCvtStringToUsePPosition ();
extern unsigned char *NextToken ();
extern Boolean StringsAreEqual ();
#else /* _NO_PROTO */
extern void AddWmResourceConverters (void);
extern void WmCvtStringToCFocus (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToCDecor (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToCFunc (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToIDecor (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToIPlace (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToKFocus (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToSize (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToShowFeedback (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToUsePPosition (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern unsigned char *NextToken (unsigned char *pchIn, int *pLen, unsigned char **ppchNext);
extern Boolean StringsAreEqual (unsigned char *pch1, unsigned char *pch2, int len);
#endif /* _NO_PROTO */
