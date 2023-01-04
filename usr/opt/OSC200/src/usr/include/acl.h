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
 * @(#)$RCSfile: acl.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/12/15 22:12:28 $
 */
#if SEC_BASE 
#if SEC_ACL

/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	acl.h,v $
 * Revision 1.1.1.2  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.5.3.2  1992/04/08  20:30:58  marquard
 * 	Added POSIX ACL support.
 * 	[1992/04/05  14:23:00  marquard]
 *
 * Revision 1.5  1991/01/07  16:11:39  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  18:00:22  dwm]
 * 
 * Revision 1.4  90/10/07  20:25:27  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:42:25  gm]
 * 
 * Revision 1.3  90/08/24  13:52:20  devrcs
 * 	Changes for type-widening
 * 	[90/08/20  14:04:28  seiden]
 * 
 * Revision 1.2  90/06/22  21:51:33  devrcs
 * 	Initial version from SecureWare
 * 	[90/06/01  08:14:30  staffan]
 * 
 * $OSF_EndLog$
 */
/* @(#)$RCSfile: acl.h,v $ $Revision: 4.2.4.3 $ (OSF) $Date: 1993/12/15 22:12:28 $ */

/*	Copyright (c) 1988 SecureWare, Inc.  All rights reserved.

	This Module contains Proprietary Information of SecureWare, Inc.
	and should be treated as Confidential.

	This is the include file for Access Control List Defines and
	Structure Definitions.

	acl.h	5.1 15:35:34 8/15/90
	Based on:
	  "acl.h	2.1 12:33:23 1/25/89"
*/


#ifndef __ACL__
#define __ACL__

#if SEC_ACL_SWARE

/* Definition of Special ACL Owner/Group Id Values */

#define ACL_OWNER	   (-3)		/* match object owner */
#define ACL_WILDCARD	   (-4)		/* match any ID */


/* Mode Bit Defines for ACL Mode Word */

#define ACL_READ	0x4		/* ACL Read bit in mode */
#define ACL_WRITE	0x2		/* ACL Write bit in mode */
#define ACL_EXEC	0x1		/* ACL Exec bit in mode */
#define ACL_MAGIC       0xC1C3CCD3

/* Return status definitions for library routines */

#define	ACL_ERR		0
#define	ACL_OK		1

/* structure definitions for acl entries and arguments to ipc acl calls */

typedef struct {
	uid_t 	acl_uid;		/* user ID */
	gid_t	acl_gid;		/* group ID */
	ushort	acl_perm;		/* permissions */
} acle_t;

typedef struct {
	int	acl_size;		/* in:  elems in acl_entries */
					/* out: entries in acl */
	acle_t	*acl_entries;		/* pointer to ACL buffer */
} ipcacl_t;

/* Defines for ACL synonym database files */

#define	ACL_DBENV	"ACLSYNDB"
#define	ACL_DBFILE	".acl"

/* Layout of database file header */

struct dbhead {
	int		magic;		/* magic number */
	int		syns;		/* number of synonym structures */
	int		strings;	/* number of bytes of synonym names */
	int		entries;	/* number of ACL entries */
};

/* Structure of a synonym definition */

typedef struct aclsyn_t {
	char		*syn_name;	/* name of synonym */
	int		syn_count;	/* number of ACL entries */
	acle_t		*syn_ents;	/* array of ACL entries */
	struct aclsyn_t	*syn_next;	/* link to next synonym */
} aclsyn_t;

/* Defines for required ACL policy files */

#define ACL_PARAM_FILE  "/etc/policy/acl/config"

/* Routines provided by the ACL library */

#ifdef __cplusplus
extern "C" {
#endif
int             acl_delete_syn();       /* remove a synonym from the list */
int             acl_read_syn_db();      /* read a synonym database file */
int             acl_write_syn_db();     /* write a synonym database file */
int             acl_expand_syn();       /* increase size of synonym's ACL */
int             acl_load_syns();        /* load synonym database from a file */
int             acl_store_syns();       /* store synonym database to a file */
void            acl_free_syn();         /* deallocate an ACL synonym struct */
void            acl_insert_syn();       /* insert a synonym into the list */
aclsyn_t        *acl_lookup_syn();      /* lookup an ACL synonym */
aclsyn_t        *acl_alloc_syn();       /* allocate an ACL synonym struct */
#ifdef __cplusplus
}
#endif

#endif  /* SEC_ACL_SWARE */

/****************************************************************
***********/
/****************************************************************
***********/

#if SEC_ACL_POSIX
#include <sys/types.h>

#define INIT_ACL_SIZE   16
#define ACL_MAGIC       0xCCD3

typedef unsigned short  acl_tag_t;
typedef unsigned short  acl_permset_t;
typedef unsigned short  acl_type_t;
typedef unsigned short  acl_package_type_t;
/* typedef unsigned int    ssize_t;     */
/*      acl_package_type_t values       */

#define ACL_DATA_PACKAGE  1 /* binary representation of an ACL */
#define ACL_TEXT_PACKAGE  2 /* Null term, text representation of
an ACL. */

/*      acl_permset_t values    */

#define ACL_NOPERM              0x0
#define ACL_PEXECUTE            0x1
#define ACL_PWRITE              0x2
#define ACL_PREAD               0x4
#define ACL_PADD                0x10
#define ACL_PREMOVE             0x20

#define ACL_PRDWR               (ACL_PREAD|ACL_PWRITE)
#define ACL_PRDEX               (ACL_PREAD|ACL_PEXECUTE)
#define ACL_PWREX               (ACL_PWRITE|ACL_PEXECUTE)
#define ACL_PRDWREX             (ACL_PREAD|ACL_PWRITE|ACL_PEXECUTE)
/*      acl_tag_t values        */

#define BOGUS_OBJ               0
#define USER_OBJ                1
#define MASK_OBJ                2
#define USER                    3
#define GROUP_OBJ               4
#define GROUP                   5
#define OTHER_OBJ               6

#define NUM_TAG_TYPES           OTHER_OBJ

#define BOGUS_UID               (uid_t)-1
#define BOGUS_GID               (gid_t)-1

#define NUM_BASE_ENTRIES        3

/*      acl_type_t values       */

#define ACL_TYPE_ACCESS         1
#define ACL_TYPE_DEFAULT        2

typedef union {
        uid_t   _acl_uid;
        gid_t   _acl_gid;
} acl_id;

#define acl_uid         acl_id._acl_uid
#define acl_gid         acl_id._acl_gid

/*
 *      Descriptor for a specific ACL entry
 *      in internal representation.
 */

typedef struct {
        acl_tag_t       acl_tag;
        acl_permset_t   acl_perm;
        acl_id          acl_id;
} acle_t;

#define ACL_IR_SIZE     sizeof(acle_t)

/*
 *      Header for data package type.
 */

typedef struct {
        ushort  acl_num;
        ushort  acl_magic;
} aclh_t;

#define ACL_HDR_SIZE    sizeof (aclh_t)

/*
 *      Descriptor for specific ACL entry
 *      in (data package format).
 */

struct acl_data {
        aclh_t          acl_hdr;
/*      acle_t          *acl_entry;     comment; acl entries foll
ow */
};

#define ACL_DATA_SIZE   sizeof(struct acl_data)

typedef struct acl_data         *acl_data_t;
/*
 *      Descriptor for a specific ACL entry
 *      in ACL working storage.
 */

struct acl_entry {
        void            *acl_head;      /* pointer to acl structure */
        void            *acl_next;      /* pointer to next entry in list */
        int             acl_magic;      /* validation member */
        acl_tag_t       acl_tag;        /* tag type */
        acl_permset_t   acl_perm;       /* entry's permissions */
        acl_id          acl_id;         /* qualifier */
};

#define ACL_ENT_SIZE    sizeof(struct acl_entry)

typedef struct acl_entry        *acl_entry_t;

/*
 *      The ACL structure.
 */

struct acl {
        int             acl_magic;      /* validation member */
        int             acl_num;        /* number of actual acl entries */
        int             acl_size;       /* number of possible acl entries */
        int             acl_counter;    /* current entry's position */
        acl_entry_t     acl_first;      /* pointer to ACL linked list */
};

#define ACL_SIZE        sizeof(struct acl)

typedef struct acl      *acl_t;

#define PACL_PARAM_FILE "/etc/policy/acl/pconfig"

#ifdef __cplusplus
extern "C" {
#endif

/* Routines provided by the POSIX library */

int     check_wk_space ();      /* allocates additional acl space */
int     get_wildcard_acl ();    /* provides a base entry acl */
int     sort_acl ();            /* sorts acl into evaluating order */
int     acldaemon_init();       /* called by daemon */

/* POSIX library routines */
int     acl_add_perm ();
int     acl_alloc ();
int     acl_calc_mask ();
int     acl_copy_entry ();
int     acl_create_entry ();
int     acl_delete_entry ();
int     acl_delete_perm ();
int     acl_free ();
int     acl_get_entry ();
int     acl_rewind();
int     acl_get_perm ();
int     acl_get_tag ();
int     acl_read ();
int     acl_set_perm ();
int     acl_set_tag ();
int     acl_valid ();
int     acl_write ();
int     acl_unpack ();
ssize_t acl_pack ();
ssize_t acl_package_size ();

#ifdef __cplusplus
}
#endif
#endif  /* SEC_ACL_POSIX */

/* argument that causes access to be governed by mode bits */

#define ACL_DELETE	((acle_t *) -1)	/* remove ACL from object */

/* Definition of the ACL configuration parameter file */

struct acl_config {
	char		dbase[50];
	unsigned long	cache_size;
	unsigned long	buffers;
	unsigned short	subj_tags;
	unsigned short	obj_tags;
	unsigned short	first_subj_tag;
	unsigned short	first_obj_tag;
	dev_t		minor_device;
	unsigned short	policy;
};

/* Variables defined by the library */

extern struct acl_config acl_config;

/* Routines provided by the ACL library */

#ifdef __cplusplus
extern "C" {
#endif
int		acl_init();		/* init parms but not databases */
char		*acl_ir_to_er();	/* convert internal to external rep */
acle_t		*acl_er_to_ir();	/* convert external to internal rep */
int             acl_1ir_to_er();        /* convert 1 entry to external rep */
#ifdef __cplusplus
}
#endif

#endif  /* __ACL__ */
#endif /* SEC_ACL */
#endif	/* SEC_BASE */
