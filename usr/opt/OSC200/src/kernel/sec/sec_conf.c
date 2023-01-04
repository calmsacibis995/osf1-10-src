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
static char *rcsid = "@(#)$RCSfile: sec_conf.c,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/04/08 19:20:50 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	sec_conf.c,v $
 * Revision 1.1.1.3  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.6.4.3  1992/04/08  20:25:22  marquard
 * 	Merged up to latest.
 * 	[1992/04/08  19:19:29  marquard]
 *
 * 	Added POSIX ACL support.
 * 	[1992/04/04  16:41:49  marquard]
 *
 * Revision 1.6.4.2  1992/03/16  17:15:20  hosking
 * 	bug 4436: delete code under conditionals that are never enabled for OSF/1
 * 	[1992/03/12  09:56:50  hosking]
 * 
 * Revision 1.6  1990/12/06  14:03:49  devrcs
 * 	Cleanup copyright and history log comments.
 * 	[90/11/15  11:08:49  gm]
 * 
 * Revision 1.5  90/10/07  14:42:09  devrcs
 * 	Fixed up EndLog Marker.
 * 	[90/09/30  16:07:09  gm]
 * 
 * 	Added EndLog Marker.
 * 	[90/09/28  11:27:07  gm]
 * 
 * Revision 1.4  90/08/24  12:20:14  devrcs
 * 	doc changes only
 * 	[90/08/14  15:37:43  hosking]
 * 
 * 	Bump up SMP_Configured revisions.
 * 	[90/07/10  21:56:48  seiden]
 * 
 * Revision 1.2  90/06/22  20:47:14  devrcs
 * 	Initial version from SecureWare.
 * 	[90/06/10  01:03:16  seiden]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All Rights Reserved.
 */

/* #ident "@(#)sec_conf.c	4.1 21:44:49 7/9/90 SecureWare" */
/*
 * Based on:	@(#)sec_conf.c	2.7.1.2 11:19:08 12/27/89
 */

#include <sec/include_sec>

#if SEC_BASE

/*	Global symbol to define system generation type */

#if !defined(SMP_CONFIGURED) && SEC_MAC
char SMP_Configured[] = "B1 3.1";
#define SMP_CONFIGURED
#endif

#if !defined(SMP_CONFIGURED)
char SMP_Configured[] = "C2 3.1";
#define SMP_CONFIGURED
#endif

#undef SMP_CONFIGURED

/*
 * Security Info Structure Definition
 *
 * This is the global structure that is allocated dynamically in
 * sec_init() from main() to match the proc in the number of entries.
 * Each process on the system maps to one of the secinfo slots
 * that keeps all of the security relevant parameters.
 */

struct security_info *secinfo;

#if SEC_ARCH

/*	Extern declarations	*/

#if SEC_ACL

/* SecureWare ACL policy */

#if SEC_ACL_SWARE
extern	aclinit(), aclaccess(), aclcreate(), acldelete(), aclgetattr(),
	aclsetattr(), aclsetattr_check(), aclmaptag();
#endif

/* 	
 * POSIX ACL policy 
 */

#if SEC_ACL_POSIX
extern	paclinit(), paclaccess(), paclcreate(), pacldelete(), paclgetattr(),
	paclsetattr(), paclsetattr_check(), paclmaptag();
#endif /* SEC_ACL_POSIX */

#else /* !SEC_ACL */

/* If no ACL policy defined, these routines emulate traditional UNIX behavior */

extern	udacinit(), udacaccess(), udaccreate(), udacdelete(), udacgetattr(),
	udacsetattr(), udacsetattr_check(), udacmaptag();
#endif /* !SEC_ACL */

#if SEC_MAC
extern	macinit(), macaccess(), maccreate(), macdelete(), macgetattr(),
	macsetattr(), macsetattr_check(), macmaptag();
#endif /* SEC_MAC */

/*
 *	This is the definition of the security policy switch entry
 *	points. If tag positions change, these entries must also be
 *	changed to reflect the new ordering. If the ACL (DAC) policy
 *	minor is changed, CHANGE sp_functions index below!!!!
 *	Configure this switch to match the configuration of your system.
 *	The definition of sec_module_desc is in sys/secpolicy.h.
 */

struct sec_module_desc sp_switch[] = {

#if SEC_ACL
#if SEC_ACL_SWARE
/* 0 */	{ aclinit, aclaccess, aclcreate, acldelete, aclgetattr,
		aclsetattr, aclsetattr_check, aclmaptag,
		SEC_ACL_MAGIC, SEC_ALLOWDACACCESS, 1, 1
	},
#endif /* SEC_ACL_SWARE */


#if SEC_ACL_POSIX
/* 0 */ { paclinit, paclaccess, paclcreate, pacldelete, paclgetattr,
                paclsetattr, paclsetattr_check, paclmaptag,
                SEC_PACL_MAGIC, SEC_ALLOWDACACCESS, 2, 2
        },
#endif /* SEC_ACL_POSIX */

#else /* !SEC_ACL */
/* 0 */	{ udacinit, udacaccess, udaccreate, udacdelete, udacgetattr,
		udacsetattr, udacsetattr_check, udacmaptag,
		0, SEC_ALLOWDACACCESS, 0, 0
	},
#endif /* !SEC_ACL */

#if SEC_MAC
/* 1 */	{ macinit, macaccess, maccreate, macdelete, macgetattr,
		macsetattr, macsetattr_check, macmaptag,
		SEC_MAC_MAGIC, SEC_ALLOWMACACCESS, 2, 1
	},
#endif /* SEC_MAC */

};

#define	SPOLICY_CNT	(sizeof sp_switch / sizeof sp_switch[0])
int spolicy_cnt = SPOLICY_CNT;

/*
 * Declare the per-policy statistics structures.
 * See the SPIOC_GET_STATS ioctl in sec_driver.c.
 * Note: The stats for this are not currently maintained.
 */

struct sp_mod_stats	sp_mod_stats[SPOLICY_CNT];

/*
 * Declare the security functions switch.
 */

extern dacowner();
#if SEC_ACL
#if SEC_ACL_SWARE
extern aclchange_obj(), aclchange_subj();
#endif /* SEC_ACL_SWARE */
/*
 *  POSIX ACLS  additions
 */
#if SEC_ACL_POSIX
extern paclchange_obj(), paclchange_subj();
#endif /* SEC_ACL_POSIX */
#else /* !SEC_ACL */
extern udacchange_obj(), udacchange_subj();
#endif /* !SEC_ACL */

struct sec_functions sp_functions = {
#if SEC_ACL
#if SEC_ACL_SWARE
                dacowner, aclchange_subj, aclchange_obj, 0,
#endif /* SEC_ACL_SWARE */
#if SEC_ACL_POSIX
                dacowner, paclchange_subj, paclchange_obj, 0,
#endif /* SEC_ACL_POSIX */
#else /* !SEC_ACL */
		dacowner, udacchange_subj, udacchange_obj, 0,
#endif /* !SEC_ACL */
};

/* Policy specific declarations */

#if SEC_ACL

#if SEC_ACL_SWARE
int aclpolicy = -1, aclobjtag = -1, aclsubjtag = -1;
#endif /* SEC_ACL_SWARE */

#if SEC_ACL_POSIX
int paclpolicy = -1, paclobjtag = -1, paclsubjtag = -1;
#endif /* SEC_ACL_POSIX */

#endif /* SEC_ACL */

#if SEC_MAC
int macpolicy = -1, macobjtag = -1, macsubjtag = -1, macclrnce = -1;
#endif

#endif /* SEC_ARCH */

/*
 * Patching sware_debug to a non-zero value enables  kernel debugging
 * printfs and similar debug code in many portions of the kernel.
 * This is typically done manually with the kernel debugger.
 */

int sware_debug = 0;

#endif /* SEC_BASE */
