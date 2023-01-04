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
 *	@(#)$RCSfile: parse_rc_file.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:11:20 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * data structures used to hold parsed rc file for sandboxes
 */

struct arg_list {
    struct arg_list *next;
    char **tokens;
    int ntokens;
    int maxtokens;
};

struct field {
    struct field *next;		/* next field */
    char *name;			/* name of this field */
    struct arg_list *args;	/* args for this field */
};

#define RC_HASHBITS 6		/* bits used in hash function */
#define RC_HASHSIZE (1<<RC_HASHBITS) /* range of hash function */
#define RC_HASHMASK (RC_HASHSIZE-1)	/* mask for hash function */
#define RC_HASH(i) ((i)&RC_HASHMASK)

struct hashent {
    struct hashent *next;
    struct field *field;
};

struct rcfile {
    struct hashent *hashtable[RC_HASHSIZE];
    struct field *list;
    struct field *last;
};
