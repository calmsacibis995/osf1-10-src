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
/*   $RCSfile: UilLexDef.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/07 00:34:58 $ */

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      This include file defines the interface to the UIL lexical
**	analyzer.
**
**--
**/

#ifndef UilLexDef_h
#define UilLexDef_h


/*
**  Define flags to indicate whether certain characters are to be
**  filtered in text output.
*/

#define		lex_m_filter_tab	(1 << 0)

/*
**  Define the default character set.  In Motif, the default character set is
**  not isolatin1, but simply the null string, thus we must be able to
**  distinguish the two.
*/


#define lex_k_default_charset	    -1
#define lex_k_userdefined_charset   -2
#define lex_k_fontlist_default_tag  -3

#endif /* UilLexDef_h */
/* DON'T ADD STUFF AFTER THIS #endif */
