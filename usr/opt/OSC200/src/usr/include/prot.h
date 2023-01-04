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
 *	@(#)$RCSfile: prot.h,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/12/15 22:13:49 $
 */ 
/*
 */
#ifndef __PROT__
#define __PROT__

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */


/* Copyright (c) 1988, 1989 SecureWare, Inc.
 *   All rights reserved.
 *
 * Header file for Security Databases
 *

 * Based on:

 */

/*
 * This Module contains Proprietary Information of SecureWare, Inc.
 * and should be treated as Confidential.
 */


#if SEC_MAC
#include <mandatory.h>
#endif
#if SEC_NCAV
#include <ncav.h>
#endif
#if SEC_ACL
#include <acl.h>
#endif

#define	AUTH_CAPBUFSIZ	10240

/* file template used to prove subsystem identity to audit_dlvr */

#define AUTH_SUBSYS_TICKET	"/tcb/files/subsys/XXXXXXXXXXXXXX"

/* Location of program support for Authentication and Subsystem auditing */



/*  Authentication database locking */

/*
 * AUTH_LOCK_ATTEMPTS used to be 8.  This is unacceptably low, especially
 * since libsecurity code will give up in half that many attempts if the
 * file in question hasn't changed.  Something as simple as being swapped
 * out for a few seconds could cause another process to assume the
 * lock will never be freed, take the lock, and cause database corruption.
 * Choosing a higher value reduces the risk of this problem, but certainly
 * doesn't eliminate it.
 */

#define	AUTH_LOCK_ATTEMPTS	100		/* before giving up lock try */
#define	AUTH_RETRY_DELAY	1		/* seconds */
#define	AUTH_CHKENT		"chkent"	/* sanity check when reading */
#define	AUTH_SILENT		0		/* do actions silently */
#define	AUTH_LIMITED		1		/* tell something of action */
#define	AUTH_VERBOSE		2		/* full disclosure of action */


/*  Database system default entry name */

#define	AUTH_DEFAULT		"default"


/*  Support for large passwords and pass-phrases. */

/*
 * The size of the ciphertext is the salt at the
 * beginning, a series of 11-character segments produced by
 * each call to crypt(), and the trailing end-of-string character.
 * Each 11-character segment uses the a newly computed salt based
 * on the previous encrypted value.
 */
#define	AUTH_MAX_PASSWD_LENGTH		80
#define	AUTH_SALT_SIZE			2
#define	AUTH_CLEARTEXT_SEG_CHARS	8
#define	AUTH_CIPHERTEXT_SEG_CHARS	11
#define	AUTH_SEGMENTS(len)		(((len)-1)/AUTH_CLEARTEXT_SEG_CHARS+1)
#define	AUTH_CIPHERTEXT_SIZE(segments)	(AUTH_SALT_SIZE+(segments)*AUTH_CIPHERTEXT_SEG_CHARS+1)
#define	AUTH_MAX_CIPHERTEXT_LENGTH	AUTH_CIPHERTEXT_SIZE(AUTH_SEGMENTS(AUTH_MAX_PASSWD_LENGTH))
#define	AUTH_MAX_PASSWD_DICT_DEPTH	9
#define	AUTH_MAX_PASSWD_DICT_SIZE	(AUTH_MAX_PASSWD_DICT_DEPTH*AUTH_MAX_CIPHERTEXT_LENGTH)
#define	AUTH_MAX_UNAME_SIZE		(8+1)
#ifndef TRUSTED_MASK_LEN
#include <sys/audit.h>
#endif
#define	AUTH_MAX_AUDIT_LENGTH		(4096)	/* 3867 last I checked */
/* Indices for fd_{old,new}crypt algorithms */
#define	AUTH_CRYPT_BIGCRYPT		0	/* index to use bigcrypt */
#define	AUTH_CRYPT_CRYPT16		1	/* index to use crypt16 */
#define	AUTH_CRYPT__MAX			1	/* last legal index */


/*  Time-of-day session login constraints */

#define	AUTH_TOD_SIZE		50	/* length of time-of-day constraints */

#if defined(TPATH)
/*
 *
 *	Length of the Trusted Path Sequence (not including the \0)
 *
 */

#define	AUTH_TRUSTED_PATH_LENGTH	12
#endif

/*
 *
 *	Values returned from create_file_securely()
 *
 */

#define	CFS_GOOD_RETURN			0
#define	CFS_CAN_NOT_OPEN_FILE		1
#define	CFS_NO_FILE_CONTROL_ENTRY	2
#define	CFS_CAN_NOT_CHG_MODE		3
#define	CFS_CAN_NOT_CHG_OWNER_GROUP	4

#ifdef SEC_ACL
#define	CFS_CAN_NOT_CHG_ACL		5
#endif
#ifdef SEC_MAC
#define	CFS_CAN_NOT_CHG_SL		6
#endif
#if SEC_NCAV
#define	CFS_CAN_NOT_CHG_NCAV		7
#endif



/*  Database access parameters */

#define	AUTH_REWIND		0	/* look for entry from file beginning */
#define	AUTH_CURPOS		1	/* look for entry from current pos'n */


/*  File updating extensions -- must use 1 char no user can have */

#define	AUTH_OLD_EXT		":o"	/* previous version of file */
#define	AUTH_TEMP_EXT		":t"	/* going to be new version of file */


/*  Time values can be set to current time on initial installation */

#define AUTH_TIME_NOW		"now"
#define	AUTH_TIME_NOW_VALUE	((time_t)-1L)

/*  Location of Subsystem database.  */

#define	AUTH_SUBSYSDIR		"/etc/auth/subsystems"


/*  for fd_secclass  */

#define	AUTH_CLASSES			"security classes"
#define AUTH_D			0	/* TCSEC Class D */
#define AUTH_C1			1	/* TCSEC Class C1 */
#define AUTH_C2			2	/* TCSEC Class C2 */
#define AUTH_B1			3	/* TCSEC Class B1 */
#define AUTH_B2			4	/* TCSEC Class B2 */
#define AUTH_B3			5	/* TCSEC Class B3 */
#define AUTH_A1			6	/* TCSEC Class A1 */
#define	AUTH_MAX_SECCLASS	6


/*  for fd_cprivs and subsystem audit recording  */

#define	AUTH_CMDPRIV			"command authorizations"
/* The actual command authorizations are specified in a control file */

/*  for fd_sprivs  */

#define	AUTH_SYSPRIV			"kernel authorizations"
/* The actual kernel authorizations are defined in <sys/security.h> . */

#define	AUTH_BASEPRIV			"base privileges"
#if SEC_PRIV
#define	AUTH_PPRIVS			"potential privileges"
#define	AUTH_GPRIVS			"granted privileges"
#endif

/*  for fd_auditmask  */

/* The actual event types are defined in <sys/audit.h>. */

#define	AUTH_TYPEVEC_SIZE	(WORD_OF_BIT(AUTH_MAX_TYPE) + 1)

/*
 * NOTE: the system must be re-compiled if more than 32 command
 *	 authorizations are defined.
 */
#define	AUTH_CPRIVVEC_SIZE	1
#define	AUTH_SECCLASSVEC_SIZE	(WORD_OF_BIT(AUTH_MAX_SECCLASS) + 1)
#define	AUTH_SEEDVEC_SIZE	(WORD_OF_BIT(AUTH_MAX_SEED) + 1)

/*
 * The number of command authorizations is defined at runtime according
 * to the Command Authorization Definition file.  This variable is
 * initialized by build_cmd_priv(), automatically invoked from Protected
 * Password and command authorization checking routines.
 */

extern int ST_MAX_CPRIV;

/*  Protected password database entry  */

struct pr_field  {
	/* Identity: */
#define	AUTH_U_NAME		"u_name"
	char	fd_name[AUTH_MAX_UNAME_SIZE];	/* uses 8 character maximum(and \0) from utmp */
#define	AUTH_U_ID		"u_id"
	uid_t	fd_uid;	 	/* uid associated with name above */
#define	AUTH_U_PWD		"u_pwd"
	char	fd_encrypt[AUTH_MAX_CIPHERTEXT_LENGTH];	/* Encrypted password */
#define	AUTH_U_OWNER		"u_owner"
	char	fd_owner[AUTH_MAX_UNAME_SIZE];	/* if a pseudo -user, the user behind it */
#define	AUTH_U_PRIORITY		"u_priority"
	int	fd_nice;	/* nice value with which to login */
#define	AUTH_U_CMDPRIV		"u_cmdpriv"
	mask_t	fd_cprivs[AUTH_CPRIVVEC_SIZE];	/* command authorization vector */
#define	AUTH_U_SYSPRIV		"u_syspriv"
	privvec_t fd_sprivs;	/* kernel authorizations vector */
#define	AUTH_U_BASEPRIV		"u_basepriv"
	privvec_t fd_bprivs;	/* base privilege vector */
#define	AUTH_U_AUDITDISP	"u_auditmask"
	char	fd_auditdisp[AUTH_MAX_AUDIT_LENGTH];	/* auditmask text */
#define	AUTH_U_AUDITCNTL	"u_audcntl"
	uchar_t	fd_auditcntl;	/* audit mask use control */

	/* Password maintenance parameters: */
#define	AUTH_U_MINCHG		"u_minchg"
	time_t	fd_min;		/* minimum time between password changes */
#define	AUTH_U_MINLEN		"u_minlen"
	int	fd_minlen;	/* minimum length of password */
#define	AUTH_U_MAXLEN		"u_maxlen"
	int	fd_maxlen;	/* maximum length of password */
#define	AUTH_U_EXP		"u_exp"
	time_t	fd_expire;	/* soft expiration interval (seconds) */
#define	AUTH_U_LIFE		"u_life"
	time_t	fd_lifetime;	/* hard expiration interval (seconds) */
#define	AUTH_U_SUCCHG		"u_succhg"
	time_t	fd_schange;	/* last successful change in secs past 1/1/70 */
#define	AUTH_U_UNSUCCHG		"u_unsucchg"
	time_t	fd_uchange;	/* last unsuccessful change */
#define	AUTH_U_PICKPWD		"u_pickpw"
	char	fd_pick_pwd;	/* can user pick his own passwords? */
#define	AUTH_U_GENPWD		"u_genpwd"
	char	fd_gen_pwd;	/* can user get passwords generated for him? */
#define	AUTH_U_RESTRICT		"u_restrict"
	char	fd_restrict;	/* should generated passwords be restricted? */
#define	AUTH_U_POLICY		"u_policy"
	char	fd_policy;	/* check passwords by policy callout ? */
#define	AUTH_U_NULLPW		"u_nullpw"
	char	fd_nullpw;	/* is user allowed to have a null password? */
#define	AUTH_U_PWCHANGER	"u_pwchanger"
	uid_t	fd_pwchanger;	/* who last changed this user's password */
#ifdef TMAC
#define	AUTH_U_PW_ADMIN_NUM	"u_pw_admin_num"
	long	fd_pw_admin_num;/* password generation verifier */
#endif
#define	AUTH_U_GENCHARS		"u_genchars"
	char	fd_gen_chars;	/* can have password of random ASCII? */
#define	AUTH_U_GENLETTERS	"u_genletters"
	char	fd_gen_letters;	/* can have password of random letters? */
#define	AUTH_U_PWDEPTH		"u_pwdepth"
	char	fd_pwdepth;	/* depth of password dictionary to keep */
#define	AUTH_U_PWDICT		"u_pwdict"
	char	fd_pwdict[AUTH_MAX_PASSWD_DICT_SIZE];	/* password history dictionary */
#define	AUTH_U_OLDCRYPT		"u_oldcrypt"
	uchar_t	fd_oldcrypt;	/* algorithm index for current crypt function */
#define	AUTH_U_NEWCRYPT		"u_newcrypt"
	uchar_t	fd_newcrypt;	/* algorithm index for next crypt function */

#if SEC_MAC
	/* Mandatory policy parameters: */
#define	AUTH_U_CLEARANCE	"u_clearance"
	mand_ir_t fd_clearance;	/* internal representation of clearance */
	char fd_clearance_filler[200]; /* MUST follow fd_clearance */
#endif

	/* Login parameters: */
#define	AUTH_U_SUCLOG		"u_suclog"
	time_t	fd_slogin;	/* last successful login */
#define	AUTH_U_UNSUCLOG		"u_unsuclog"
	time_t	fd_ulogin;	/* last unsuccessful login */
#define AUTH_U_SUCTTY		"u_suctty"
	char	fd_suctty[14];	/* tty of last successful login */
#define	AUTH_U_NUMUNSUCLOG	"u_numunsuclog"
	short	fd_nlogins;	/* consecutive unsuccessful logins */
#define AUTH_U_UNSUCTTY		"u_unsuctty"
	char	fd_unsuctty[14];/* tty of last unsuccessful login */
#define	AUTH_U_TOD		"u_tod"
	char	fd_tod[AUTH_TOD_SIZE];		/* times when user may login */
#define	AUTH_U_MAXTRIES		"u_maxtries"
	ushort	fd_max_tries;	/* maximum unsuc login tries allowed */
#define	AUTH_U_UNLOCK		"u_unlock"
	time_t	fd_unlockint;	/* interval (seconds) before unlocking again */
#define	AUTH_U_RETIRED		"u_retired"
	char	fd_retired;	/* Is account retired? */
#define	AUTH_U_LOCK		"u_lock"
	char	fd_lock;	/* Unconditionally lock account? */
#define	AUTH_U_EXPDATE		"u_expdate"
	time_t	fd_expdate;	/* time at which to auto-retire the account */

#if SEC_NCAV

#define	AUTH_U_NATCITIZEN	"u_natcitizen"
	ncav_ir_t	*fd_nat_citizen;
#define	AUTH_U_NATCAVEATS	"u_natcaveats"
	ncav_ir_t	*fd_nat_caveats;
#endif

#define	AUTH_U_ISTEMPLATE	"u_istemplate"
	char	fd_istemplate;	/* this account is a template only */
#define	AUTH_U_TEMPLATE		"u_template"
	char	fd_template[AUTH_MAX_UNAME_SIZE];	/* name of (template) account for defaults */
};


struct pr_flag  {
	unsigned int
		/* Identity: */
		fg_name:1,		/* Is fd_name set? */
		fg_uid:1,		/* Is fd_uid set? */
		fg_encrypt:1,		/* Is fd_encrypt set? */
		fg_owner:1,		/* Is fd_owner set? */
		fg_nice:1,		/* Is fd_nice set? */
		fg_cprivs:1,		/* Is fd_sprivs set? */
		fg_sprivs:1,		/* Is fd_sprivs set? */
		fg_bprivs:1,		/* Is fd_bprivs set? */
		fg_auditcntl:1,		/* Is fd_auditcntl set? */
		fg_auditdisp:1,		/* Is fd_auditdisp set? */

		/* Password maintenance parameters: */
		fg_min:1,		/* Is fd_min set? */
		fg_minlen:1,		/* Is fd_minlen set? */
		fg_maxlen:1,		/* Is fd_maxlen set? */
		fg_expire:1,		/* Is fd_expire set? */
		fg_lifetime:1,		/* Is fd_lifetime set? */
		fg_schange:1,		/* Is fd_schange set? */
		fg_uchange:1,		/* Is fd_fchange set? */
		fg_pick_pwd:1,		/* Is fd_pick_pwd set? */
		fg_gen_pwd:1,		/* Is fd_gen_pwd set? */
		fg_restrict:1,		/* Is fd_restrict set? */
		fg_policy:1,		/* Is fd_policy set? */
		fg_nullpw:1,		/* Is fd_nullpw set? */
		fg_pwchanger:1,		/* Is fd_pwchanger set? */
		fg_pwdepth:1,		/* Is fd_pwdepth set? */
		fg_pwdict:1,		/* Is fd_pwdict set? */
#ifdef TMAC
		fg_pw_admin_num:1,	/* Is fd_pw_admin_num set? */
#endif
		fg_gen_chars:1,		/* Is fd_gen_chars set? */
		fg_gen_letters:1,	/* Is fd_gen_letters set? */
		fg_oldcrypt:1,		/* Is fd_oldcrypt set? */
		fg_newcrypt:1,		/* Is fd_newcrypt set? */

#if SEC_MAC
		/* Mandatory policy parameters: */
		fg_clearance:1,		/* Is fd_clearance set? */
#endif

		/* Login parameters: */
		fg_slogin:1,		/* Is fd_slogin set? */
		fg_suctty: 1,		/* is fd_suctty set ? */
		fg_unsuctty: 1,		/* is fd_unsuctty set ? */
		fg_ulogin:1,		/* Is fd_ulogin set? */
		fg_nlogins:1,		/* Is fd_nlogins set? */
		fg_max_tries:1,		/* Is fd_max_tries set? */
		fg_retired:1,		/* Is fd_retired set? */
		fg_lock:1,		/* Is fd_lock set? */
		fg_unlockint:1,		/* Is fd_unlockint set? */
		fg_tod:1,		/* Is fd_tod set? */
		fg_expdate:1,		/* Is fd_expdate set? */
#if SEC_NCAV
		fg_nat_citizen : 1,  	/* fd_nat_citizen? */
		fg_nat_caveats : 1,	/* fd_nat_caveats? */
#endif
		fg_istemplate:1,	/* Is fd_istemplate set? */
		fg_template:1		/* Is fd_template set? */
		;
};

struct pr_passwd  {
	struct pr_field ufld;	/* Fields assoc specifically with this user */
	struct pr_flag uflg;	/* Flags assoc specifically with this user */
	struct pr_field sfld;	/* Fields assoc with system */
	struct pr_flag sflg;	/* Flags assoc with system */
};

/*  Terminal Control Database Entry  */

struct	t_field  {
#define	AUTH_T_DEVNAME		"t_devname"
	char	fd_devname[14];	/* Device/host name */
#define	AUTH_T_UID		"t_uid"
	uid_t	fd_uid;		/* uid of last successful login */
#define	AUTH_T_LOGTIME		"t_logtime"
	time_t	fd_slogin;	/* time stamp of   "        "   */
#define	AUTH_T_UNSUCUID		"t_unsucuid"
	uid_t	fd_uuid;	/* uid of last unsuccessful login */
#define	AUTH_T_UNSUCTIME	"t_unsuctime"
	time_t	fd_ulogin;	/* time stamp of  "           "   */
#define	AUTH_T_PREVUID		"t_prevuid"
	uid_t	fd_loutuid;	/* uid of last logout */
#define	AUTH_T_PREVTIME		"t_prevtime"
	time_t	fd_louttime;	/* time stamp of   "    */
#define	AUTH_T_FAILURES		"t_failures"
	ushort	fd_nlogins;	/* consecutive failed attempts */
#define	AUTH_T_MAXTRIES		"t_maxtries"
	ushort	fd_max_tries;	/* maximum unsuc login tries allowed */
#define	AUTH_T_LOGDELAY		"t_logdelay"
	time_t	fd_logdelay;	/* delay between login tries */
#define	AUTH_T_UNLOCK		"t_unlock"
	time_t	fd_unlockint;	/* delay before clearing t_failures */
#define	AUTH_T_LOCK		"t_lock"
	char	fd_lock;	/* terminal locked? */
#define AUTH_T_XDISPLAY		"t_xdisplay"
	char	fd_xdisp;	/* this entry is for an X display (xdm) */
#define AUTH_T_LOGIN_TIMEOUT	"t_login_timeout"
	ushort	fd_login_timeout ;	/* login timeout in seconds */
};


struct	t_flag  {
	unsigned int
		fg_devname:1,		/* Is fd_devname set? */
		fg_uid:1,		/* Is fd_uid set? */
		fg_slogin:1,		/* Is fd_stime set? */
		fg_uuid:1,		/* Is fd_uuid set? */
		fg_ulogin:1,		/* Is fd_ftime set? */
		fg_loutuid:1,		/* Is fd_loutuid set? */
		fg_louttime:1,		/* Is fd_louttime set? */
		fg_nlogins:1,		/* Is fd_nlogins set? */
		fg_max_tries:1,		/* Is fd_max_tries set? */
		fg_logdelay:1,		/* Is fd_logdelay set? */
		fg_lock:1,		/* Is fd_lock set? */
		fg_unlockint:1,		/* Is fd_unlockint set? */
		fg_login_timeout : 1,	/* is fd_login_timeout valid? */
		fg_xdisp : 1		/* Is fd_xdisp valid ? */
		;
};


struct	pr_term  {
	struct t_field ufld;
	struct t_flag uflg;
	struct t_field sfld;
	struct t_flag sflg;
};


/*  File Control Database Entry  */

struct	f_field  {
	char	*fd_name;	/* Holds full path name */
#define	AUTH_F_OWNER		"f_owner"
	uid_t	fd_uid;		/* uid of owner */
#define	AUTH_F_GROUP		"f_group"
	gid_t	fd_gid;		/* gid of group */
#define	AUTH_F_MODE		"f_mode"
	mode_t	fd_mode;	/* permissions */
#define	AUTH_F_TYPE		"f_type"
	char 	fd_type[2];	/* file type (one of r,b,c,d,f,s,m) */
#if SEC_MAC
#define	AUTH_F_SLEVEL		"f_slevel"
#define	AUTH_F_SYSLO		"syslo"
#define	AUTH_F_SYSHI		"syshi"
#define	AUTH_F_WILD		"WILDCARD"
	mand_ir_t *fd_slevel;	/* sensitivity level for file */
#endif
#if SEC_ACL
#define	AUTH_F_ACL		"f_acl"
#ifndef AUTH_F_WILD
#define AUTH_F_WILD		"WILDCARD"	/* def. shared with MAC */
#endif
	acle_t	*fd_acl;	/* access control list for file */
	int	fd_acllen;	/* number of entries in fd_acl */
#endif
#if SEC_NCAV
#define AUTH_F_NCAV		"f_ncav"
#define AUTH_F_ALL		"all"
	ncav_ir_t *fd_ncav;	/* nationality caveat set */
#endif
#if SEC_PRIV
#define	AUTH_F_PPRIVS		"f_pprivs"
	privvec_t fd_pprivs;	/* potential privileges */
#define	AUTH_F_GPRIVS		"f_gprivs"
	privvec_t fd_gprivs;	/* granted privileges */
#endif
};

struct	f_flag  {
	unsigned int
		fg_name:1,	/* Is fd_name set? */
		fg_uid:1,	/* Is fd_uid set? */
		fg_gid:1,	/* Is fd_gid set? */
		fg_mode:1,	/* Is fd_mode set? */
		fg_type:1	/* Is fd_type set? */
#if SEC_MAC
	      , fg_slevel:1	/* Is fd_slevel set? */
#endif
#if SEC_ACL
	      , fg_acl:1	/* Is fd_acl set? */
#endif
#if SEC_NCAV
	      , fg_ncav:1	/* Is fd_ncav set? */
#endif
#if SEC_PRIV
	      , fg_pprivs:1,	/* Is fd_pprivs set? */
		fg_gprivs:1	/* Is fd_gprivs set? */
#endif
		;
};

struct	pr_file  {
	struct f_field ufld;
	struct f_flag uflg;
};


#if SEC_MAC || SEC_ILB

/*  Printer Control Database Entry  */

struct	l_field  {
#define	AUTH_L_LPNAME		"l_name"
	char	fd_name[15];	/* holds printer name */
#define	AUTH_L_INITSEQ		"l_initseq"
	char	fd_initseq[256];/* initial sequence */
#define	AUTH_L_TERMSEQ		"l_termseq"
	char	fd_termseq[256];/* termination sequence */
#define	AUTH_L_EMPH		"l_emph"
	char	fd_emph[256];	/* emphasize sequence */
#define	AUTH_L_DEEMPH		"l_deemph"
	char	fd_deemph[256];	/* de-emphasize sequence */
#define	AUTH_L_CHRS		"l_chrs"
	char	fd_chrs[130];	/* characters to filter */
#define	AUTH_L_CHRSLEN		"l_chrslen"
	ushort	fd_chrslen;	/* length of string of illegal chars */
#define	AUTH_L_ESCS		"l_escs"
	char	fd_escs[256];	/* escape sequences */
#define	AUTH_L_ESCSLEN		"l_esclens"
	ushort	fd_escslen;	/* length of string of illegal escape codes */
#define	AUTH_L_LINELEN		"l_linelen"
	int	fd_linelen;	/* length of a line in characters */
#define	AUTH_L_PAGELEN		"l_pagelen"
	int	fd_pagelen;	/* length of a page in lines */
#define	AUTH_L_TRUNCLINE	"l_truncline"
	char	fd_truncline;	/* does printer truncate long lines? */
};

struct	l_flag  {
	unsigned int
		fg_name:1,	/* Is fd_name set? */
		fg_initseq:1,	/* Is fd_initseq set? */
		fg_termseq:1,	/* Is fd_termseq set? */
		fg_emph:1,	/* Is fd_emph set? */
		fg_deemph:1,	/* Is fd_deemph set? */
		fg_chrs:1,	/* Is fd_chrs set? */
		fg_chrslen:1,	/* Is fd_chrslen set? */
		fg_escs:1,	/* Is fd_escs set? */
		fg_escslen:1,	/* Is fd_escslen set? */
		fg_linelen:1,	/* Is fd_linelen set? */
		fg_pagelen:1,	/* Is fd_pagelen set? */
		fg_truncline:1	/* Is fd_truncline set? */
		;
};

struct	pr_lp  {
	struct l_field ufld;
	struct l_flag uflg;
	struct l_field sfld;
	struct l_flag sflg;
};

#endif /* SEC_MAC */

/* Device Assignment Database entry */

#define AUTH_DEV_TYPE "device type"
#define AUTH_DEV_PRINTER	0
#define AUTH_DEV_TERMINAL	1
#define AUTH_DEV_TAPE		2
#define AUTH_DEV_REMOTE		3
#define AUTH_DEV_XDISPLAY	4
#define AUTH_MAX_DEV_TYPE	4

#define AUTH_DEV_TYPE_SIZE	(WORD_OF_BIT (AUTH_MAX_DEV_TYPE) + 1)

#if SEC_ARCH

#define AUTH_DEV_ASSIGN	"device assignment"
#if SEC_MAC
#define AUTH_DEV_SINGLE  	0	/* single-level sens. labels. */
#define AUTH_DEV_MULTI   	1	/* multilevel sens. labels. */
#endif
#define AUTH_DEV_LABEL   	2	/* labeled import/export enabled */
#define AUTH_DEV_NOLABEL 	3	/* unlabeled import/export enabled */
#define AUTH_DEV_IMPORT  	4	/* enabled for import */
#define AUTH_DEV_EXPORT  	5	/* enabled for export */
#define AUTH_DEV_PASS	 	6	/* internal to mltape */
#if SEC_ILB
#define AUTH_DEV_ILSINGLE  	7	/* single-level info. labels. */
#define AUTH_DEV_ILMULTI   	8	/* multilevel info. labels. */
#endif
#if SEC_NCAV
#define AUTH_DEV_NCAVSINGLE  	9	/* single-level nat. caveats */
#define AUTH_DEV_NCAVMULTI   	10	/* multilevel  nat. caveats*/
#endif
#define AUTH_MAX_DEV_ASSIGN 	10
#define AUTH_DEV_ASSIGN_SIZE	(WORD_OF_BIT (AUTH_MAX_DEV_ASSIGN) + 1)

#endif /* SEC_ARCH */

struct dev_field {
	char	*fd_name;	/* external name */
#define AUTH_V_DEVICES	"v_devs"
	char	**fd_devs;	/* device list */
#define AUTH_V_TYPE	"v_type"
	mask_t	fd_type[AUTH_DEV_TYPE_SIZE];	/* tape, printer, terminal */
#if SEC_MAC
#define AUTH_V_MAX_SL	"v_maxsl"
#define AUTH_V_MIN_SL	"v_minsl"
#define AUTH_V_CUR_SL	"v_cursl"
	mand_ir_t	*fd_max_sl;	/* maximum sensitivity level */
	mand_ir_t	*fd_min_sl ;	/* minimum sensitivity level */
	mand_ir_t	*fd_cur_sl ;	/* currently assigned s.l. */
#endif
#if SEC_ILB
#define AUTH_V_CUR_IL	"v_curil"
	ilb_ir_t	*fd_cur_il ;	/* currently assigned info l. */
#endif
#if SEC_ARCH
#define AUTH_V_ASSIGN	"v_assign"
	mask_t	fd_assign[AUTH_DEV_ASSIGN_SIZE];/* single-, multilevel, etc. */
#endif

#if SEC_NCAV

#define	AUTH_V_MAX_NAT_CAVEATS	"v_max_nat_caveats"
#define	AUTH_V_MIN_NAT_CAVEATS	"v_min_nat_caveats"
#define	AUTH_V_CUR_NAT_CAVEATS	"v_cur_nat_caveats"

	ncav_ir_t	*fd_max_nat_caveats ;
	ncav_ir_t	*fd_min_nat_caveats ;
	ncav_ir_t	*fd_cur_nat_caveats ;
#endif

#define	AUTH_V_USERS	"v_users"
	char	**fd_users ; 	/* list of users */
};

struct dev_flag {
	unsigned int
			fg_name : 1,
			fg_devs : 1,
			fg_type : 1,
#if SEC_MAC
			fg_max_sl : 1,
			fg_min_sl : 1,
			fg_cur_sl : 1,
#endif
#if SEC_ILB
			fg_cur_il : 1,
#endif
			fg_assign : 1,
#if SEC_NCAV
			fg_max_nat_caveats : 1,	/* fd_max_nat_caveats? */
			fg_min_nat_caveats : 1,	/* fd_min_nat_caveats? */
			fg_cur_nat_caveats : 1,	/* fd_cur_nat_caveats? */
#endif
			fg_users  : 1;
};

struct dev_asg {
	struct dev_field ufld;
	struct dev_flag  uflg;
	struct dev_field sfld;
	struct dev_flag  sflg;
};


/*
 *
 *	Structure definitions for the System Default global values.
 *
 */

#define	AUTH_D_INACTIVITY_TIMEOUT	"d_inactivity_timeout"
#define	AUTH_D_PW_EXPIRE_WARNING	"d_pw_expire_warning"
#define AUTH_D_PW_SITE_CALLOUT		"d_pw_site_callout"
#define	AUTH_D_BOOT_AUTHENTICATE	"d_boot_authenticate"
#define	AUTH_D_SECCLASS			"d_secclass"
#define AUTH_D_SINGLE_USER_SL		"d_single_user_sl"

#ifdef TPATH	/*{*/
#define	AUTH_D_LOGIN_SESSION_TIMEOUT	"d_login_session_timeout"
#define	AUTH_D_LOGIN_SESSION_WARNING	"d_login_session_warning"
#if SEC_CHOTS
#define AUTH_D_MULTIPLE_LOGIN		"d_multiple_login_rule"
#else
#define AUTH_D_TRUSTED_PATH_SEQ		"d_trusted_path_seq"
#endif

#if SEC_CHOTS
/*
 *
 *	values associated with AUTH_D_MULTIPLE_LOGIN
 *
 */

#define	D_ALLOW_MULTIPLE_LOGINS		0
#define	D_PREVENT_SECOND_LOGIN		1
#define D_PREVENT_SECOND_LOGOUT_FIRST	2

#define D_MIN_MULTIPLE_LOGIN_VALUE	D_ALLOW_MULTIPLE_LOGINS
#define	D_MAX_MULTIPLE_LOGIN_VALUE	D_PREVENT_SECOND_LOGOUT_FIRST
#endif
#endif /*} TPATH */


struct	system_default_fields
  {
    time_t	fd_inactivity_timeout;
    time_t	fd_pw_expire_warning;
    char	fd_pw_site_callout[MAXPATHLEN];

#if SEC_MAC
    mand_ir_t	*fd_single_user_sl;
#endif

    mask_t	fd_secclass[AUTH_SECCLASSVEC_SIZE];/* System security class */
    char	fd_boot_authenticate;
    char	fd_audit_enable;

#if defined(TPATH)
    ushort	fd_session_timeout;
    ushort	fd_session_warning;
#if SEC_CHOTS
    ushort	fd_multiple_login_rule;
#else
    char	fd_trusted_path_seq[AUTH_TRUSTED_PATH_LENGTH+1];
#endif
#endif
  } ;

struct	system_default_flags
  {
    unsigned int
		fg_inactivity_timeout  : 1,
		fg_pw_expire_warning   : 1,
		fg_pw_site_callout     : 1,
#if SEC_MAC
		fg_single_user_sl      : 1,	/* sens. level for single-user*/
#endif
		fg_boot_authenticate   : 1,
		fg_audit_enable        : 1,
#ifdef TPATH
		fg_session_timeout     : 1,	/* set if fd_session valid */
		fg_session_warning     : 1,	/* set if fd_session valid */
#if SEC_CHOTS
		fg_single_user_sl      : 1,	/* sens. level for single-user*/
   	 	fg_multiple_login_rule : 1,	/* set if fd_multiple valid */
#else
		fg_trusted_path_seq    : 1,	/* set if fd_trusted valid */
#endif
#endif
		fg_secclass            : 1 ;
  } ;

struct	pr_default  {
#define	AUTH_D_NAME			"d_name"
	char				dd_name[20] ;
	char				dg_name ;
	struct pr_field			prd ;
	struct pr_flag			prg ;
	struct t_field			tcd ;
	struct t_flag			tcg ;
	struct dev_field		devd ;
	struct dev_flag			devg ;
	struct system_default_fields	sfld ;
	struct system_default_flags	sflg ;
} ;


struct namepair  {
	char *name;
	ulong value;
};


extern char *command_name;
extern char *command_line;

extern struct namepair user_type[];
extern struct namepair *cmd_priv;
extern struct namepair sys_priv[];
extern struct namepair secclass[];
extern struct namepair audit_mask[];
extern struct namepair auth_dev_assign[];
extern struct namepair auth_dev_type[];

/*
 * Functions to be performed by the site password policy callout.
 * This has been made extensible in this fashion in case we need
 * to add additional callouts in the future.
 *
 * The calling program will pass the function code as an ASCII
 * representation of a decimal integer into the callout executable on
 * its standard input, followed by a newline (in other words, on a line
 * by itself).  Any parameters which the function requires will then be
 * passed on separate lines.  The called executable will then exit with
 * status 0 if the input was acceptable, or non-0 if it was not.  Unless
 * a function needs to receive some other sort of response from the callout,
 * it should produce no output on stdout or stderr.  It is free to make
 * calls to syslog or to write audit records, however.
 *
 * Here are the functions, and a brief description of their parameters:
 */

typedef enum AUTH_PW_funcs {
	AUTH_pw_init,	/* must be first */

	AUTH_pw_okpassword,

	AUTH_pw_limit	/* must be last */
} AUTH_PW_FUNC_T;

#define	AUTH_PW_FUNC_MIN	((int) AUTH_pw_init+1)
#define	AUTH_PW_OKPASSWORD	((int) AUTH_pw_okpassword)
/*
 * First parm is username for which we're trying to change the password.
 * Second is the proposed new password.
 * Exit status is the only expected response.
 */
#define	AUTH_PW_FUNC_MAX	((int) AUTH_pw_limit-1)

#ifdef __cplusplus
extern "C" {
#endif

/* Functions from authaudit.c */
extern void	audit_lax_file(), audit_security_failure(),
		sa_audit_security_failure(), audit_no_resource(),
		sa_audit_no_resource(), audit_auth_entry(), audit_subsystem(),
		sa_audit_subsystem(), audit_login(), audit_passwd(),
		audit_lock(), audit_adjust_mask(), sa_audit_lock(),
		sa_audit_audit();

/* Functions from authcap.c */
extern void	asetdefaults(), open_auth_file(), end_authcap(), endprdfent(),
		setprdfent();
extern char	*agetfile(), *agetdvag(), *agetstr(), **agetstrlist(),
		*find_auth_file(), *agetdefault_buf(), **agetnmlist(),
		*adecodestr(), *agetnextfield(), *agetnextident();
extern long	adecodenum();

/* Functions from discr.c */
extern void	setuid_least_privilege();

#if SEC_ARCH
/* Functions from disk.c */
extern void	disk_set_file_system(), disk_inode_incr();
#ifndef BSD
extern daddr_t	disk_itod(), disk_itoo();
#else
extern void disk_inode_in_block();
#endif
#endif /* SEC_ARCH */

/* Functions from fields.c */
extern void	loadnamepair();
extern char	*storenamepair(), *storebool();

/* Fucntions from getprpwent.c */
extern struct pr_passwd
		*getprpwent(), *getprpwuid(), *getprpwnam();
extern void	setprpwent(), endprpwent();

/* Functions from getprtcent.c */
extern struct pr_term
		*getprtcent(), *getprtcnam();
extern void	setprtcent(), endprtcent(), read_tc_fields();

/* Functions from getprfient.c */
extern struct pr_file
		*getprfient(), *getprfinam();
extern void	setprfient(), endprfient();

/* Functions from getprdfent.c */
extern struct pr_default
		*getprdfent(), *getprdfnam();

/* Functions from getdvagent.c */
extern struct dev_asg
		*getdvagent(), *getdvagnam(), *copydvagent();
extern void	setdvagent(), enddvagent();

#if SEC_MAC || SEC_ILB
/* Functions from getprlpent.c */
extern struct pr_lp
		*getprlpent(), *getprlpnam();
extern void	setprlpent(), endprlpent();
#endif

/* Functions from identity.c */
extern void	set_auth_parameters(), check_auth_parameters(),
		enter_quiet_zone(), exit_quiet_zone();
extern uid_t	starting_luid(), starting_ruid(), starting_euid();
extern gid_t	starting_rgid(), starting_egid();

/* Functions from getpasswd.c */
extern char	*getpasswd(), *bigcrypt(), *dispcrypt();

/* Functions from seed.c */
extern void	set_seed();
extern long	get_seed();

/* Functions from map_ids.c */
extern char	*pw_idtoname(), *gr_idtoname();

/* Functions from printbuf.c */
extern void	printbuf();

#if SEC_PRIV
/* Functions from privileges.c */
extern void	initprivs();
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PROT__ */
