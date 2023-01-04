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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
static char rcsid[] = "@(#)$RCSfile: diff.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 18:20:19 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * diff.h	1.2  com/cmd/files/diff,3.1,9008 9/11/89 15:43:43 
 */

#include "diff_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_DIFF,Num,Str)

#define	DI_NORMAL	0	/* Normal output */
#define	DI_EDIT		-1	/* Editor script out */
#define	DI_REVERSE	1	/* Reverse editor script */
#define	DI_CONTEXT	2	/* Diff with context */
#define	DI_IFDEF	3	/* Diff with merged #ifdef's */
#define	DI_NREVERSE	4	/* Reverse ed script with numbered */

