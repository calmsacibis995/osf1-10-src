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
static char *rcsid = "@(#)$RCSfile: Locore.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/12 12:48:04 $";
#endif

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/types.h>

/*
 * Description:
 *	This file is used during the 'lint' process, to declare
 * assembler functions defined in .s files.  The declarations are
 * broken into sub-groups based on the file name they reside in.
 *
 */ 
/*LINTLIBRARY*/

/****************************************************************************/
/*
 * File: arch/alpha/fastcopy.s
 */
int bcopy(char *source, char *destination, int length) { return 0; }
int bcmp(char *string1, char *string2, int length) { return 0; }
size_t strlen(char *s) { return 0; }
void bzero(char *string, int length) { return; }

/****************************************************************************/
/*
 * File: arch/alpha/usercopy.s
 */
int copyinstr(char *user_src, char *kernel_dest, int maxlength, int *lencopied) { return 0; }
int copyoutstr(char *kernel_src, char *user_dest, int maxlength, int *lencopied) { return 0; }

int fubyte(char *user_addr) { return 0; }
int fuibyte(char *user_addr) { return 0; }

int fuword(char *user_addr) { return 0; }
int fuiword(char *user_addr) { return 0; }

long fuqword(char *user_addr) { return 0; }
long fuiqword(char *user_addr) { return 0; }

int subyte(char *user_addr, char value) { return 0; }
int suibyte(char *user_addr, char value) { return 0; }

int suword(char *user_addr, int value) { return 0; }
int suiword(char *user_addr, int value) { return 0; }

int suqword(char *user_addr, long value) { return 0; }
int suiqword(char *user_addr, long value) { return 0; }

/****************************************************************************/
