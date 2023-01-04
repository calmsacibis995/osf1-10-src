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
/********************************************************************/
/*                    Compiler Revisions									  */
/*																						  */
/*  init   Date        Revision/Changes                             */
/*	 ----   ----        ----------------									  */
/*  bsb   9/7/90       1.0	/ Initial general customer release       */
/********************************************************************/
/*                    portable.h Revisions								  */
/*																						  */
/*  init   Date        Revision/Changes									  */
/*  ----   ----        -----------------									  */
/*  bsb   9/7/90       1.0 / Initial general customer release       */
/********************************************************************/


/*
    PORTABLE.H - Machine-dependent definitions go here
*/
#ifndef port
#define port
#define YACC_HEADER "ytab.h"

#include <string.h>

typedef int INT32;
typedef unsigned int UINT32;
typedef short BOOL;
#endif
