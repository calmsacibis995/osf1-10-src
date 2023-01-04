/*
 *	*****************************************************************
 *	*                                                               *
 *	*    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 *	*                                                               *
 *	*   All Rights Reserved.  Unpublished rights  reserved  under   *
 *	*   the copyright laws of the United States.                    *
 *	*                                                               *
 *	*   The software contained on this media  is  proprietary  to   *
 *	*   and  embodies  the  confidential  technology  of  Digital   *
 *	*   Equipment Corporation.  Possession, use,  duplication  or   *
 *	*   dissemination of the software and media is authorized only  *
 *	*   pursuant to a valid written license from Digital Equipment  *
 *	*   Corporation.                                                *
 *	*                                                               *
 *	*   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 *	*   by the U.S. Government is subject to restrictions  as  set  *
 *	*   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 *	*   or  in  FAR 52.227-19, as applicable.                       *
 *	*                                                               *
 *	*****************************************************************
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: inp.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/06/11 21:09:06 $ */

/* Header: inp.h,v 2.0 86/09/17 15:37:25 lwall Exp
 *
 * Log:	inp.h,v
 * Revision 2.0  86/09/17  15:37:25  lwall
 * Baseline for netwide release.
 * 
 */

EXT LINENUM input_lines INIT(0);	/* how long is input file in lines */
EXT LINENUM last_frozen_line INIT(0);	/* how many input lines have been */
					/* irretractibly output */

bool rev_in_string();
void scan_input();
bool plan_a();			/* returns false if insufficient memory */
void plan_b();
char *ifetch();

