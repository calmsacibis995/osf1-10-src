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
/* $XConsortium: minimain.c,v 1.2 91/10/10 11:18:25 rws Exp $ */
 
#include "ximager5.h"
 
main()
{
       XYspace S;
       path p;
 
       InitImager();
       S = Scale(IDENTITY, (double)300.0, (double)-300.0);
       p = Join(Line(Loc(S, (double)0.0, (double)1.0)), 
	Line(Loc(S, (double)1.0, (double)0.0)));
       Interior(ClosePath(p), EVENODDRULE);
}
 
void Trace()
{
}
 
void *DEFAULTDEVICE;
