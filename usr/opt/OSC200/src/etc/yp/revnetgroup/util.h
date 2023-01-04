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
 *      @(#)$RCSfile: util.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:00:02 $
 */
/*
 */

#define EOS '\0'

#ifndef NULL 
#	define NULL ((char *) 0)
#endif


#define MALLOC(object_type) ((object_type *) malloc(sizeof(object_type)))

#define FREE(ptr)	free((char *) ptr) 

#define STRCPY(dst,src) \
	(dst = malloc((unsigned)strlen(src)+1), (void) strcpy(dst,src))

#define STRNCPY(dst,src,num) \
	(dst = (char *) malloc((unsigned)(num) + 1), \
	strncpy(dst,src,num), \
	(dst)[num] = EOS) 

extern char *malloc();
extern char *alloca();

char *getline();
void fatal();


