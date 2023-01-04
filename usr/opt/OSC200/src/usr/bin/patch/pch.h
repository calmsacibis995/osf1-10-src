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
/* @(#)$RCSfile: pch.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/06/11 21:14:32 $ */

/* Header: pch.h,v 2.0.1.1 87/01/30 22:47:16 lwall Exp
 *
 * Log:	pch.h,v
 * Revision 2.0.1.1  87/01/30  22:47:16  lwall
 * Added do_ed_script().
 * 
 * Revision 2.0  86/09/17  15:39:57  lwall
 * Baseline for netwide release.
 * 
 */

EXT FILE *pfp INIT(Nullfp);		/* patch file pointer */

void re_patch();
void open_patch_file();
void set_hunkmax();
void grow_hunkmax();
bool there_is_another_patch();
int intuit_diff_type();
void next_intuit_at();
void skip_to();
bool another_hunk();
bool pch_swap();
char *pfetch();
short pch_line_len();
LINENUM pch_first();
LINENUM pch_ptrn_lines();
LINENUM pch_newfirst();
LINENUM pch_repl_lines();
LINENUM pch_end();
LINENUM pch_context();
LINENUM pch_hunk_beg();
char pch_char();
char *pfetch();
char *pgets();
void do_ed_script();
