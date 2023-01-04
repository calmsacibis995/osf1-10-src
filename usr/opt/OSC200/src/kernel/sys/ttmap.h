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
 *	@(#)$RCSfile: ttmap.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:01:49 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/* com/inc/sys,3.1,8943E 11/9/89 14:06:57 */

/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 9, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _SYS_TTMAP_H_
#define _SYS_TTMAP_H_


/* structures for Terminal Mapping support */

#define TTMAP_VERSION	1	/* current map version (incompat w/version 0) */
#define TTMAP_NAMELEN	32	/* length of a ttmaps name (31 + null) */
#define TTMAP_BUFMAX	10	/* maximum number of buffered characters. */
#define TTMAP_MAXSIZE	131072	/* maximum size of a tty map */

#define TTMAP_PAT_SIZE	10	/* max length of pattern match string */
				/* pattern length is limited by $ meta-char */
#define TTMAP_REP_SIZE	16	/* max length of replacement string */

/* code values for tty_map.tm_flags */
#define TM_INPUT	1	/* operation applies to the input map */
#define TM_OUTPUT	2	/* operation applies to output map */
#define TM_CLEAR	4	/* no longer do mapping on this tty */
#define TM_USE		8	/* use named map for this tty */
/* only root may use the following */
#define TM_LOAD		0x10	/* load a map, may be for no specific tty */
#define TM_RELOAD	0x20	/* To force loading of a map even if loaded */
#define TM_STICKY	0x40	/* map will hang around after last user */
#define TM_DEFUNCT	0x80	/* map was replaced (used inside kernel) */


/* structure defining a single mapping rule */

struct ttrule {
       short   tm_next;		/* index of next rule to try if match fails */
       char    tm_pattern [TTMAP_PAT_SIZE+1];	/* pattern to try to match */
       char    tm_replace [TTMAP_REP_SIZE+1];	/* pattern to replace it with */
};			/* these sizes work out to an even number of words */

/*
 * Structure defining a single shared tty map
 * This is the actual map.  These float around in the kernel and are
 * chained together in a linked list.  One or more ttys can be pointing
 * at any given map, the info here is shared.  The array of rules is
 * variable in size and continues contiguous to the one element declared
 * here.  There are ttmap->tm_num_rules rules in the array, including the
 * one declared.  The length in ttmap->tm_len is the total overall length
 * of the map, used by ioctl routines for copying maps in and out.
 */

struct ttmap {
	struct ttmap	*tm_next;	/* next map in chain */
	int		tm_len;		/* total length of this map */
	char		tm_count;	/* how many ttys using this map */
	unsigned char	tm_flags;	/* sticky, defunct, etc */
	short		tm_num_rules;	/* number of xlate rules in map */
	short		tm_default;	/* index of default rule */
	short		tm_first_wild;	/* index of first wildcard rule */
	char		tm_mapname [TTMAP_NAMELEN];     /* map unique name */
	short		tm_hash [256];	/* hash table of rule indexes */
	struct ttrule	tm_rule [1];	/* variable size */
};

#define NULL_MAP	(struct ttmap *)0


/*
 * State information about a map in use.  Two of these per tty structure 
 * This structure lives in the actual tty structure, it holds the pointer
 * to the real map in use, if any, and holds state information about 
 * translations in progress.  There may not be enough characters queued
 * on a tty to unambiguously satisfy a rule at the time the kernel mapping
 * function is called, in that case, this structure is used to hold the
 * mapping state so that translation can continue when more characters
 * come in.  There is one of these structs for the tty input queue and
 * one for the output queue, each can point at different maps (usually do).
 */

struct ttmapinfo {
	struct ttmap	*tm_map;	/* null if no mapping here */
	char		tm_state;	/* state: user byte */
	char		tm_bufindx;	/* state: buffer index */
	short		tm_rulindx;	/* state: rules index */
	char		tm_buffer [TTMAP_BUFMAX]; /* Used internally */
};


/*
 * structure for ioctls TCSMAP and TCGMAP
 * This is a special structure for communicating with the kernel in order
 * manipulate maps.  This struct is passed as an argument to an ioctl and
 * tells the kernel what is to be done and to which maps or ttys.  See
 * comments about the flags above
 */

struct tty_map {
	char		tm_version;	/* Must be TTMAP_VERSION */
	unsigned char	tm_flags;	/* specifies what this operation is */
	char		tm_mapname [TTMAP_NAMELEN]; /* name of map */
	char		*tm_addr;	/* pointer to map in user space */
	int		tm_len;		/* length of map */
};

#endif /* _SYS_TTMAP_H_ */
