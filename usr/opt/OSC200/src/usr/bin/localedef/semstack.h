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
 * @(#)$RCSfile: semstack.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 23:13:08 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.3  com/cmd/nls/semstack.h, cmdnls, bos320 6/20/91 01:01:06 
 */

#ifndef _SEMSTACK_H_
#define _SEMSTACK_H_

#include "symtab.h"

typedef struct {
  wchar_t min;
  wchar_t max;
} range_t;


/* valid types for item_type */
typedef enum {SK_NONE, SK_INT, SK_STR, SK_RNG, SK_CHR, SK_SUBS, SK_SYM } item_type_t;

typedef struct {
  item_type_t	type;
  
  union {		     /* type =  */
    int        int_no;	     /*   SK_INT */
    char       *str;	     /*   SK_STR */
    range_t    *range;	     /*   SK_RNG */
    chr_sym_t  *chr;	     /*   SK_CHR */
    _LC_subs_t *subs;        /*   SK_SUBS */
    symbol_t   *sym;         /*   SK_SYM */
  } value;

} item_t;


/* semstack errors */
#define SK_OK       0
#define SK_OVERFLOW 1

/* semstack limits */
#define SK_MAX_DEPTH 16384

int sem_push(item_t *);
item_t *sem_pop(void);
void destroy_item(item_t *);
item_t *create_item(int, ...);
#endif /* _SEMSTACK_H_ */
