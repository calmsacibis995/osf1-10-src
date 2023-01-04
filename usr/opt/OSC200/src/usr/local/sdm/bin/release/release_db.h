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
 *	@(#)$RCSfile: release_db.h,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:02:40 $
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

#if	USE_REFERENCE_COUNTING
#define	DECL_REFCNT	u_short refcnt;
#else	/* USE_REFERENCE_COUNTING */
#define	DECL_REFCNT
#endif	/* USE_REFERENCE_COUNTING */

/* table of pointers */
struct table {
    u_short cur;
    u_short max;
    union {
	char **u_tab;
	struct string **u_stab;
	struct file_info **u_fitab;
	struct file **u_ftab;
	struct dir **u_dtab;
    } tab_u;
#define stab	tab_u.u_stab
#define fitab	tab_u.u_fitab
#define ftab	tab_u.u_ftab
#define dtab	tab_u.u_dtab
};

/* string header */
struct strhdr {
    struct strhdr *sh_first;
#define sh_next sh_first
    struct strhdr *sh_last;
#define sh_prev sh_last
};

/* string structure */
struct string {
    struct strhdr sh;		/* string header */
    u_short index;		/* index */
    DECL_REFCNT			/* reference count */
    u_short len;		/* length of string */
    u_char ahash;		/* add hash value */
    u_char xhash;		/* xor hash value */
    char *data;			/* contents */
};

struct loginfo {
    DECL_REFCNT			/* reference count */
    struct string *installer;	/* installer */
    struct string *message;	/* message */
    time_t itime;		/* installation time */
    time_t wtime;		/* when to warn about release status */
};

struct instance {
    struct instance *next;	/* next instance of this file */
    u_short index;		/* index for this instance */
    DECL_REFCNT			/* reference count */
    struct loginfo *loginfo;	/* log information */
    u_short flags;		/* flags */
#define	IF_COMPRESSED	1	/* file is compressed */
    time_t ctime;		/* last time "inode" changed */
    time_t mtime;		/* last time file data modified */
};

struct file_info {
    u_short index;		/* index for this file info */
    DECL_REFCNT			/* reference count */
    struct string *owner;	/* owner name */
    struct string *group;	/* group name */
    u_short mode;		/* file permission mode */
};

struct file {
    u_short index;		/* index for this file */
    DECL_REFCNT			/* reference count */
    struct dir *parent;		/* directory that contains this file */
    struct file *links;		/* other links to this file */
    struct dir *srcdir;		/* directory to build this file */
    struct string *name;	/* file name (shared) */
    struct file_info *info;	/* file info (shared) */
    struct instance *instances;	/* instances of file for distribution */
};

struct dir {
    u_short index;		/* index for this directory */
    DECL_REFCNT			/* reference count */
    struct dir *parent;		/* directory that contains this directory */
    struct string *name;	/* directory name (shared) */
    struct table ftable;	/* table of files in directory */
    struct table dtable;	/* table of subdirectories in directory */
    struct file_info *info;	/* directory info (shared) */
};
