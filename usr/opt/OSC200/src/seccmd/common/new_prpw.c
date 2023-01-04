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
static char	*sccsid = "@(#)$RCSfile: new_prpw.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:26 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* create a new protected password entry for the user.  Use the
 * system defaults in most fields.
 */

static char *dflt_subsys_table[] = {
	"default"
};

static void
new_prpw (username,userid, pr)
char *username;
int userid;
struct pr_passwd *pr;
{
	/* clear out the structure to all zeros (defaults) */
	strncpy (pr, "", sizeof (*pr));

#ifdef TMAC
	gen_admin_num(pr);
#endif
	(void) strcpy (pr->ufld.fd_name, username);
	pr->uflg.fg_name = 1;
	pr->ufld.fd_uid = userid;
	pr->uflg.fg_uid = 1;
	ADDBIT (pr->ufld.fd_type, AUTH_GENERAL_TYPE);
	pr->uflg.fg_type = 1;
	pr->uflg.fg_lock = 1;
	pr->ufld.fd_lock = 0;

	/* assume that the user can change his own password. */
	pr->ufld.fd_pswduser = userid;
	pr->uflg.fg_pswduser = 1;
#if defined(B1)
	mand_init() ;
	pr-> uflg.fg_clearance = 1 ;
#ifdef SHW
	mand_copy_ir(mand_syshi, &pr-> ufld.fd_clearance) ;
#else
	mand_copy_ir(mand_syslo, &pr-> ufld.fd_clearance) ;
#endif
#if defined(NCAV)
	pr-> ufld.fd_nat_caveats = (ncav_ir_t *) Malloc(sizeof(ncav_ir_t)) ;
	if (pr-> ufld.fd_nat_caveats != (ncav_ir_t *) 0) {
		ncav_init() ;
		pr-> uflg.fg_nat_caveats = 1 ;
		memcpy(pr-> ufld.fd_nat_caveats, ncav_max_ir, sizeof(ncav_ir_t)) ;
	}
#endif
#endif

	if (putprpwnam (username, pr) != 1)
		audit_subsystem (username,
		  "successful update of protected password entry", 
		  ET_SYS_ADMIN);

	/* Add user to the default subsystem authorizations file */
	write_authorizations (pr->ufld.fd_name, dflt_subsys_table, 1);

	return;
}
