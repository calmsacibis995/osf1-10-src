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
 * @(#)$RCSfile: secpolicy.h,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/04/08 19:21:14 $
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/* @(#)secpolicy.h	6.2 15:10:06 2/17/91 SecureWare */
#ifndef __SECPOLICY__
#define __SECPOLICY__

/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 *
 * Based on:
 *	@(#)secpolicy.h	2.6.2.3 14:35:33 12/27/89 SecureWare
 *
 * This is unpublished proprietary source code of SecureWare, Inc. and
 * should be treated as confidential.
 */

#if SEC_ARCH

#include <sys/secdefines.h>
#ifdef _KERNEL
#include <kern/lock.h>
#endif

/*
 * The last slot in each object tag pool is reserved to hold an object
 * number for use in indexing into the process bitmaps used to maintain
 * information labels.  We therefore define a new constant for those
 * places that are interested in the number of policy tags, and leave
 * the old one for those places that care about the size of the tag pool.
 */
#define	SEC_NUM_TAGS	SEC_TAG_COUNT

/* Policy independent wildcard tag value */

#define SEC_WILDCARD_TAG_VALUE	0
#define	sec_wildcard(t)		(*(t) == SEC_WILDCARD_TAG_VALUE)

/* Maximum IR size that the kernel can handle */

#define SEC_MAX_IR_SIZE		8192

/*
 * Prototype structure for IRs imbedded in policy daemon messages.
 * The actual data associated with the IR immediately follows the
 * length field.
 */

typedef struct {
	ulong_t	ir_length;	/* length of internal representation */
} ir_t;

/* Prototype structure for decisions imbedded in policy daemon messages.
 * The actual data associated with the decision immediately follows the
 * length field.
 */

typedef struct {
	ulong_t	dec_length;	/* decision length */
} dec_t;

/*
 * Indicator for whether the internal representation comes from kernel
 * or user space for SP_MAPTAG().
 */

#define SEC_IR_KERNEL	0
#define SEC_IR_USER	1

/*
 * Security policy module/daemon message types
 */

#define SPD_MAP_TAG		1	/* Map Tag to IR */
#define SPD_SET_TAG		2	/* Set Tag on Object */
#define SPD_MAKE_DECISION	3	/* Make an Access Decision */
#define	SPD_DECISION		4	/* Decision from Daemon */
#define SPD_GET_ATTRIBUTE	5	/* Read IR for Tag */
#define SPD_INTERNAL_REP	6	/* Response with IR for Tag */
#define SPD_CHANGE_DISCR	7	/* Change Discretionary */
#define SPD_NEW_DISCR		8	/* Change Discretionary Response */
#define SPD_SHUTDOWN		9	/* Terminate the Daemon */
#define SPD_SHUTDOWN_RESP	10	/* ACK the Shutdown Request */
#define SPD_PREPARE_SHUTDOWN	11	/* Shutdown Notification */
#define SPD_PREPARE_RESP	12	/* Shutdown Notification Response */
#define SPD_ERROR		13	/* Error Message type */
/*
 * POSIX ACLS
 */
#define SPD_GET_NEW_OBJTAG	14	/* Get Tag For New Object */
#define SPD_SET_NEW_OBJTAG	15	/* Set Tag For New Object */



/*
 * Security policy module/daemon message formats
 */

/*********** Common Security Policy Message Header Format ************/

typedef struct spd_message_header {
	long	sync_id;	/* process sync id for driver */
	short	msg_type;	/* request-response type */
	short	mh_flags;	/* state field */
	short	error_code;	/* error return for audit */
} mhdr_t;

#define SPD_MHDR_SIZE	(sizeof(mhdr_t))

/*********** Message Pair for Mapping IRs to Tags ************/

struct spd_map_tag {
	mhdr_t		mhdr;		/* message header */
	udac_t		unixdac;	/* owner/creator uid/gid, mode */
	short		which_tag;	/* policy-specific tag number */
	attrtype_t	ir_type;	/* is tag for subject or object */
	ir_t		ir;		/* IR to be mapped to a tag */
};

#define SPD_MAP_TAG_SIZE	(sizeof(struct spd_map_tag))

struct spd_set_tag {
	mhdr_t		mhdr;		/* message header */
	tag_t		tag;		/* tag returned */
	ulong_t		obj_flags;	/* POSIX ACLS -- SEC_NEW_ID etc. */
	dac_t		obj_discr;	/* object discretionary */
};

#define SPD_SET_TAG_SIZE	(sizeof(struct spd_set_tag))

/*********** Message Pair for Requesting Policy Decisions ************/

struct spd_make_decision {
	mhdr_t	mhdr;			/* message header */
	tag_t	subject;		/* subject tag */
	tag_t	object;			/* object tag */
};

#define SPD_MAKE_DEC_SIZE	(sizeof(struct spd_make_decision))

struct spd_decision {
	mhdr_t	mhdr;			/* message header */
	tag_t	subject;		/* subject tag */
	tag_t	object;			/* object tag */
	dec_t	decision;		/* policy decision */
};

#define SPD_DECISION_SIZE	(sizeof(struct spd_decision))

/*********** Message Pair for Mapping Tags to IRs ************/

struct spd_get_attribute {
	mhdr_t		mhdr;		/* message header */
	tag_t		tag;		/* tag to map to ir */
	attrtype_t	tag_type;	/* subject or object */
};

#define SPD_GET_ATTR_SIZE	(sizeof(struct spd_get_attribute))

struct spd_internal_rep {
	mhdr_t	mhdr;			/* message header */
	tag_t	tag;			/* tag that was mapped */
	ir_t	ir;			/* internal representation */
};

#define SPD_INTERNAL_REP_SIZE	(sizeof(struct spd_internal_rep))

/*********** Message Pair for Discretionary Changes ************/

struct spd_change_discr {
	mhdr_t		mhdr;		/* message header */
	tag_t		object_tag;	/* object tag value */
	dac_t		object_dac;	/* new owner, group, and mode */
	attrtype_t	object_type;	/* subject or object */
	ushort_t	object_flags;	/* indicates which dac changed */
};

#define SPD_CHANGE_DISCR_SIZE	(sizeof(struct spd_change_discr))

struct spd_new_discr {
	mhdr_t		mhdr;		/* message header */
	tag_t		object_tag;	/* new object tag */
	dac_t		object_discr;	/* new owner, group, and mode */
	ushort_t	object_flags;	/* indicates which dac to change */
};

#define SPD_NEW_DISCR_SIZE	(sizeof(struct spd_new_discr))

/*********** POSIX ACLS begin ************/
/*********** Message Pair for Object Creation ************/

struct spd_object_create {
	mhdr_t	mhdr;			/* message header */
	tag_t	access_tag;		/* parents access tag value */
	tag_t	default_tag;		/* parents default tag value */
	edac_t	edac;			/* effective uid and gid */
	int	mode;			/* mode requested */
};

#define SPD_OBJECT_CREATE_SIZE	(sizeof(struct spd_object_create))

struct spd_new_objtag {
	mhdr_t	mhdr;			/* message header */
	tag_t	access_tag;		/* new access tag */
	tag_t	default_tag;		/* new default tag */
	int	mode;			/* mode returned */
};

#define SPD_NEW_OBJTAG_SIZE	(sizeof(struct spd_new_objtag))

/*********** POSIX ACLS end ************/


/*
 * Error Codes for Message Passing Functions
 */

#define SPD_OK		0		/* No error */
#define SPD_EFULLDB	1		/* Database is full */
#define SPD_ENOSUBJTAG	2		/* No subject tag found */
#define SPD_ENOOBJTAG	3		/* No object tag found */
#define SPD_EBADIREP	4		/* Bad internal representation */
#define SPD_EDEADLOCK	5		/* Re-send message after wait */
#define SPD_EBADMSG	6		/* Bad message type */

/*
 * Security Policy Magic Numbers for Configuraton Recognition
 * Changes to these definitions must be reflected in the policy_name[]
 * table in seccmd/reduce.h.
 */

#define SEC_ACL_MAGIC		0xf000	/* SecureWare Access Control Lists */
#define SEC_MAC_MAGIC		0xf001	/* Mandatory Access Control */
#define SEC_NCAV_MAGIC		0xf003	/* Nationality Caveats */
#define SEC_MACILB_MAGIC	0xf004	/* MAC and Information Labels */
#define SEC_PACL_MAGIC		0xf010	/* POSIX Access Control Lists */

#define SEC_MAGIC_MAX		SEC_PACL_MAGIC

#define	SEC_MAGIC_COUNT		(((unsigned short) SEC_MAGIC_MAX - \
					(unsigned short) SEC_ACL_MAGIC) + 1)
/*
 * Map a tag within a policy to a tag pool offset
 */

/*
 * Ioctl Structure Definitions 
 */

/* Structure for SPIOC_INIT and SPIOC_GETCONF commands */

struct sp_init {
	ulong_t		cache_size;		/* number of cache entries */
	ulong_t		cache_width;		/* decision size */
	ushort_t	magic;			/* policy magic number */
	ushort_t	spminor;		/* policy minor device */
	ushort_t	policy;			/* policy number */
	ushort_t	first_subj_tag;		/* first subject tag */
	ushort_t	subj_tag_count;		/* subject tag count */
	ushort_t	first_obj_tag;		/* first object tag */
	ushort_t	obj_tag_count;		/* object tag count */
};

#define SP_INITSIZE	(sizeof(struct sp_init))

/* Structure for SPIOC_INV_TAG command */

struct sp_inv_tag {
	tag_t	tag;			/* tag to invalidate */
};

/* Structure for SPIOC_SET_CACHE_SIZE command */

struct sp_set_cache_size {
	ulong_t	cache_size;		/* new cache size */
	ulong_t	cache_width;		/* new cache width */
};

/* Structure for SPIOC_GET_STATS command */

struct sp_mod_stats {
	ulong_t	decisions;		/* decisions made */
	ulong_t	queries;		/* daemon queries made */
	ulong_t	responses;		/* responses from daemon */
	ulong_t	collisions;		/* cache entry collisions */
	ulong_t	delay;			/* aggregate delay */
	ulong_t	max_delay;		/* maximum decision delay */
	ulong_t	cache_size;		/* number of cache entries */
	ulong_t	cache_width;		/* decision size */
};

#define SP_MOD_STAT_SIZE	(sizeof(struct sp_mod_stats))

#ifdef KERNEL
extern struct sp_mod_stats sp_mod_stats[];
#endif

/**************** Security Policy Configuration Definitions ****************/

/* Security Policy Configuration and Message Driver Files */

#define SP_CONFIG_FILE		"/tcb/files/sp_config"
#define SP_DAEMON_DEVICE	"/dev/spd"

/* Security Policy Configuration Devices */

#define	SPD_CONTROL_DEVICE	"/dev/spdcontrol"
#define SPD_CONTROL_MINOR	0xff		/* minor device number */

/* Security Policy Configuration File Format */

struct sec_policy_conf {
	char	daemon[64];	/* policy daemon executable */
	char	confdir[64];	/* policy daemon config directory */
};

#ifdef KERNEL

/*
 * Each policy daemon may have a decision cache.
 * Cache management routines in sec/spd_cache.c manipulate these cache
 * entries.
 */

#define DCACHE_BUCKETS	4		/* number of cache buckets per slot */

/* Definition of each element of the decision cache */

struct dcache_entry {
	tag_t	tag1;			/* first tag to hash */
	tag_t	tag2;			/* second tag to hash */
	time_t	time;			/* LRU time value */
	caddr_t	decision;		/* decision pointer */
	struct {
		uint_t	valid:1;	/* valid entry flag */
	} dc_flags;
};

/*
 * This is the definition of a decision cache hash slot.
 * 
 *			 MP locking notes
 * On multiprocessor systems, a spin lock exists for each hash
 * slot in a cache.  The lock protects all buckets in that slot.  No
 * thread will ever hold more than one cache slot lock at a time, nor
 * will it pend with a cache slot lock held.
 */
 
struct decision_cache {
	decl_simple_lock_data(, lock)
	struct dcache_entry bucket[DCACHE_BUCKETS];
};

typedef struct decision_cache dec_cache_t;

extern dec_cache_t **dcachep;

#define DEC_CACHE_ENTRY_SIZE	(sizeof(struct decision_cache))
#endif

#if SEC_ACL
/*
 * Definitions for Access Control Lists
 */

#if SEC_ACL_SWARE

typedef struct {
	udac_t		acl_head;	/* Creator/owner uid/gid, mode */
/*	acle_t		acl_entry[1];	COMMENT - Securieware ACL entries */
} acl_t;

/* SecureWare ACL Internal Representation Structure */

struct acl_ir {
	ulong_t	ir_length;		/* length of ir */
	acl_t	ir;			/* ACL header and entries */
};

/* tag allocation */

#define ACL_SUBJ_TAG	0	/* subject identity */
#define ACL_OBJ_TAG	0	/* object access control list */

#define ACL_HEADERSIZE	(sizeof(udac_t))
#endif /* SEC_ACL_SWARE */

#if SEC_ACL_POSIX

typedef struct {
	cdac_t		acl_head;	/* Creator uid/gid */
/*	acl_data_t	acl_entry[1];	COMMENT - POSIX ACL en tries */
} pacl_t;

/* POSIX ACL Internal Representation Structure */
struct acl_ir {
	ulong_t	ir_length;		/* length of ir */
	pacl_t	ir;			/* ACL header and entries */
};


#define PACL_SUBJ_TAG		0	/* subject identity */
#define PACL_OBJ_ACCESS_TAG	0	/* object access control list */
#define PACL_OBJ_DFLT_TAG	1	/* default access control list */

#define PACL_HEADERSIZE (sizeof(cdac_t))

#endif /* SEC_ACL_POSIX */

/* Access Control List Policy Special Tag Values */

#define SEC_ACL_NULL_TAG	1L	/* empty ACL-no access */

/* Access Control List Cache Decision Format */

struct acl_decision {
	ulong_t	dec_length;		/* decision length */
	struct {
		uint_t	read:1,		/* subject may read object */
			write:1,	/* subject may write object */
			exec:1,		/* subject may execute object */
			delete:1,	/* subject may delete object */
			rfu:28;
	} dec;
};

typedef struct acl_decision acl_dec_t;

#define ACL_DECISION_SIZE	(sizeof(struct acl_decision) - sizeof(long))

struct acl_cache_decision {
	struct {
		uint_t	read:1,		/* subject may read object */
			write:1,	/* subject may write object */
			exec:1,		/* subject may execute object */
			delete:1,	/* subject may delete object */
			rfu:28;
	} decision;
};

typedef struct acl_cache_decision acache_dec_t;

#define	ACL_CACHE_WIDTH	(sizeof(struct acl_cache_decision))

#endif /* SEC_ACL */

#if SEC_MAC

/*
 * Definitions for Mandatory Access Control
 */

/* MAC Special Tag Values */

#define SEC_MAC_NULL_TAG	0L	/* no access */
#define SEC_MAC_SYSLO_TAG	2L	/* system lo tag */
#define SEC_MAC_SYSHI_TAG	3L	/* system hi tag */

/* MAC tag allocation */

#define MAND_SUBJ_SL_TAG	0	/* subject sensitivity label */
#define MAND_OBJ_SL_TAG		0	/* object sensitivity label */
#define MAND_SUBJ_CL_TAG	1	/* subject clearance */

/* MAC Internal Representation Structure */

struct mac_ir {
	ulong_t		ir_length;	/* length of ir */
	ulong_t		class;		/* classifications */
					/* compartments and markings follow */
};

#define MAC_IR_SIZE	(sizeof(struct mac_ir))

/* MAC Decision Format */

struct mac_decision {
	ulong_t	dec_length;		/* decision length */
	struct {
		uint_t	subj_dom:1,	/* subject dominates object */
			obj_dom:1,	/* object dominates subject */
			same:1,		/* levels are equal */
			incomp:1,	/* levels are incomparable */
			rfu:28;
	} dec;
};

typedef struct mac_decision mac_dec_t;

#define MAC_DECISION_SIZE	(sizeof(struct mac_decision) - sizeof(long))

/* MAC Cache Decision Format */

struct mac_cache_decision {
	struct {
		uint_t	subj_dom:1,	/* subject dominates object */
			obj_dom:1,	/* object dominates subject */
			same:1,		/* levels are equal */
			incomp:1,	/* levels are incomparable */
			rfu:28;
	} decision;
};

typedef struct mac_cache_decision mcache_dec_t;

#define	MAC_CACHE_WIDTH	(sizeof(struct mac_cache_decision))

/* Return values from mac_getdecision */

#define	MAC_SDOM	1
#define	MAC_ODOM	2
#define	MAC_INCOMP	4
#define	MAC_SAME	8

#endif /* SEC_MAC */

#ifdef KERNEL
/*
 * This is the definition of the Security Policy Switch. This
 * switch serves as the connection between the various security
 * policy checks such as access or object create and the Security
 * Modules that are responsible for making the policy decisions.
 */

struct sec_module_desc {
	int	(*sw_init)	(/* policy */);
	int	(*sw_access)	(/* subjtags,objtags,mode,dcheck,udac */);
	int	(*sw_obj_create)(/* subjtags,objtags,partags,attrtype */);
	int	(*sw_obj_delete) (/* subjtags,objtags,attrtype */);
	int	(*sw_getattr)	(/* attrtype,tag,attr */);
	int	(*sw_setattr)	(/* attrtype,tags,tagnum,new */);
	int	(*sw_setattr_check) (/* objtype,objp,parent,tags,tagnum,new */);
	int	(*sw_maptag) 	(/* attrtype,tagnum,attr,udac,new */);
	ushort_t	sw_magic;
	ushort_t	sw_priv_required;
	char	sw_subj_count;
	char	sw_obj_count;
	char	sw_first_subj_tag;
	char	sw_first_obj_tag;
};

extern struct sec_module_desc sp_switch[];
extern int spolicy_cnt;

/*
 * Macros to access elements of the policy switch
 */
#define	SP_INIT(policy)		(*sp_switch[policy].sw_init)(policy)
#define	SP_GETATTR(policy,attrtype,tag,attr)\
	(*sp_switch[policy].sw_getattr)(attrtype,tag,attr)
#define	SP_SETATTR(policy,attrtype,tags,tagnum,new)\
	(*sp_switch[policy].sw_setattr)(attrtype,tags,tagnum,new)
#define	SP_SETATTR_CHECK(policy,objtype,objp,par,tags,tagnum,new)\
	(*sp_switch[policy].sw_setattr_check)(objtype,objp,par,tags,tagnum,new)
#define	SP_MAPTAG(policy,attrtype,tagnum,attr,udac,new)\
	(*sp_switch[policy].sw_maptag)(attrtype,tagnum,attr,udac,new)

#define	OBJECT_TAG(policy,which)  (sp_switch[policy].sw_first_obj_tag + which)
#define	SUBJECT_TAG(policy,which) (sp_switch[policy].sw_first_subj_tag + which)
#define	SUBJECT_TAG_COUNT(policy) (sp_switch[policy].sw_subj_count)
#define	OBJECT_TAG_COUNT(policy)  (sp_switch[policy].sw_obj_count)

/*
 * Security Policy Miscellaneous Function Table
 */

struct sec_functions {
	int	(*sw_owner)		(/* owner, creator */);
	int	(*sw_change_subj)	();
	int	(*sw_change_obj)	(/* tags, newdac, flags */);
	int	sw_discr_index;
};

extern struct sec_functions sp_functions;

/*
 * Macros to call members of the sec_functions table
 */

#define	sec_owner(ouid,cuid)	(*sp_functions.sw_owner)(ouid, cuid)

#define	SP_CHANGE_SUBJECT()	\
	(security_is_on ? (*sp_functions.sw_change_subj)() : 0)
#define	SP_CHANGE_OBJECT(tags,newdac,flags)\
	(security_is_on ? (*sp_functions.sw_change_obj)(tags,newdac,flags) : 0)


/*
 * Macros and declarations for tag pools that are allocated separately
 * from their associated object data structures (IPC objects and ptys).
 */

extern tag_t semtag[], shmtag[], msgtag[];

#define SEMTAG(sp,t)	(&semtag[((((sp) - sema) * SEC_TAG_COUNT) + (t))])
#define SHMTAG(sp,t)	(&shmtag[((((sp) - shmem) * SEC_TAG_COUNT) + (t))])
#define MSGTAG(mp,t)	(&msgtag[((((mp) - msgque) * SEC_TAG_COUNT) + (t))])

#if SEC_PTY

extern tag_t ptctag[], ptstag[];

#define	PTCTAG(d,t)	(&ptctag[(minor(d) * SEC_TAG_COUNT) + (t)])
#define	PTSTAG(d,t)	(&ptstag[(minor(d) * SEC_TAG_COUNT) + (t)])

#endif /* SEC_PTY */

/*
 * External declarations for policy specific information (sec_conf.c)
 */

#if SEC_ACL
extern int paclpolicy, paclobjtag, paclsubjtag;
#endif /* SEC_ACL */


#if SEC_MAC
extern int macpolicy, macobjtag, macsubjtag, macclrnce;
#endif



#endif /* KERNEL */

#else /* SEC_ARCH */

#define	sec_owner(ouid,cuid)\
	(u.u_uid==(ouid) || u.u_uid==(cuid) || privileged(SEC_OWNER, EPERM))
#endif /* SEC_ARCH */
#endif /* __SECPOLICY__ */
