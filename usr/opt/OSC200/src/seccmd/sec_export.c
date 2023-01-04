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
static char *rcsid = "@(#)$RCSfile: sec_export.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/04/08 19:21:29 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * This Module contains Proprietary Information of SecureWare, Inc. and
 * should be treated as Confidential.
 */
/*
 * HISTORY
 * $OSF_Log:	sec_export.c,v $
 * Revision 1.1.1.5  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.6.6.2  1992/07/26  01:52:07  hosking
 * 	bug 6899: fix core dump due to uninitialized gpriv/ppriv
 * 	string length variables
 * 	[1992/07/26  01:51:20  hosking]
 *
 * Revision 1.6.2.5  1992/04/28  17:50:33  valin
 * 	Fix ifdefing of Secureware acls to only compile for Secure Acls and not POSIX.
 * 	[1992/04/28  17:28:32  valin]
 * 
 * Revision 1.6.2.4  1992/04/08  20:29:10  marquard
 * 	Changes from merging.
 * 	[1992/04/05  13:35:08  marquard]
 * 
 * Revision 1.6.2.3  1992/03/06  21:53:57  coren
 * 	Added check for null pointer in ie_ml_import (bug 4190).
 * 	[1992/03/06  21:52:46  coren]
 * 	Revision 1.6.3.2  1992/04/05  13:33:05  marquard
 * 	Changes to enable POSIX ACLs.
 * 
 * Revision 1.6.2.2  1992/01/21  19:06:17  damon
 * 	Cleaned up log
 * 	[1992/01/21  18:53:33  damon]
 * 
 * Revision 1.6  1991/07/18  10:38:49  devrcs
 * 	Fix the catalog.
 * 	[91/07/05  04:36:17  aster]
 * 
 * Revision 1.5  91/04/09  11:54:53  devrcs
 * 	<<<replace with log message for ./seccmd/sec_export.c>>>
 * 	[91/03/24  20:10:39  devrcs]
 * 
 * Revision 1.4  91/03/23  17:21:24  devrcs
 * 	Fixed following bugs -
 * 	 1) There wasn't any option to perform the captive find.  Added '-F' to do
 * 	    that.  '-M' used by other SecureWare platforms is used for something else
 * 	    on OSF.
 * 	 2) Privileges were not getting archived.
 * 	 3) Mlds that dominated the device max sl, were getting transffered without any
 * 	    errors.
 * 	 4) Could not import trees, where file SL dominated its parent's SL.
 * 	 5) Was creating intermediate directories, which did not fall in the device SL
 * 	    range.
 * 	 6) Exporting a tree with an mld, without multileveldir kernel authorization,
 * 	    did not work.
 * 
 * 	Merge changes up from 1.0.1
 * 	[91/03/12  12:20:22  seiden]
 * 
 * 	Fixes to ensure we're exporting from a tagged fs.
 * 	[91/02/21  16:57:12  seiden]
 * 
 * Revision 1.3  90/10/07  15:41:32  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  13:03:26  gm]
 * 
 * Revision 1.2  90/09/13  11:59:16  devrcs
 * 	Initial version from SecureWare.
 * 	[90/08/21  14:13:25  seiden]
 * 
 * 	Initial version from SecureWare.
 * 	[90/08/21  10:30:33  seiden]
 * 
 * $OSF_EndLog$
 */
/* @(#)sec_export.c	6.7 15:07:44 3/15/91 SecureWare, Inc. */
/*
 * Based on (DEC PORT VERSION):
 *    "@(#)export_sec.c	3.1 07:03:33 4/6/90 SecureWare, Inc."
 */

/*
 * Copyright (c) 1988-1990 SecureWare, Inc.
 * All Rights Reserved.
 *
 *
 * sec_export.c 
 *
 * This file contains the routines used for import/export.  The hooks calling
 * these routines are in mltape, cpio and tar.
 *
 * Import/export is complicated by the number of policies that may be existing
 * on the source and destination systems.  Presently sensitivity labels, 
 * information labels and caveat sets can all be either single or multi-level.
 * All combinations of the three are valid.
 *
 * The cpio and tar functions are strictly single-level.  They do not carry
 * any security labels on their media and the commands will not work unless
 * all policies for the specified device are defined to be single-level in the
 * device assignment database.
 *
 * Mltape is multi-level.  Any policy for a device may be configured either 
 * single-level or multi-level.
 *
 */


/*********************
 *  INCLUDES
 *********************/
#include <sys/secdefines.h>
#if SEC_FSCHANGE /*{*/
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#if SEC_ARCH
#include <sys/secpolicy.h>
#endif
#if SEC_ACL
#include <acl.h>
#endif
#if SEC_MAC
#include <mandatory.h>
#endif
#include <prot.h>
#include <pwd.h>

#if SEC_NCAV
#include <ncav.h>
#endif

#include "sec_export.h"

/************************
 *  FORWARD DECLARATIONS. 
 *************************/
static char *m_alloc();
static void do_opts();
static int ie_recovery();
static char *get_er();
static int  set_er();
static int  copy_attr();
#if SEC_MAC
static void set_proc_sl();
void ie_mkmld();
#endif

/***************************************
 * EXPORTED GLOBAL VARIABLES FOR MLTAPE.
 ***************************************/
short	ie_recovery_mode;	/* operate in recovery mode */
#if SEC_ACL
short	ie_omit_acls = 0;		/* don't export ACLs with files */
#endif
#if SEC_PRIV
short	ie_omit_privs = 0;		/* don't export privileges with files */
#endif

/**********************************
 * EXTERNAL ROUTINES AND VARIABLES. 
 **********************************/
extern char *malloc();
extern struct passwd *getpwuid();
#if SEC_MAC
extern statslabel(), lstatslabel();
extern chslabel(), lchslabel();
#if SEC_ILB
extern statilabel(), lstatilabel();
extern chilabel(), lchilabel();
#endif
#endif
extern priv_t *checksysauths();


/****************************
 * LOCAL VARIABLES
 ***************************/

/* The following is the mltape magic number.  It should match the definition
 * in mltape.c, but things will work if it doesn't
 */
#define MAGIC 071727

#define END_STRING "._MLTAPE_END_."

/* The mltape header bit positions.  Also used to indicate attribute type. */
#define MLTAPE_TYPE_FLAGS	1
#define MLTAPE_PRIVS		2
#define MLTAPE_SENS_LABEL	4
#define MLTAPE_INFO_LABEL	8
#define MLTAPE_ACL		0x10
#define MLTAPE_NCAV		0x20
#define MLTAPE_PACL             0x40
#define MLTAPE_PDACL            0x80
#define MAX_MLTAPE_HEADER_BIT	0x80
#define DONT_SET_TYPE_FLAGS	0x100
#define DONT_SET_PRIVS		0x200
#define DONT_SET_ACL		0x400
int tape_format;

int recover_fd = -1;

static struct stat sb;		/* Holds file status structure. */

/* External representation for labeling the media. */
static char *WildCard 		= "WILDCARD";

/*
 * The next section refers to the security info buffer
 * appended to the file headers.
 */
static char *buffer = NULL;
static char *bufptr = NULL;
static int bufsiz   = 0;	/* buffer size   */

/* Various pointers used to assemble the different parts of the security info
 * buffer.
 */
static char *opt = NULL;
static char *acl = NULL;
static char *pacl = NULL;
static char *pdacl = NULL;
static char *lab = NULL;
static char *ncv = NULL;
static char *kid = NULL;
static char *ppr = NULL;	/* potential priv set */
static char *gpr = NULL;	/* granted priv set */
static char *ilb = NULL;	/* information labels */
static int iskid = 0;
static int running_mltape;
static int mld_kludge;		/* Used to signal that we wish to export a file
				 * because it is an mld, even though its 
				 * sensitivity label does not pass the bounds
				 * check.
				 */
static int slink;		/* Is the current file a symbolic link? */

/*
 * Priv vectors.
 */
#ifdef SEC_PRIV
static 	privvec_t	ppriv;		/* Potential set. */
static 	privvec_t	gpriv;		/* Granted set. */
#endif
static	privvec_t 	original_privs;	/* Original set owned by
					 * process. 
					 */
static 	privvec_t 	old_privs;	/* Used to save process
					 * privs when forcing privs for
					 * an operation.
					 */

/*
 * Pointers for irs describing the file to be imported/exported.
 */
#if SEC_MAC
static mand_ir_t	*mand_ir;
static mand_ir_t	*file_ir;
static mand_ir_t	*kid_ir;	   	/* mld child level	*/
#endif
#if SEC_NCAV
static ncav_ir_t	*ncv_ir;
#endif
#if SEC_ILB
static ilb_ir_t		*ilb_ir;
#endif


/*
 * Pointers for irs. describing the process.
 */
#if SEC_MAC
static mand_ir_t	*proc_ir;		/* process level 	*/
static mand_ir_t	*proc_clr;		/* process clearance 	*/
#endif
#if SEC_NCAV
static ncav_ir_t	*proc_ncav;		/* process level	*/
#endif

/*
 * Pointers for irs. describing the device. 
 */
#if SEC_MAC
static mand_ir_t	*tape_ir;		/* device level		*/
static mand_ir_t	*tape_cur_ir;		/* dev asg db cur level	*/
static mand_ir_t	*tape_max_ir;		/* dev asg db max level	*/
static mand_ir_t	*tape_min_ir;		/* dev asg db min level	*/
#endif
#if SEC_NCAV
static ncav_ir_t	*tape_ncav;		/* dev default level	*/
static ncav_ir_t	*tape_max_ncav;		/* dev max level	*/
static ncav_ir_t	*tape_min_ncav;		/* dev min level	*/
#endif
#if SEC_ILB
static ilb_ir_t		*tape_il;		/* dev default level	*/
#endif


/* A flag and its associated bit definitions used to keep track of which 
 * policies are currently single-level.
 */
static int level_flags;		
#define SL_SLABEL	1
#define SL_ILABEL	2
#define SL_NCAVLABEL	4
#define SL_NOT_MULTI	0x100

static char 		*devnam;		/* device path 		*/
static int		asg_impexp = 0;		/* import or export 	*/


/* Strdup()
 *
 * strdup() for those systems for which it is not standard.
 */
static char *
Strdup(str)
	char *str;
{
	char *s;
	int i;

	i = strlen(str) + 1;
	s = (char *)m_alloc(i);
	strcpy(s,str);
	return(s);
}


#ifdef DEBUG
static void
audit_subsystem(aud_op,aud_res,type)
	int aud_op,aud_res,type;
{
}
#endif


/* remark(
 *	Message code)
 *
 * This routine is called to note an exception, error or auditable condition.
 * The code value selects an element of the msg[] array contained in the  file
 * sec_export.h.  Each element contains a string to be printed to stderr,
 * a string that if not NULL is used in creating an audit record, a string
 * representing the audit result, and action code, which can be either
 * TERMINATE or NOACTION.
 */
static int
remark(code)
	int code;
{
	if(msg[code].msg)
	   fprintf(stderr,"%s\n",msg[code].msg);
	if(msg[code].aud_op != NULL) {
	   setgid(starting_egid());
/*
	   audit_subsystem(msg[code].aud_op,msg[code].aud_res,ET_SUBSYSTEM);
*/
	   setgid(starting_rgid());
	}
	if(msg[code].action == TERMINATE) {
	   exit(1);
	}
	return(-1);
}



/* allocbuff(
 *	size to be allocated)
 *
 * This routine allocates a buffer for the security info header.  If the 
 * size requested fits in the old buffer, we reuse the old buffer.
 */
static char *
allocbuff(size)
	int size;
{
	if(size>bufsiz) {
	   if(buffer) free(buffer);
	   buffer = m_alloc(size);
	   bufsiz = size;
	   bufptr = buffer;
	}
	return(buffer);
}


#if SEC_MAC /*{*/
/*
 * Some helpers for multileveldirs.  Most of the time this program runs with
 * multileveldir priv off.  These routines bracket any operation that would
 * require multileveldir by enabling the privilege if the process possesses it.
 */
static int
Ismultdir(path)
	char *path;
{
	int st;
	privvec_t saveprivs;

	enableprivs(privvec(SEC_MULTILEVELDIR, -1), saveprivs);
	st = ismultdir(path);
	seteffprivs(saveprivs, (priv_t *) 0);
	return(st==1);
}
static int
Mkmultdir(path)
	char *path;
{
	int st;
	privvec_t saveprivs;

	enableprivs(privvec(SEC_MULTILEVELDIR, -1), saveprivs);
	st = mkmultdir(path);
	seteffprivs(saveprivs, (priv_t *) 0);
	return(st);
}



/* ml_bounds(
 *	ir of file to be imported or exported.)
 * 
 * This routine determines whether or not a file of the given mand_ir can be
 * imported/exported by this process.
 *
 * Returns 0 - NO.
 *	   1 - YES.
 */
static int
ml_bounds(ir)
	mand_ir_t *ir;
{
	int comp1;
	int comp2;
#ifdef DEBUG
	privvec_t current_privs;

	getpriv(SEC_EFFECTIVE_PRIV, current_privs);
	fprintf(stderr,"ml_bounds: privs are %s\n",
		privstostr(current_privs, " "));
#endif

	if(!ir) return(1);		/* wildcard */

	/* Determine the relationship between the file and the device.
 	 */
	comp1 = mand_ir_relationship(ir,tape_min_ir);
	comp2 = mand_ir_relationship(ir,tape_max_ir);

#ifdef DEBUG
	fprintf(stderr, "ml_bounds: min: %x %s\n", comp1,
		mand_ir_to_er(tape_min_ir));
	fprintf(stderr, "           file: %s\n", mand_ir_to_er(ir));
	fprintf(stderr, "           max: %x %s\n", comp2,
		mand_ir_to_er(tape_max_ir));
#endif

	/* Check normal conditions for import/export. */
	if(    (comp1 & MAND_EQUAL) ||	/* match lower level   */
               (comp2 & MAND_EQUAL) ||	/* match upper level   */
	     ( (comp1 & MAND_SDOM) &&  	/* illegal in Ga.      */
	       (comp2 & MAND_ODOM) ) )
	       return(1);

        return(0);
}


/* mld_bounds(
 *	ir of mld to be imported or exported.)
 * 
 * This routine determines whether or not a mld of the given mand_ir can be
 * imported/exported by this process. If the mld dominates the max tape level
 * then we shouldn't import/export however if the mld is dominated by the
 * tape minimum then we should because it may contain children that are in the
 * range
 *
 * Returns 0 - NO.
 *	   1 - YES.
 */
static int
mld_bounds(ir)
	mand_ir_t *ir;
{
	int comp1;
	int comp2;
#ifdef DEBUG
	priv_t 	current_privs[SEC_SPRIVVEC_SIZE];
	int i;

	if (getpriv(SEC_EFFECTIVE_PRIV, current_privs) < 0) 
		remark(MSG_GETPRIVS);

	fprintf(stderr,"Process Privilege Set = ");
	for (i=0;i<SEC_SPRIVVEC_SIZE;i++) 
		fprintf(stderr,"%x ",current_privs[i]);
	fprintf(stderr,"\n");
#endif

	if(!ir) return(1);		/* wildcard */

	/* Determine the relationship between the file and the device. */
	comp2 = mand_ir_relationship(ir,tape_max_ir);

	/* Check normal conditions for import/export. */
	if( (comp2 & MAND_SDOM) )   	/* illegal in Ga. (over 14)    */
	       return(0);

        return(1);
}

#endif /*} SEC_MAC */


#if SEC_NCAV /*{*/
/* ncav_bounds(
 *	ir of file to be imported or exported.)
 * 
 * This routine determines whether or not a file of the given ncav_ir can be
 * imported/exported by this process.
 *
 * Returns 0 - NO.
 *	   1 - YES.
 */
static int
ncav_bounds(ir)
	ncav_ir_t *ir;
{
	int comp1;
	int comp2;
	
	if(!ir) return(1);		/* wildcard */

	/* Determine the relationship between the file and the device.
 	 */
	comp1 = ncav_ir_relationship(ir,tape_min_ncav);
	comp2 = ncav_ir_relationship(ir,tape_max_ncav);

	if(    (comp1 & NCAV_EQUAL) ||	/* match lower level   */
               (comp2 & NCAV_EQUAL) ||	/* match upper level   */
	     ( (comp1 & NCAV_SDOM) &&  	/* illegal in Ga.      */
	       (comp2 & NCAV_ODOM) ) )
	       return(1);

        return(0);
}
#endif /*} SEC_NCAV */



/* ie_init()
 *
 * This procedure must be called at the start of the secure import/export
 * program to initialize the security variables and perform the required
 * authorization checks.
 */
void
ie_init(argc,argv, is_mltape)
	int argc, is_mltape;
	char *argv[];
{
	int i;

	running_mltape = is_mltape;
	set_auth_parameters(argc,argv);
	initprivs();

#if SEC_ACL
	acl_init();
#endif
#if SEC_MAC
	mand_init();
#endif
#if SEC_NCAV
	ncav_init();
#endif

#if SEC_MAC || SEC_NCAV
	/* Allocate storage for all of ir variabels. */
	if(
#if SEC_MAC
	   ((proc_ir     = mand_alloc_ir()) == NULL) ||
	   ((proc_clr    = mand_alloc_ir()) == NULL) ||
	   ((kid_ir      = mand_alloc_ir()) == NULL) ||
	   ((mand_ir     = mand_alloc_ir()) == NULL) ||
	   ((file_ir     = mand_alloc_ir()) == NULL) ||
	   ((tape_max_ir = mand_alloc_ir()) == NULL) ||
	   ((tape_min_ir = mand_alloc_ir()) == NULL) ||
	   ((tape_ir     = mand_alloc_ir()) == NULL) ||
	   ((tape_cur_ir = mand_alloc_ir()) == NULL) ||
#endif
#if SEC_NCAV
	   ((ncv_ir	   = ncav_alloc_ir()) == NULL) ||
	   ((proc_ncav	   = ncav_alloc_ir()) == NULL) ||
	   ((tape_max_ncav = ncav_alloc_ir()) == NULL) ||
	   ((tape_min_ncav = ncav_alloc_ir()) == NULL) ||
	   ((tape_ncav	   = ncav_alloc_ir()) == NULL) ||
#endif
#if SEC_ILB
	   ((ilb_ir	 = ilb_alloc_ir()) == NULL) ||
	   ((tape_il     = ilb_alloc_ir()) == NULL) ||
#endif
							0) remark(MSG_NOMEM);
#endif

	/* Record the process's original attributes */

#if SEC_MAC
	if (getclrnce(proc_clr)) remark(MSG_PROCLVL);
	if (getslabel(proc_ir)) remark(MSG_PROCLVL);
#endif
#if SEC_NCAV
	if (getncav(proc_ncav)) remark(MSG_PROCNCAV);
#endif

	/* This routine is called by mltape, cpio and tar.  If it is mltape,
	 * we jam on all sorts of privileges, but we test the kernel auths
	 * directly from within the program so it is OK.  With the other two
	 * programs we need to be more careful, so we only enable them.
	 */
	if (is_mltape) {
		/* Restore any privileges in the user's authorizations. */
        	forceprivs(privvec(SEC_CHOWN, SEC_OWNER
#if SEC_MAC
				 , SEC_ALLOWMACACCESS
#endif
#if SEC_ILB
				 , SEC_ALLOWILBACCESS
        			 , SEC_ILNOFLOAT
#endif
#if SEC_NCAV
				, SEC_ALLOWNCAVACCESS
#endif
				, -1), (priv_t *) 0);
		enableprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_PRIV
				    SEC_CHPRIV,
#endif
        			    SEC_MKNOD, SEC_LOCK, SEC_CHMODSUGID
#if SEC_MAC
        			  , SEC_WRITEUPSYSHI, SEC_WRITEUPCLEARANCE
        			  , SEC_DOWNGRADE
#endif
				  , -1), (priv_t *) 0);
	}
	else {	/* cpio or tar */
		/* Restore any privileges in the user's authorizations. */
		enableprivs(privvec(SEC_ALLOWDACACCESS, SEC_OWNER
        			  , SEC_CHMODSUGID, SEC_CHOWN
        			  , SEC_MKNOD, SEC_LOCK
#if SEC_PRIV
        			  , SEC_CHPRIV
#endif
#if SEC_MAC
				  , SEC_ALLOWMACACCESS, SEC_WRITEUPSYSHI
        			  , SEC_WRITEUPCLEARANCE, SEC_DOWNGRADE
#endif
#if SEC_ILB
				  , SEC_ALLOWILBACCESS, SEC_ILNOFLOAT
#endif
#if SEC_NCAV
				  , SEC_ALLOWNCAVACCESS
#endif
				  , -1), (priv_t *) 0);
	}

	/* Save the user's authorizations. */
	if (getpriv(SEC_EFFECTIVE_PRIV, original_privs) < 0) 
		remark(MSG_GETPRIVS);

#ifdef DEBUG
	fprintf(stderr,"Process Privilege Set = ");
	for (i=0;i<SEC_SPRIVVEC_SIZE;i++) 
		fprintf(stderr,"%x ",original_privs[i]);
	fprintf(stderr,"\n");
#endif

	level_flags = 0; 	/* Used to store whether or not the device is 
			 	 * multi/single level for sensitivity labels, 
				 * information labels and national caveats.
	 			 */
}



/* ie_pr_format(
 *	tape_format);
 *
 * Prints a description of the tape format to the stderr device.
 */
void
ie_pr_format(format)
	short format;
{
	if (format & MLTAPE_TYPE_FLAGS) fprintf(stderr,MSGSTR(TYPE_FLAG, "\n\tTYPE FLAGS"));
	if (format & MLTAPE_PRIVS) 	fprintf(stderr,MSGSTR(PRIVS, "\n\tPRIVS"));
	if (format & MLTAPE_SENS_LABEL) fprintf(stderr,MSGSTR(SENS_LBL, "\n\tSENS. LABEL"));
	if (format & MLTAPE_INFO_LABEL) fprintf(stderr, "\n\tINFO. LABEL");
	if (format & MLTAPE_ACL) 	fprintf(stderr,MSGSTR(ACL_I_E, "\n\tACL"));
	if (format & MLTAPE_NCAV) 	fprintf(stderr,"\n\tNCAV");
	fprintf(stderr,"\n");
}

	
/* ie_check_device(
 *	device pathname,
 *	single or multi level,
 *	import or export,
 *	device file descriptor)
 *
 * This procedure checks the device to see if it can be used for the requested
 * operation.  It also fills out the needed IR's and flags so the software can
 * handle single/multi levels for the sensitivity label, information label and
 * national caveat set, as configured.
 *
 * The routine does not return anything.  It exits upon failure.
 */
void
ie_check_device(dev,direction)
	char *dev;	
	int direction;	/* AUTH_DEV_IMPORT,AUTH_DEV_EXPORT,AUTH_DEV_PASS */
{
	struct dev_asg *d;
	char **alias;
	int comp1,comp2;
	int st;
	int i,x;
	struct passwd *pw;
	privvec_t saveprivs;

	struct ml_bin_format {
		short magic;
		short format;
	} *ml_bin_format;
	char ml_format_buffer[10];
	int ml_magic;
	int ml_format;

	asg_impexp  = direction;	/* Remember transfer direction. */
	tape_format = 0;		/* tape_format contains a bit map of
		 * all security policies that will be used during the transfer.
		 */

	/* If the -A or -P options are set requesting that we not copy a
	 * security attribute, make sure that the user is privileged to do
	 * so.
	 */
#if SEC_PRIV
	if (!hassysauth(SEC_CHPRIV)) 		ie_omit_privs = 0;
#endif
#if SEC_ACL
	if (!hassysauth(SEC_ALLOWDACACCESS)) 	ie_omit_acls = 0;
#endif


	/********************
	 *  PASS MODE 
	 ********************/
	/* In pass mode there is no device to check. */
	if (direction == AUTH_DEV_PASS) {
#if SEC_MAC
		/* In pass mode we consider the destination directory to be
		 * a device with a range set from the user's clearance
		 * down to syslo.
		 */
		mand_copy_ir(proc_clr, tape_max_ir);
		mand_copy_ir(mand_syslo, tape_min_ir);
		tape_format |= MLTAPE_SENS_LABEL;
#else
		level_flags |= SL_SLABEL;
#endif
#if SEC_ILB
		tape_format |= MLTAPE_INFO_LABEL;
#else
		level_flags |= SL_ILABEL;
#endif
#if SEC_NCAV
		tape_max_ncav = proc_ncav;
		tape_min_ncav = ncav_max_ir;
		tape_format |= MLTAPE_NCAV;
#else
		level_flags |= SL_NCAVLABEL;
#endif

#if SEC_ACL_SWARE
		if (!ie_omit_acls) tape_format |= MLTAPE_ACL;
#endif
#if SEC_ACL_POSIX
                if (!ie_omit_acls) {
                        tape_format |= MLTAPE_PACL;
                        tape_format |= MLTAPE_PDACL;
                }
#endif

#if SEC_PRIV
		if (!ie_omit_privs) tape_format |= MLTAPE_PRIVS;
#endif
		return;
	}


	/*********************************
	 * IMPORT OR EXPORT
 	 *********************************/

	if(!authorized_user("tape")) remark(MSG_NOAUTH);

	/* We do not allow the user to specify a device through indirection */
	if (dev == NULL) {
	   if (direction == AUTH_DEV_IMPORT)
		fprintf(stderr,MSGSTR(ERR_I_OPTION, "ERROR: specify input device with -I option\n"));
	   else fprintf(stderr,MSGSTR(ERR_O_OPT, "ERROR: specify output device with -O option\n"));
	   exit(1);
	}

	/* See if the user has the recovery authorization. */
	if (ie_recovery_mode) {
		/* If this flag is set, we must have the authorization to
		 * continue, for we are trying to invoke trusted recovery.
		 */
		if (ie_recovery(dev)) exit(1);
		else return;
	}

#if SEC_MAC
	if (!running_mltape)
		forceprivs(privvec(SEC_ALLOWMACACCESS, -1), saveprivs);
	/* Make sure the device has a proper label. */
	if (statslabel(dev,tape_ir)!=0)
		if (errno != EINVAL) {
			fprintf(stderr, "%s: ", dev);
		   	remark(MSG_DEVLAB);
		} else {
			mand_free_ir(tape_ir);
			tape_ir = (mand_ir_t *) 0;
		}
#endif

#ifdef DEBUG
	fprintf(stderr,"*ie_check_device():\n");
	fprintf(stderr," tape: %s\n",dev);
#endif

	/* Check the device assignment data base to see if the requested device
	 * has been configured by the SSO.
	 */
	while( (d = getdvagent()) != NULL) {	/* get a device record. */
   		if(d->uflg.fg_devs) {
			/* Get pointer to the list of valid device pathnames	
			 * for this device.
			 */
      			alias = d->ufld.fd_devs;
	
			/* Search the list for a match for our device. */
      			while(*alias != NULL) {
         			if(strcmp(dev,*alias) == 0) break;
	 			alias++; 
      			}
       		}
		/* break out of loop if we found a match. */
   		if(*alias != NULL) break;
	}

	/* If we didn't find the device in the data base, then only accept if
	 * it is a regular file or the file does not exist.
	 */
	if (d == NULL) {
		st = stat(dev,&sb);

	   	if (((st ==  0) && ((sb.st_mode & S_IFMT) == S_IFREG)) ||
		    ((st == -1) && (errno == ENOENT))) {
#if SEC_MAC
			/* Input/Output to disk file.  Consider the disk file 
			 * to be a multilevel device ranging from the 
			 * process clearance down to syslo.
		 	 */
			if (tape_ir == (mand_ir_t *) 0) {
				tape_ir = mand_alloc_ir();
				if (tape_ir == (mand_ir_t *) 0)
					remark(MSG_NOMEM);
			}
			mand_copy_ir(proc_ir, tape_ir);
			mand_copy_ir(mand_syslo, tape_min_ir);
			mand_copy_ir(proc_clr, tape_max_ir);
#endif


#if SEC_MAC
			tape_format |= MLTAPE_SENS_LABEL | MLTAPE_TYPE_FLAGS;
#endif
#if SEC_ILB
			tape_format |= MLTAPE_INFO_LABEL;
#endif
#if SEC_PRIV
			if (!ie_omit_privs)	tape_format |= MLTAPE_PRIVS;
#endif
#if SEC_ACL_SWARE
			if (!ie_omit_acls)	tape_format |= MLTAPE_ACL;
#endif
#if SEC_ACL_POSIX
                        if (!ie_omit_acls) {
                                tape_format |= MLTAPE_PACL;
                                tape_format |= MLTAPE_PDACL;
                        }
#endif

#if !SEC_MAC
			/* If MAC is not configured, we will consider the
			 * file to be single-level WRT sensitivity labels.
			 */
			level_flags |= SL_SLABEL;
#endif

#if !SEC_ILB
			/* If ILB's are not configured, we will consider the 
			 * file to be single-level WRT information labels.
	 		 */
			level_flags |= SL_ILABEL;
#endif

#if SEC_NCAV
			/* If NCAV's are configured, we will consider the file
			 * to be a multi-level ncav device with a max level
			 * set at the process's current level, and the min
			 * level set to the full caveat set.
			 */
			tape_ncav     = proc_ncav;
			tape_max_ncav = proc_ncav;
			tape_min_ncav = ncav_max_ir;

			tape_format |= MLTAPE_NCAV;
#else
			/* If NCAV's are not configured, we will consider the
			 * file to be single-level WRT nationality caveats.
			 */
			level_flags |= SL_NCAVLABEL;
#endif

#ifdef DEBUG
	      		fprintf(stderr,
				"ie_check_device(): disk save set specified\n");
#endif
	      		return;
		}
		/* Trying to redirect to device, note error and exit. */
		else {
			fprintf(stderr, "%s: ", dev);
			remark(MSG_DEVDB);
		}
	}

	/* 
	 * Is user authorized to use this device?
	 */
	if (d->uflg.fg_users) {
		pw = getpwuid(getluid());    /* Get user name. */
		alias = d->ufld.fd_users;    /* Get list of allowed users */
		while(*alias != NULL) {	     /* Search for match. */
			if(strcmp(pw->pw_name,*alias) == 0)
				break;
			alias++; 
		}
		if(*alias == NULL) remark(MSG_AUTHDEV);
	}

	/* 
	 * Make sure that the device is authorized for the requested operation,
	 * i.e. import or export.
	 */
	if (!d->uflg.fg_assign || !ISBITSET(d->ufld.fd_assign,asg_impexp)) 
           remark(MSG_DEVIMPEXP);


	/* Set up the tape format parameters. */
#if SEC_PRIV
	if (!ie_omit_privs) tape_format |= MLTAPE_PRIVS;
#endif
#if SEC_ACL_SWARE_SWARE
	if (!ie_omit_acls) tape_format |= MLTAPE_ACL;
#endif
#if SEC_ACL_POSIX
        if (!ie_omit_acls) {
                tape_format |= MLTAPE_PACL;
                tape_format |= MLTAPE_PDACL;
        }
#endif

#if SEC_MAC /*{*/
	/*********************************************
	 * Check the device sensitivity label range.
	 ********************************************/
	/* Is it a multi-level device? */
	if (ISBITSET(d->ufld.fd_assign,AUTH_DEV_MULTI)) {
		/* Can only assign it one way. */
		if (ISBITSET(d->ufld.fd_assign,AUTH_DEV_SINGLE)) 
			remark(MSG_DEVASSIGN);

		if (d->uflg.fg_min_sl)
			mand_copy_ir(d->ufld.fd_min_sl, tape_min_ir);
		else if (d->sflg.fg_min_sl)
			mand_copy_ir(d->sfld.fd_min_sl, tape_min_ir);
		else remark(MSG_DEVSENSLEV);

		if (d->uflg.fg_max_sl)
			mand_copy_ir(d->ufld.fd_max_sl, tape_max_ir);
		else if (d->sflg.fg_max_sl)
			mand_copy_ir(d->sfld.fd_max_sl, tape_max_ir);
		else remark(MSG_DEVSENSLEV);

		tape_format |= MLTAPE_SENS_LABEL | MLTAPE_TYPE_FLAGS;
	}
	/* Is it a single level device? */
	else if (ISBITSET(d->ufld.fd_assign,AUTH_DEV_SINGLE)) {
		if (d->uflg.fg_cur_sl)
			mand_copy_ir(d->ufld.fd_cur_sl, tape_cur_ir);
		else if (d->sflg.fg_cur_sl)
			mand_copy_ir(d->sfld.fd_cur_sl, tape_cur_ir);
		else remark(MSG_DEVNOLEV);

		level_flags |= SL_SLABEL;
		/*
		 * Check that the device's sensitivity level matches
		 * the single level assignment
		 */
#ifdef DEBUG
		fprintf(stderr,
		  "ie_check_device: comparison single tape er %s\n",
		  mand_ir_to_er(tape_ir));
		fprintf(stderr, "ie_check_device: tape_cur_ir %s\n",
		  mand_ir_to_er(tape_cur_ir));
#endif
		if (tape_ir != (mand_ir_t *) 0 &&
		    memcmp(tape_ir, tape_cur_ir, mand_bytes()) != 0)
			remark(MSG_DEVSLMATCH);
	}

	/* The SSO must specify the device in one of the options above, else
	 * we have an error, so notify the user and exit. 
	 */
	else remark(MSG_DEVNOLEV);

	/* If single-level, only files that match the designated level
	 * exactly can be exported.  Restrict the min/max_ir to the 
	 * designated device level.
	 */
	if (level_flags & SL_SLABEL) {
		mand_copy_ir(tape_cur_ir, tape_max_ir);
		mand_copy_ir(tape_cur_ir, tape_min_ir);
#ifdef DEBUG
		if (tape_ir) lab = mand_ir_to_er(tape_ir);
		else lab = WildCard;
		fprintf(stderr,
			"Device single-level for sensitivity labels: %s\n",lab);
#endif
	}
	else {	/* If the device is multi-level, make sure that we are using
		 * one of the multilevel programs.
		 */
	   if (!running_mltape) {
		fprintf(stderr,
		       MSGSTR(ERR_DEV_MLS, "ERROR: the device is configured for multi-level sls\n"));
		exit(1);
	   }
#ifdef DEBUG
	   if (tape_max_ir) lab = mand_ir_to_er(tape_max_ir);
	   else lab = WildCard;
	   fprintf(stderr,"Device multi-level for sens. labels, max: %s\n",lab);
	   if (tape_min_ir) lab = mand_ir_to_er(tape_min_ir);
	   else lab = WildCard;
	   fprintf(stderr,"                                     min: %s\n",lab);
#endif
	}

	/*
	 * Now make sure that the specified sensitivity range is valid.
	 */

  	/* Determine the relationship between the process's clearance and the 
	 * device. 
	 */
	if (memcmp(proc_clr, tape_max_ir, mand_bytes()) == 0)
		comp1 = MAND_EQUAL;
	else
   		comp1 = mand_ir_relationship(proc_clr,tape_max_ir);
	if (memcmp(tape_max_ir, tape_min_ir, mand_bytes()) == 0)
		comp2 = comp1;
	else if (memcmp(proc_clr, tape_min_ir, mand_bytes()) == 0)
		comp2 = MAND_EQUAL;
	else
   		comp2 = mand_ir_relationship(proc_clr,tape_min_ir);
#ifdef DEBUG
	fprintf(stderr, "ie_check_device: tape max er: %s\n",
	  mand_ir_to_er(tape_max_ir));
	fprintf(stderr, "ie_check_device: tape min er: %s\n",
	  mand_ir_to_er(tape_min_ir));
	fprintf(stderr, "ie_check_device: proc clr er: %s\n",
	  mand_ir_to_er(proc_clr));
	fprintf(stderr, "ie_check_device: comp1 0x%x comp2 0x%x\n",
	  comp1, comp2);
#endif

	/* If the process has allowmac, no need to check further. */
	if (!hassysauth(SEC_ALLOWMACACCESS)) {
	   /* See if it dominates the tape. */
   	   if (!(comp1 & (MAND_EQUAL | MAND_SDOM))) {
		/* If the process does not dominate the tape, see if it
		 * can at least use a subrange.  We only allow this for
		 * multi-level devices.
		 */
		if (level_flags & SL_SLABEL) remark(MSG_PNOTDOM);
		if (comp2 & (MAND_EQUAL | MAND_SDOM)) {
			/* We do get a subrange, so reset our notion of the
			 * maximum tape level. 
			 */
			mand_copy_ir(proc_clr, tape_max_ir);
		}
		else remark(MSG_PNOTDOM);
	   }
	}

	/* For single-level devices, set the process level equal to 
	 * the device label.
 	 */
	if (level_flags & SL_SLABEL) {
		mand_copy_ir(tape_ir, proc_ir);
	 	if(setslabel(proc_ir) < 0) remark(MSG_CHGLEV);
	}
	if (!running_mltape)
		seteffprivs(saveprivs, (priv_t *) 0);
#endif /*} SEC_MAC */


#if SEC_ILB /*{*/
	/*******************************************
	 * Check the device information label range.
	 *******************************************/
	/* Are they multi-level? */
	if (ISBITSET(d->ufld.fd_assign,AUTH_DEV_ILMULTI)) {
		/* Can't define it to be both. */
		if (ISBITSET(d->ufld.fd_assign,AUTH_DEV_ILSINGLE)) 
			remark(MSG_DEVNOILB);

		tape_il = (ilb_ir_t *)0;

		tape_format |= MLTAPE_INFO_LABEL;
	}
	/* Are they single-level? */
	else if (ISBITSET(d->ufld.fd_assign,AUTH_DEV_ILSINGLE)) {
		if (d->uflg.fg_cur_il)
			tape_il = d->ufld.fd_cur_il;
		else if (d->sflg.fg_cur_il)
			tape_il = d->sfld.fd_cur_il;
		else remark(MSG_DEVNOILB);

		level_flags |= SL_ILABEL;
	}
	else remark(MSG_DEVNOILB);

	if (level_flags & SL_ILABEL) {
#ifdef DEBUG
	   if (tape_il) ilb = ilb_ir_to_er(tape_il);
	   else ilb = WildCard;
	   fprintf(stderr,"Device single-level for info. labels: %s\n",ilb);
#endif
	}
	else {
	   if (!running_mltape) {
		fprintf(stderr,
		       "ERROR: the device is configured for multi-level ils\n");
		exit(1);
	   }
#ifdef DEBUG
	   fprintf(stderr,"Device multi-level for info. labels.\n");
#endif
	}
#else
	/* If ILB's are not configured for this system, then we will ignore
	 * them by considering the device to be single level.
	 */
	level_flags |= SL_ILABEL;
#ifdef DEBUG
	fprintf(stderr,"Information labels not configured.\n");
#endif

	/* There are no checks here on the value for the information label, for
	 * we allow the wildcard on single-level devices.
	 */
#endif /*} SEC_ILB */


#if SEC_NCAV /*{*/
	/*****************************************
	 * Check the device national caveat range.
	 *****************************************/
	/* Check for national caveat set.  First, is the device multi-level? */
	if (ISBITSET(d->ufld.fd_assign,AUTH_DEV_NCAVMULTI)) {
		/* Can't define it to be both. */
		if (ISBITSET(d->ufld.fd_assign,AUTH_DEV_NCAVSINGLE)) 
			remark(MSG_DEVASSIGN);

		if (d->uflg.fg_min_ncav)
			tape_min_ncav = d->ufld.fd_min_ncav;
		else if (d->sflg.fg_min_ncav)
			tape_min_ncav = d->sfld.fd_min_ncav;
		else remark(MSG_DEVNCAVLEV);

		if (d->uflg.fg_max_ncav)
			tape_max_ncav = d->ufld.fd_max_ncav;
		else if (d->sflg.fg_max_ncav)
			tape_max_ncav = d->sfld.fd_max_ncav;
		else remark(MSG_DEVNCAVLEV);

		tape_format |= MLTAPE_NCAV;
	}
	/* Else is it single-level */
	else if (ISBITSET(d->ufld.fd_assign,AUTH_DEV_NCAVSINGLE)) {
       		if (d->uflg.fg_cur_ncav)
			tape_ncav = d->ufld.fd_cur_sl;
       		else if (d->sflg.fg_cur_ncav)
			tape_ncav = d->sfld.fd_cur_sl;
		else remark(MSG_DEVNCAVLEV);

		level_flags |= SL_NCAVLABEL;
	}
	else remark(MSG_DEVNCAVLEV);
		
	/* Make sure that our ncav labels are  valid. */
	if (level_flags & SL_NCAVLABEL) {
		/* For single level device, restrict the export range to the
		 * designated device level only.
		 */
		tape_max_ncav = tape_ncav;
		tape_min_ncav = tape_ncav;
#ifdef DEBUG
		if (tape_ncav) ncv = ncav_ir_to_er(tape_ncav);
		else ncv = WildCard;
		fprintf(stderr,
			"Device single-level for nat. caveats: %s\n",ncv);
#endif
	}
	else {	/* The device is set up multi-level in the database. */
	   	if (!running_mltape) {
		   fprintf(stderr,
		       MSGSTR(ERR_DEV_ML_USE, "ERROR: the device is configured for multi-level use\n"));
		   exit(1);
	   	}

		/* No wildcards allowed for multi level devices. */
		if(tape_max_ncav == (ncav_ir_t *)0) remark(MSG_DEVNCAVLEV);
		if(tape_min_ncav == (ncav_ir_t *)0) remark(MSG_DEVNCAVLEV);
#ifdef DEBUG
	if (tape_max_ncav) ncv = ncav_ir_to_er(tape_max_ncav);
	else ncv = WildCard;
	fprintf(stderr,"Device multi-level for nat. caveats, max: %s\n",ncv);
	if (tape_min_ncav) ncv = ncav_ir_to_er(tape_min_ncav);
	else ncv = WildCard;
	fprintf(stderr,"                                     min: %s\n",ncv);
#endif
	}

  	/* Determine the relationship between the process and the 
	 * device. 
	 */
   	comp1 = ncav_ir_relationship(proc_ncav,tape_max_ncav);
   	comp2 = ncav_ir_relationship(proc_ncav,tape_min_ncav);

	/* If the process has allowncav, no need to check further. */
	if (!hassysauth(SEC_ALLOWNCAVACCESS)) {
	   /* See if if dominates the tape. */
   	   if (!(comp1 & (NCAV_EQUAL | NCAV_SDOM))) {
		/* If the process does not dominate the tape, see if it
		 * can at least use a subrange.  We only allow this for
		 * a multi-level device.
		 */
		if (level_flags & SL_NCAVLABEL) remark(MSG_PNOTDOM);
		if (comp2 & (NCAV_EQUAL | NCAV_SDOM)) {
			/* We do get a subrange, so reset our notion of the
			 * maximum tape level. 
			 */
			tape_max_ncav = proc_ncav;
		}
		else remark(MSG_PNOTDOM);
	   }
	}

	/* For single-level devices, set the process level equal to 
	 * the device label.
 	 */
	if (level_flags & SL_NCAVLABEL) {
		proc_ncav     = tape_ncav;
	 	if(setncav(proc_ncav) < 0) remark(MSG_CHGLEV);
	}

#else
	/* If NCAV's are not configured for this system, then we will ignore
	 * them by considering the device to be single level.
	 */
	level_flags |= SL_NCAVLABEL;
#ifdef DEBUG
	fprintf(stderr, "National Caveats not configured.\n");
#endif
#endif /*} SEC_NCAV */

#if MLTAPE
	/* If we have made it this far, we have passed all tests and import/
	 * export will proceed.  If this is export, write the tape format
	 * to the tape so we can remember it.  If this is import, read in
	 * the tape format and compare it to what the device is configured
	 * for.  The header consists of two shorts: magic_number,tape_format.
	 */
	if (running_mltape) { 	/* header only for mltape. */
	   if (asg_impexp == AUTH_DEV_IMPORT) {
		/* Import, read in the header from the tape and  compare it
		 * against the device settings.
		 */
		readhdr(ml_format_buffer,12);
		sscanf(ml_format_buffer,"%o %x",&ml_magic,&ml_format);

		/* See if magic number matches. */
		if (ml_magic != MAGIC) remark(MSG_BADMAGIC);

		/* See if formats are compatible. */
		if (ml_format != tape_format) {
		   i = ml_format ^ tape_format;
	           for (x=1; x<=MAX_MLTAPE_HEADER_BIT; x=x<<1) {
			switch (x & i) {
			   case 0: break;
			   case MLTAPE_TYPE_FLAGS: /* OK if flags don't match */
				if (ml_format & MLTAPE_TYPE_FLAGS) {
					/* Tape has type field, but we do not
					 * want to set it. 
					 */
					tape_format |= MLTAPE_TYPE_FLAGS;
					tape_format |= DONT_SET_TYPE_FLAGS;
				}
				else {
					/* Tape does not have type flags, so
					 * we cannot read or set it.
					 */
					tape_format &= ~MLTAPE_TYPE_FLAGS;
				}
				break;
			   case MLTAPE_PRIVS: /* OK if privs don't match */
				if (ml_format & MLTAPE_PRIVS) {
					/* Tape has privs, but we do not
					 * want to set it. 
					 */
					tape_format |= MLTAPE_PRIVS;
					tape_format |= DONT_SET_PRIVS;
				}
				else {
					/* Tape does not have privs, so
					 * we cannot read or set it.
					 */
					tape_format &= ~MLTAPE_PRIVS;
				}
				break;
			   case MLTAPE_ACL: 	/* OK if acl doesn't match */
				if (ml_format & MLTAPE_ACL) {
					/* Tape has acls, but we do not
					 * want to set it. 
					 */
					tape_format |= MLTAPE_ACL;
					tape_format |= DONT_SET_ACL;
				}
				else {
					/* Tape does not have acls , so
					 * we cannot read or set it.
					 */
					tape_format &= ~MLTAPE_ACL;
				}
				break;
 			case MLTAPE_PACL:    /* OK if acl doesn't match */
                                if (ml_format & MLTAPE_PACL) {
                                        /* Tape has posix acls, but we do not
                                         * want to set it.
                                         */
                                        tape_format |= MLTAPE_PACL;
                                        tape_format |= DONT_SET_ACL;
                                }
                                else {
                                        /* Tape does not have posix acls , so
                                         * we cannot read or set it.
                                         */
                                        tape_format &= ~MLTAPE_PACL;
                                }
                                break;
                    	case MLTAPE_PDACL:   /* OK if acl doesn't match */
                                if (ml_format & MLTAPE_PDACL) {
                                        /* Tape has posix acls, but we do not
                                         * want to set it.
                                         */
                                        tape_format |= MLTAPE_PDACL;
                                        tape_format |= DONT_SET_ACL;
                                }
                                else {
                                        /* Tape does not have posix acls , so
                                         * we cannot read or set it.
                                         */
                                        tape_format &= ~MLTAPE_PDACL;
                                }
                                break;

			   case MLTAPE_SENS_LABEL:	/* ERROR if any of the
			   case MLTAPE_INFO_LABEL:	 * labels don't match
			   case MLTAPE_NCAV:		 */
			   default:
		   	     fprintf(stderr,
				MSGSTR(MLTAPE_ERR, "\nMLTAPE ERROR: Incorrect tape format\n\n"));
		   	     fprintf(stderr, MSGSTR(TAPE_FORMAT, "Tape Format = "));
			     ie_pr_format(ml_format);
		   	     fprintf(stderr, MSGSTR(DEV_FORMAT, "Dev. Format = "));
			     ie_pr_format(tape_format);
			     exit(1);
			}
		   }
		}
	   }
	   else {	/* EXPORT - write out tape header. */
		sprintf(ml_format_buffer,"%06o %04x\0",MAGIC,tape_format);
		writehdr(ml_format_buffer,12);
	   }
	}
#endif /* MLTAPE */
}


/* ie_sl_export(
 *	pathname)
 *
 * This procedure checks files to be written to single-level
 * devices to see if the export is allowed.  Checks are made with all
 * configured policies.
 *
 * Returns: 0 - not OK
 *	    1 - OK.
 */
int
ie_sl_export(path, follow)
	char *path;
	int follow;
{
	int comp;
#if SEC_MAC
	extern int statslabel(), lstatslabel();
#endif

	/* 
	 * for pass operation, allow regardless of level.
	 */
	 if(asg_impexp == AUTH_DEV_PASS) return(1);


#if SEC_MAC /*{*/
	/*************************************
	 * First check sensitivity labels. 
	 ************************************/

	/* Get file level */
	if((follow ? statslabel : lstatslabel)(path,file_ir)) {
	   if(errno==EINVAL)			/* Wildcard always ok */
	      goto ie_sl_ex_mac_ok;
	   else {				/* Can't get label */
	      fprintf(stderr,"%s: ",path);
	      remark(MSG_BADLAB);		/* ignore file     */
	      return(0);
	   }
	}

	/*
	 * Determine  relationship of file & tape.
	 */
	if (memcmp(file_ir, tape_ir, mand_bytes()) != 0) {
		privvec_t saveprivs;

		forceprivs(privvec(SEC_ALLOWMACACCESS, -1), saveprivs);
		comp = mand_ir_relationship(file_ir,tape_ir);
		seteffprivs(saveprivs, (priv_t *) 0);

		/* Only export the file if its level matches the tape level. */
		if (!(comp & MAND_EQUAL)) {
	      		fprintf(stderr,"%s: ",path);
			remark(MSG_NOTEXP);
			return(0);
		}
	}
ie_sl_ex_mac_ok:
#endif /*} SEC_MAC */


#if SEC_NCAV /*{*/
	/*************************************
	 * Now check national caveats
	 ************************************/

	if(statncav(path,ncv_ir)) {		/* Get file level */
	   if(errno==EINVAL)			/* Wildcard always ok */
	      goto ie_sl_ncav_ok;
	   else {				/* Can't get label */
	      fprintf(stderr,"%s: ",path);
	      remark(MSG_BADNCAV);		/* ignore file     */
	      return(0);
	   }
	}

	/*
	 * Determine  relationship of file & tape.
	 */
	comp = ncav_ir_relationship(ncv_ir,tape_ncav);

	if (!(comp & NCAV_EQUAL)) {
		fprintf(stderr,"%s: ",path);
		remark(MSG_NOTEXPCAV);
		return(0);
	}

ie_sl_ncav_ok:
#endif /*} SEC_NCAV */

	return(1);
}

#if SEC_MAC
static void
set_proc_sl(path)
	char *path;
{
	char *parent;
	char *p;
	int	i;
	mand_ir_t	*parent_ir;

	p = path + strlen (path);
	while ( (*p != '/') && (p != path) ) 
		p--;
	parent_ir = mand_alloc_ir();
	if (p == path) {
		i = statslabel (".", parent_ir);
	}
	else {
		parent = (char *) malloc (strlen(path) + 1);
		strncpy (parent, path, (p -path));
		parent [p - path] = '\0';
		i = statslabel (parent, parent_ir);
	}
	/* Need to change the process sensitivity level */
	if (i == 0)
		setslabel (parent_ir);
}
#endif

/* ie_openout(
 *	pathname to be opened,
 *	mode to open it in,
 *	type of file,	0 = file,
 *			1 = directory,
 *			2 = special.
 *	extra flags) 	if type = dir, 1=mld;
 *			if type = special, flags=device number.
 *
 * This routine creates a file of the specified type.
 *
 * Returns the file descriptor.
 */
int
ie_openout(path,mode,type,extra)
	char *path;
	int mode,type,extra;
{
	int f;

#if SEC_MAC
	set_proc_sl(path);
#endif

	switch(type) {
	   case 0:				/* file */
			if (extra)			/* symlink */
				f = 0;
			else
				f=creat(path,0200);	/* file */
			break;
	   case 1:				/* dir */
			f = mkdir(path,mode);
#if SEC_MAC
			if (!f && extra && 
				hassysauth(SEC_MULTILEVELDIR)) 
			    ie_mkmld(path);
#endif
			break;
	   case 2:				/* special */
			if ((mode & S_IFMT) != S_IFIFO &&
			    !hassysauth(SEC_MKNOD))
				return(-1);
			f = mknod(path,mode,extra);
			break;
	   default:
			return -1;
	}
			
#ifdef DEBUG
	if (f < 0) {
		privvec_t curprivs;
		static char *ops[] = { "creat", "mkdir", "mknod" };
#if SEC_MAC
		mand_ir_t *proc_ir;
#endif

		fprintf(stderr, "%s ", ops[type]);
		perror(path);
		getpriv(SEC_EFFECTIVE_PRIV, curprivs);
		fprintf(stderr, MSGSTR(PRIVS_I_E, "    privs: %s\n"), privstostr(curprivs, " "));
#if SEC_MAC
		if ((proc_ir = mand_alloc_ir()) && getslabel(proc_ir) == 0)
		    fprintf(stderr, MSGSTR(LABEL_I_E, "    label: %s\n"), mand_ir_to_er(proc_ir));
		if (proc_ir)
		    free(proc_ir);
#endif
	}
#endif
	return(f);
}



/* ie_ml_import()
 *
 * This procedure, called after the security info has been
 * read, determines whether we can import the given file.
 *
 * Returns 	-1 if fails security tests.
 *		0 if failure creating file.
 *		1 for non-files and links
 *		file descriptor if successful for type=file
 */
/* file types. */
#define IE_FILE		0
#define IE_DIR		1
#define IE_SPECIAL	2
#define IE_LINK		3
/* extra bit */
#define IE_NOT_SYM_LINK	0
#define IE_SYM_LINK	1
#define IE_MLD		1
int
ie_ml_import(path,mode,type,extra)
	char *path;	/* pathname */
	int  mode;	/* desired mode */
	int  type;	/* 0=file 1=dir 2=special 3=link */
	int  extra;	/* Extra info:
				for files    = symbolic link.
				for specials = dev no
				for dirs     = 1 for mld
			 */
{
	int f;
	int st;
	int done;
#if SEC_MAC
	mand_ir_t *k_ir;
	mand_ir_t *f_ir;
#endif
#if SEC_ILB
	ilb_ir_t  *f_il;
#endif
#if SEC_NCAV
	ncav_ir_t *f_ncav;
#endif

	/* We can bypass checks in pass mode, for we already checked the
	 * file in ie_ml_export.
	 */
	if (asg_impexp==AUTH_DEV_PASS) {
		st = ie_openout(path,mode,type,extra);
		if (type == IE_FILE && extra != IE_SYM_LINK) return(st);
		else if (st==0) return(1);
		return(0);
	}

	/* Is the file a symbolic link? */
	if ((type == IE_FILE) && (extra == IE_SYM_LINK)) slink = 1;
	else slink = 0;

#if SEC_MAC
	mld_kludge = 0;
#endif

	/* For the trusted recovery mechanism, we bypass security checks. 
	 * This mechanism is only allowed to the system administrator if
	 * properly authorized, and is used only for restoring critical files
	 * after a crash severe enough to wipe out the security databases.
	 */
	if (ie_recovery_mode) {
		iskid = 0;
		f = -1;
		goto recovery;	
	}

	/*
	 * Unpack the security info buffer
	 */
#ifdef DEBUG
	fprintf(stderr,"ie_ml_import(): path %s\n",path);
	fprintf(stderr,"buffer contents:\n");
#endif
	ie_unpack();

	/***********************************
	 * Check the file's sensitivity label
	 ***********************************/
	/* If we are expecting a label and don't have one, there is a tape
	 * format error.
	 */
	if (tape_format & MLTAPE_SENS_LABEL) {
	   if (strlen(lab) == 0) {
		fprintf(stderr,MSGSTR(MISSING_SL, "%s: missing sensitivity label\n"), path);
		remark(MSG_TAPEFMT);
	   }
	}
	else if (lab) {
		fprintf(stderr,MSGSTR(UNEXPECTED_SL, "%s: unexpected sensitivity label\n"), path);
		remark(MSG_TAPEFMT);
	}

	/* If we are set up for a single-level device. */
	if (level_flags & SL_SLABEL) {
		/* If we have a label, it is a tape format error. */
		if (lab) remark(MSG_TAPEFMT);
	}
	else {	/* We are set for a multi-level device, which is only valid
	   	 * if the policy is running on the system.
		 */
#if SEC_MAC /*{*/
	   if (strcmp(lab,WildCard) == 0) f_ir = NULL;
	   else {
		/* We have some error trapping around the er_to_ir conversion
		 * to allow a privileged user to reenter the label if it
		 * cannot be converted by this system.
		 */
		done = 1;
		do {
			f_ir = mand_er_to_ir(lab);
			if (!f_ir) done = ie_new_label(MLTAPE_SENS_LABEL,&lab);
		} while (!f_ir && !done);
		if (!f_ir) {
			remark(MSG_BADLAB);
			return(-1);
		}
	   }

	   /* If the imported file is an mld, we do not perform a bounds check.
	    * by definition the directory is valid for all levels.  Instead
	    * we set the mld_kludge flag to tell set_er() to handle this
	    * file differently.
	    */
	   if ((type==IE_DIR) && (extra==IE_MLD)) {
                /* If SL of mld dominates tape max level then don't want it */
                if (!mld_bounds(f_ir)) {
			fprintf(stderr, "%s: ", path);
                        remark(MSG_NOTIMP);
                        return (-1);
                }
		mld_kludge = 1;
	   }
	   /* 
	    * within bounds?  
	    */
	   else if (!ml_bounds(f_ir)) {
		fprintf(stderr, "%s: ", path);
	   	remark(MSG_NOTIMP);
		if (f_ir) free(f_ir);
	      	return(-1);
	   }
	   if (f_ir) free(f_ir);
#else /*}{ !SEC_MAC */
	   fprintf(stderr, MSGSTR(ERR_SYS_NOT_CONF_SL, "ERROR: System not configured for sensitivity labels\n"));
	   remark(MSG_TAPEFMT);
#endif /*} SEC_MAC */
	}

	/************************************
	 * Check the file's information label
	 ************************************/
	/* If we are expecting a label and don't find one, it is a tape
	 * format error.
	 */
	if (tape_format & MLTAPE_INFO_LABEL) {
#if SEC_ILB
	   if (strlen(ilb) == 0) {
		fprintf(stderr,"ERROR: Tape is missing information label.\n");
		remark(MSG_TAPEFMT);
	   }
#else
	   fprintf(stderr,"ERROR: System not set up for info. labels.\n");
	   remark(MSG_TAPEFMT);
#endif
	}
	else if (ilb) {
		fprintf(stderr,"ERROR: Device not set for info. labels.\n");
		remark(MSG_TAPEFMT);
	}

	/* We are pretty easy about info.labels.  We let in anything as long
	 * as it matches our expectations about the existence of the label.
	 */

	/**********************************
	 * Check national caveat set.
	 **********************************/
	/* If we are expecting a label and it doesn't exist, we have a  tape
	 * format error, unless the file is a symbolic link, in which case
	 * we loaded in a null string because we don't have a lstatncav().
	 */
	if (!slink) {
	   if (tape_format & MLTAPE_NCAV) {
	     if (strlen(ncv) == 0) {
		fprintf(stderr, "ERROR: Tape is missing national caveats.\n");
		remark(MSG_TAPEFMT);
	     }
	   }
	   else if (ncv) {
	   	fprintf(stderr, "ERROR: Device not set for national caveats.\n");
		   remark(MSG_TAPEFMT);
	   }
		

	   /* If the device is single-level for the policy, */
	   if (level_flags & SL_NCAVLABEL) {
			/* If a label is found it is a tape format error. */
			if (ncv) remark(MSG_TAPEFMT);
	   }
	   else { 	/* Device is multi-level, which can only be valid if the
		 	 * the policy is running. 
		 	 */
#if SEC_NCAV /*{*/
	   	if(strcmp(ncv,WildCard) == 0) f_ncav = NULL;
	   	else {
			/* We have some error trapping around the er_to_ir 
			 * conversion to allow a privileged user to reenter 
			 * the label if it cannot be converted by this system.
		 	 */
			done = 1;
			do {
			   f_ncav = ncav_er_to_ir(ncv);
			   if (!f_ncav) 
				done = ie_new_label(MLTAPE_NCAVLABEL,&ncv);
			} while (!f_ncav && !done);
			if (!f_ncav) {
	      			remark(MSG_BADNCAV);
				return(-1);
			}
	   	}
	   	/* 
	    	 * within bounds?  Let symbolic links bypass the check.
	    	 */
	   	if (!((type == IE_FILE) && (extra == IE_SYM_LINK))) {
  			if(!ncav_bounds(f_ncav)) {
	      			remark(MSG_NOTIMPCAV);
				if (f_ncav) free(f_ncav);
	      			return(-1);
	   		}
	   	}
	   	if (f_ncav) free(f_ncav);
#else
	   	fprintf(stderr,"ERROR: System not set for National Caveats\n");
	   	remark(MSG_TAPEFMT);
#endif /*} SEC_NCAV */
	   }
	} /* end of if (!slink) */

	/*
	 * get kid level
	 */
	iskid = 0;
	if((kid != NULL) && (strlen(kid)>0)) {
#if SEC_MAC /*{*/
	   k_ir = mand_er_to_ir(kid);
	   if(!k_ir) {
	      fprintf(stderr, "%s: ", path);
	      remark(MSG_BADKID);
	      return(-1);
	   }

	   /* We found a valid mld child.  If the user did not have
	    * multileveldir, then we will only import files from the
	    * branch that matches the user's level.
	    */
	   if (!hassysauth(SEC_MULTILEVELDIR)) {
	   	if ((mand_ir_relationship(k_ir,proc_ir) & MAND_EQUAL) == 0)
			return(-1);
	   }
	
	   iskid = 1;
#else
	   fprintf(stderr,
	     MSGSTR(ERR_SYS_NOT_CONF_SL, "ERROR: System not configured for sensitivity labels\n"));
	   remark(MSG_TAPEFMT);
#endif /*} SEC_MAC */
	}
#if SEC_MAC
	else
		k_ir = NULL;
#endif

	/*
	 * Get pprivs only if file is not a  symbolic link.
	 */
	if (!slink) {
	   if (tape_format & MLTAPE_PRIVS) {
#if SEC_PRIV /*{*/
		if((type==0) && !extra) {
			if(strtoprivs(ppr, " ", ppriv) != -1) {
				fprintf(stderr, "%s: ", path);
	      			remark(MSG_GETPPRIVS);
	      			return(-1);
	   		}
			if(strtoprivs(ppr, " ", gpriv) != -1) {
				fprintf(stderr, "%s: ", path);
	      			remark(MSG_GETGPRIVS);
	      			return(-1);
	   		}
		}
#else
		fprintf(stderr,
		  MSGSTR(ERR_SYS_NOT_CONF_PRIV, "ERROR: System not configured for file privilege sets\n"));
		remark(MSG_TAPEFMT);
#endif /*} SEC_PRIV */
	   }
	}
	
	/* 
	 * Create file.
	 */
	f = -1;

#if SEC_MAC
	if(iskid) {
	   if(setslabel(k_ir) != 0) {
	      if(k_ir) free(k_ir);		
	      fprintf(stderr, "%s: ", path);
	      remark(MSG_CHGLEV);
	      return(-1);
	   }
	}
	if(k_ir) free(k_ir);
#endif

	/* If the file in question is about to be linked to some other file
	 * return now so we do not create a new copy.
	 */
	if (type == IE_LINK) return(1);

	/* Trusted recovery skips all of the file checking above and comes
	 * directly here to create the file.
	 */
recovery:
	f = ie_openout(path,mode,type,extra);

	if(f == -1) return(0);		/* if error occured go home */
	if(type == IE_FILE && extra != IE_SYM_LINK)
		close(f); 	/* close file for now */

#if SEC_MAC
	if((type==IE_DIR) && (extra==IE_MLD)) enablepriv(SEC_MULTILEVELDIR);
#endif

	/* We now take the external ascii representations of the security
	 * attributes, map them to the local representations and apply them
	 * to the newly create file.  If any of the mappings fail, we remove
	 * the file and print an error message to alert the SSO of a potential
	 * mismatch in the mappings.
	 */
#if SEC_MAC
	if(set_er(path,MLTAPE_SENS_LABEL,lab)) {
		fprintf(stderr, "%s: ", path);
	 	remark(MSG_SETLAB);
		if (type==IE_DIR) rmdir(path);
		else unlink(path);
		return(-1);
	}
#endif
#if SEC_ACL_SWARE
	if(set_er(path,MLTAPE_ACL,acl)) {
		fprintf(stderr, "%s: ", path);
		remark(MSG_SETACL);
		if (type==IE_DIR) rmdir(path);
		else unlink(path);
		return(-1);
	}
#endif
#if SEC_ACL_POSIX
        if(set_er(path,MLTAPE_PACL,pacl)) {
                fprintf(stderr, "%s: ", path);
                remark(MSG_SETACL);
                if (type==IE_DIR) rmdir(path);
                else unlink(path);
                return(-1);
        }
        if (type == IE_DIR) {
                if(set_er(path,MLTAPE_PDACL,pdacl)) {
                        fprintf(stderr, "%s: ", path);
                        remark(MSG_SETACL);
                        rmdir(path);
                        return(-1);
                }
        }
#endif

#if SEC_NCAV
	if(set_er(path,MLTAPE_NCAV,ncv)) {
		fprintf(stderr, "%s: ", path);
		remark(MSG_SETCAV);
		if (type==IE_DIR) rmdir(path);
		else unlink(path);
		return(-1);
	}
#endif
#if SEC_ILB
	if(set_er(path,MLTAPE_INFO_LABEL,ilb)) {
		fprintf(stderr, "%s: ", path);
		remark(MSG_SETILB);
		if (type==IE_DIR) rmdir(path);
		else unlink(path);
		return(-1);
	}
#endif
	/* do other stuff */
	if (!ie_recovery_mode && !(tape_format & DONT_SET_TYPE_FLAGS) &&
	   (tape_format & MLTAPE_TYPE_FLAGS)) do_opts(path,opt);

#if SEC_MAC
	if((type==IE_DIR) && (extra==IE_MLD)) disablepriv(SEC_MULTILEVELDIR);
#endif

	/*
	 * Now re-open the file & return
	 * the file desc.
	 */
	if(type == IE_FILE && extra != IE_SYM_LINK) {
	  	f = open(path,O_WRONLY);
	  	chmod(path,mode);
	  	return(f);
	}
	else return(1);
}

/* ie_unpack()
 *
 * Decode the security attributes of the current file.
 */
ie_unpack()
{
	register char	*buff_ptr = buffer;

	if (tape_format & MLTAPE_TYPE_FLAGS) {
		opt = buff_ptr;
		buff_ptr += strlen(opt) + 1;
		kid = buff_ptr;
		buff_ptr += strlen(kid) + 1;
#ifdef DEBUG
		fprintf(stderr,"opt: %s\n",opt);
		fprintf(stderr,"kid: %s\n",kid);
#endif
	}
	else {
		opt = NULL;
   		kid = "";
	}
	if (tape_format & MLTAPE_SENS_LABEL) {
		lab = buff_ptr;
		buff_ptr += strlen(lab) + 1;
#ifdef DEBUG
		fprintf(stderr,"lab: %s\n",lab);
#endif
	}
	else 	lab = NULL;
	if (tape_format & MLTAPE_ACL) {
		acl = buff_ptr;
		buff_ptr += strlen(acl) + 1;
#ifdef DEBUG
		fprintf(stderr,"acl: %s\n",acl);
#endif
	}
	else 	acl = NULL;
#if SEC_ACL_POSIX
        if (tape_format & MLTAPE_PACL) {
                pacl = buff_ptr;
                buff_ptr += strlen(pacl) + 1;
#ifdef DEBUG
                fprintf(stderr,"pacl: %s\n",pacl);
#endif
        }
        else    pacl = NULL;
        if (tape_format & MLTAPE_PDACL) {
                pdacl = buff_ptr;
                buff_ptr += strlen(pdacl) + 1;
#ifdef DEBUG
                fprintf(stderr,"pdacl: %s\n",pdacl);
#endif
        }
        else    pdacl = NULL;

#endif
	if (tape_format & MLTAPE_NCAV) {
		ncv = buff_ptr;
		buff_ptr += strlen(ncv) + 1;
#ifdef DEBUG
		fprintf(stderr,"ncv: %s\n",ncv);
#endif
	}
	else 	ncv = NULL;
	if (tape_format & MLTAPE_INFO_LABEL) {
		ilb = buff_ptr;
		buff_ptr += strlen(ilb) + 1;
#ifdef DEBUG
		fprintf(stderr,"ilb: %s\n",ilb);
#endif
	}
	else 	ilb = NULL;
	if (tape_format & MLTAPE_PRIVS) {
		ppr = buff_ptr;
		buff_ptr += strlen(ppr) + 1;
		gpr = buff_ptr;
		buff_ptr += strlen(gpr) + 1;
#ifdef DEBUG
		fprintf(stderr,"ppr: %s\n",ppr);
		fprintf(stderr,"gpr: %s\n",gpr);
#endif
	}
	else {
		ppr = NULL;
		gpr = NULL;
	}
}

/* ie_pentry()
 *
 * Print the security attributes on standard output
 */
ie_pentry(ismld)
	short	ismld;
{
	if ((tape_format & MLTAPE_TYPE_FLAGS) && (*opt || *kid || ismld)) {
		if (*opt)
			printf(MSGSTR(OPT_I_E, "\tOpt: %s"), opt);
		if (ismld)
			printf(MSGSTR(MLD_I_E, "\tMld"));
		if (*kid)
			printf(MSGSTR(MLD_BRANCH, "\tMld branch: %s"), kid);
		putchar('\n');
	}
	if ((tape_format & MLTAPE_SENS_LABEL) && *lab)
		printf(MSGSTR(I_E_LABEL, "\tSLABEL: %s\n"), lab);
	if ((tape_format & MLTAPE_INFO_LABEL) && *ilb)
		printf("\tILABEL: %s\n", ilb);
	if ((tape_format & MLTAPE_ACL) && strcmp(acl, WildCard))
		printf(MSGSTR(I_E_ACL, "\tACL:    %s\n"), acl);
	if ((tape_format & MLTAPE_PACL) && strcmp(pacl, WildCard))
                printf("\tPACL:    %s\n", pacl);
        if ((tape_format & MLTAPE_PDACL) && strcmp(pdacl, WildCard))
                printf("\tDEFAULT ACL:    %s\n", pdacl);
	if ((tape_format & MLTAPE_NCAV) && *ncv)
		printf("\tNCAV:   %s\n", ncv);
	if (tape_format & MLTAPE_PRIVS) {
		if (*ppr)
			printf(MSGSTR(I_E_PRIVS, "\tPPRIVS: %s\n"), ppr);
		if (*gpr)
			printf(MSGSTR(I_E_GPRIVS, "\tGPRIVS: %s\n"), gpr);
	}
}

/* ie_ml_export()
 *
 * This procedure returns 1 if we can export the given file.
 * If we are dealing with an mld child, ie_stripmld() will
 * have been called to put us at the correct level
 * to access the file and path will have had the mac..n 
 * removed.
 *
 * If we can export the file, we also build the ascii security info header.
 */
int
ie_ml_export(path,symlink,is_a_directory)
	char *path;
	int symlink,is_a_directory;
{
	int i;
	int size;
	char *s;
	struct namepair *np;
	int l_lab,l_acl,l_pacl, l_pdacl,l_ncv,l_ilb,l_kid,l_ppriv,l_gpriv;
	int l_opt;
	char extended_fs;

	size = 0;				/* buffer size */
	mld_kludge = 0;
	slink = symlink;

#ifdef DEBUG
	fprintf(stderr,"ie_ml_export()\n");
#endif
	   

#if SEC_MAC /*{*/
	/* First check the sensitivity label. */ 
	if ((!slink ? statslabel : lstatslabel)(path,file_ir) == -1) {
	   if(errno != EINVAL) {
#ifdef DEBUG
	      fprintf(stderr,"(failed statslabel)\n");
#endif
	      fprintf(stderr, "%s: ", path);
	      remark(MSG_BADLAB);
	      return(0);
	   }
	   /* Wildcard label, so allow export. */
	}
	else if(!ml_bounds(file_ir)) {	/* check limits, note that this works 
					 * for both single and multi-level 
					 * devices.
					 */
#ifdef DEBUG
		fprintf(stderr,"(failed bounds)\n");
#endif
		
		/* We have failed the bounds test, but if this is a 
		 * multilevel directory and we have multileveldir, we need
		 * to export it anyway.
		 */
		if (!is_a_directory || !ie_ismld(path)) {
			fprintf(stderr, "%s: ", path);
			remark(MSG_NOTEXP);
			return(0);
		}
		/* it is an mld, so force it to the device minimum level */
		else {
			/* One last check, if the SL of the mld dominates the
			 * tape max level then we don't want it */
			if (!mld_bounds (file_ir)) {
				fprintf(stderr, "%s: ", path);
				remark(MSG_NOTEXP);
				return (0);
			}
		   	mld_kludge = 1;
#ifdef DEBUG
		   fprintf(stderr,"but it is an mld so will export anyway\n");
#endif
		}
	}

	/* If this file is the child of an mld, we must check to make sure that
	 * the mld branch can be exported, i.e. if the device is set up from
	 * confidential to syshi, we will not export a syshi file in a syslo
	 * branch of an mld.  The reason being that we cannot generate the
	 * correct child on the other end.
	 */
	if (iskid && !ml_bounds(kid_ir)) {
		fprintf(stderr, "%s: ", path);
		remark(MSG_NOTEXP);
		return(0);
	}

#ifdef DEBUG
	fprintf(stderr,"passed slabel test\n");
#endif
#endif /*} SEC_MAC */

#if SEC_NCAV /*{*/
	/* SYMBOLIC LINK KLUDGE!!!
	 * There is no lstatncav() at this time, so if we are dealing with a
	 * symbolic link, we will bypass ncav checking.
	 */
	if (!slink) {
		/* Check national caveats */
		if(statncav(path,ncv_ir) == -1) {
	   		if(errno != EINVAL) {
#ifdef DEBUG
	      			fprintf(stderr,"(failed statncav)\n");
#endif
				fprintf(stderr, "%s: ", path);
	      			remark(MSG_BADNCAV);
	      			return(0);
	   		}

	   		/* Wildcard label, so allow export. */
		}
		/* check limits, note that this works for both single and 
		 * multi-level devices.
		 */
		else if(!ncav_bounds(ncv_ir)) {
#ifdef DEBUG
	      		fprintf(stderr,"(failed ncav check)\n");
#endif
			fprintf(stderr, "%s: ", path);
	      		remark(MSG_NOTEXPCAV);
	      		return(0);
		}
	}
#else /*}{*/
	/* If we are not configured for single-user, fail. */
	if (!(level_flags & SL_NCAVLABEL)) return(0);
#endif /*} SEC_NCAV */
#ifdef DEBUG
	fprintf(stderr,"passed ncav test\n");
#endif

	
	/********************************************
	 * Build the security part of the file header.
	 ********************************************/
	/* If we are in pass mode, we don't need the header. */
	if (asg_impexp == AUTH_DEV_PASS) return(1);

	l_ppriv = l_gpriv = 1;

#if SEC_PRIV /*{*/
	/*
	 * Get potential & granted priv vectors.
	 */
	
	/* No lstatpriv(), so only get the privs if we are not dealing with
	 * a symbolic link.
	 */
	extended_fs = islabeledfs(path);

	if (!slink && extended_fs) {
		if (statpriv(path, SEC_POTENTIAL_PRIV, ppriv) < 0) {
			fprintf(stderr, "%s: ", path);
			remark(MSG_GETPPRIVS);
			return(0);
		}
	
		if (statpriv(path, SEC_GRANTED_PRIV, gpriv) < 0) {
			fprintf(stderr, "%s: ", path);
	      		remark(MSG_GETGPRIVS);
	      		return(0);
		}

		/* Find size required to hold privs. */
		for(np=sys_priv;np->name != NULL;np++) {
	      		i = np->value;
	      		if(ISBITSET(ppriv,i))
	         		l_ppriv += strlen(np->name) + 1;
	      		if(ISBITSET(gpriv,i))
	         		l_gpriv += strlen(np->name) + 1;
		}
	}
	else {
		for (i=0; i<SEC_SPRIVVEC_SIZE; ++i) {
			ppriv[i] = 0;
			gpriv[i] = 0;
		}
	}
#endif /*} SEC_PRIV */

	/*
	 * This opt string contains other file options,
  	 * currently just 2 person rule.
	 */
	opt = "";
	l_opt = strlen(opt) + 1;

#if SEC_MAC
	/* get_er() malloc()s space. */
	lab = get_er(path,MLTAPE_SENS_LABEL);	/* file level */
	if (lab)
		l_lab = strlen(lab) + 1;
	else
#endif
		l_lab = 1;

#if SEC_ACL_SWARE
	acl = get_er(path,MLTAPE_ACL);		/* acl */
	if (acl)
		l_acl = strlen(acl) + 1;
	else
#endif
		l_acl = 1;
#if SEC_ACL_POSIX
        pacl = get_er(path,MLTAPE_PACL);        /* posix acl */
        if (pacl)
                l_pacl = strlen(pacl) + 1;
        else
#endif
                l_pacl = 1;

#if SEC_ACL_POSIX
        pdacl = get_er(path,MLTAPE_PDACL);      /* posix default acl */
        if (pdacl)
                l_pdacl = strlen(pdacl) + 1;
        else
#endif
                l_pdacl = 1;



#if SEC_NCAV
	ncv = get_er(path,MLTAPE_NCAV);		/* caveat */	
	if (ncv)
		l_ncv = strlen(ncv) + 1;
	else
#endif
		l_ncv = 1;

#if SEC_ILB
	ilb = get_er(path,MLTAPE_INFO_LABEL);	/* info label */
	if (ilb)
		l_ilb = strlen(ilb) + 1;
	else
#endif
		l_ilb = 1;

	/* If we failed getting any of the external representations, do not
	 * export the file.  Free any allocated memory and return failure.
	 */
	i = 0;
	if ((tape_format & MLTAPE_SENS_LABEL) && !lab) {
		if (lab) free(lab);
		else remark(MSG_BADLAB);
		i = 1;
	}
	if ((tape_format & MLTAPE_INFO_LABEL) && !ilb) {
		if (ilb) free(ilb);
		else remark(MSG_BADILB);
		i = 1;
	}
	if ((tape_format & MLTAPE_NCAV) && !ncv) {
		if (ncv) free(ncv);
		else remark(MSG_BADNCAV);
		i = 1;
	}
	if ((tape_format & MLTAPE_ACL) && !acl) {
		if (acl) free(acl);
		else remark(MSG_BADACL);
	}
	if ((tape_format & MLTAPE_PACL) && !pacl) {
                if (pacl) free(pacl);
                else remark(MSG_BADACL);
                i = 1;
        }

	if (i) return(0);

#if SEC_MAC
	/* If the file is an mld child and the tape is multi-level for
	 * sensitivity labels, load in the child level. 
	 */
	if (iskid && !(level_flags & SL_SLABEL))
		kid = mand_ir_to_er(kid_ir);
	else
#endif
		kid = "";
	l_kid = strlen(kid) + 1;

	/*
	 * now allocate buffer.  This may be slightly larger than we will
	 * really use.
	 */
	size = l_opt+l_lab + l_acl +l_pacl + l_pdacl + l_ncv + l_ilb + l_kid + l_ppriv + l_gpriv;
	allocbuff(size);

	/*
	 * Now fill buffer
	 */
	size = 0;
	bufptr = buffer;
#ifdef DEBUG
	fprintf(stderr,"ie_ml_export(): path %s\n",path);
	fprintf(stderr,"buffer contents:\n");
#endif
	if (tape_format & MLTAPE_TYPE_FLAGS) {
		memcpy(bufptr,opt,l_opt);		/* copy in opt   */
		bufptr 	+= l_opt;
		size	+= l_opt;
		memcpy(bufptr,kid,l_kid);
		bufptr 	+= l_kid;
		size	+= l_kid;
#ifdef DEBUG
	fprintf(stderr,"opt: %s\n",opt);
	fprintf(stderr,"kid: %s\n",kid);
#endif
	}
	if (tape_format & MLTAPE_SENS_LABEL) {
		memcpy(bufptr,lab,l_lab);		/* copy in label */
		bufptr 	+= l_lab;			
		size	+= l_lab;
#ifdef DEBUG
	fprintf(stderr,"lab: %s\n",lab);
#endif
	}
	if (tape_format & MLTAPE_ACL) {
		memcpy(bufptr,acl,l_acl);		/* copy in ACL   */
		bufptr 	+= l_acl;
		size	+= l_acl;
#ifdef DEBUG
	fprintf(stderr,"acl: %s\n",acl);
#endif
	}
       if (tape_format & MLTAPE_PACL) {
                memcpy(bufptr,pacl,l_pacl);             /* copy in posix ACL*/
                bufptr  += l_pacl;
                size    += l_pacl;
#ifdef DEBUG
        fprintf(stderr,"pacl: %s\n",pacl);
#endif
        }
        if (tape_format & MLTAPE_PDACL) {
                memcpy(bufptr,pdacl,l_pdacl);           /* copy in pos def ACL*/
                bufptr  += l_pdacl;
                size    += l_pdacl;
#ifdef DEBUG
        fprintf(stderr,"pdacl: %s\n",pdacl);
#endif
	}
	if (tape_format & MLTAPE_NCAV) {
		memcpy(bufptr,ncv,l_ncv);		/* copy in caveat*/
		bufptr 	+= l_ncv;
		size	+= l_ncv;
#ifdef DEBUG
	fprintf(stderr,"ncv: %s\n",ncv);
#endif
	}
	if (tape_format & MLTAPE_INFO_LABEL) {
		memcpy(bufptr,ilb,l_ilb);		/* copy in info lab */
		bufptr 	+= l_ilb;
		size	+= l_ilb;
#ifdef DEBUG
	fprintf(stderr,"ilb: %s\n",ilb);
#endif
	}

	if (lab) free(lab);
	if (acl) free(acl);
        if (pacl) free(pacl);
        if (pdacl) free(pdacl);
	if (ncv) free(ncv);
	if (ilb) free(ilb);

	if (tape_format & MLTAPE_PRIVS) {
#ifdef DEBUG
		fprintf(stderr,"ppr:\n");
#endif

#if SEC_PRIV
		/* Fill in the potential privs. */
		for(np=sys_priv;np->name != NULL;np++) {
	      		if(ISBITSET(ppriv,np->value)) {
	         		i = strlen(np->name);
#ifdef DEBUG
	         		fprintf(stderr,"\t%s\n",np->name);
#endif
	         		memcpy(bufptr,np->name,i);
	         		bufptr += i;
	         		*bufptr++ = ' ';
				size += i+1;
	      		}
		}
#endif
		*bufptr++ = '\0';
		++size;
#ifdef DEBUG
		fprintf(stderr,"gpr:\n");
#endif
#if SEC_PRIV
		/* Fill in the granted privs. */
		for(np=sys_priv;np->name != NULL;np++) {
	      		if(ISBITSET(gpriv,np->value)) {
	         		i = strlen(np->name);
#ifdef DEBUG
	         		fprintf(stderr,"\t%s\n",np->name);
#endif
	         		memcpy(bufptr,np->name,i);
	         		bufptr += i;
	         		*bufptr++ = ' ';
				size += i+1;
	       		}
		}
#endif

		*bufptr++ = '\0';
		++size;
	}

	return(size);
}	


/*
 * This procedure returns the address of the security
 * buffer.
 */
char *
ie_findbuff()
{
	return(buffer);
}

/*
 * This procedure allocates buffer space for reading
 * the security information on import.
 */
char *
ie_allocbuff(size)
	int size;
{
	return(allocbuff(size));
}

#if SEC_MAC /*{*/
/*
 * Check if path is a multileveldir.
 */
int
ie_ismld(path)
	char *path;
{
	int st;

	if(!hassysauth(SEC_MULTILEVELDIR))	/* we can't tell */
	   return(0);

	st = Ismultdir(path);
	return(st==1);
}


/*
 * Make multileveldir.
 */
void
ie_mkmld(path,mode)
	char *path;
	int mode;
{
#ifdef DEBUG
	fprintf(stderr,"*ie_mkmld(): %s\n",path);
#endif
	if(!Ismultdir(path))			/* is it an mld already? */
	   if(Mkmultdir(path) != 0) {		/* no - make it one	 */
	      rmdir(path);			/* can't.		 */
	      remark(MSG_BADMLD);		/* terminate.		 */
	   }
}
#endif /*} SEC_MAC */

/*
 * This next section is concerned with scanning paths for
 * children of multileveldirs.
 */
static char *ndbuff = NULL;
static char *ndptr  = NULL;
static int   ndsave = 0;
/*
 * Return successive parts of pathname.
 */
static char *
nextpart(path)
	char *path;
{
	if(path) {
	  if(ndbuff) free(ndbuff);
	  ndbuff = Strdup(path);
	  ndptr  = ndbuff;
	  ndsave = *ndptr;
	}
	else {
	   if(ndsave == '\0')
	      return(NULL);
	   *ndptr = ndsave;
	   ndptr++;
	}

	while( *ndptr && (*ndptr != '/') )
	   ndptr++;

	ndsave = *ndptr;
	*ndptr = '\0';

	return(ndbuff);
}

#if SEC_MAC /*{*/
/*
 * Check a path for children of mlds.
 * Returns tag (int part of mac....n) or -1.
 * ..kind of "OHMYGOD THE CHILDREN!!!"
 */	
tag_t
ie_ischild(path)
	char *path;
{
	char *this;
	char *p;
	tag_t tag;

	if(!hassysauth(SEC_MULTILEVELDIR))
	   return(-1);

	this = nextpart(path);
	while(this) {
	   if(Ismultdir(this)==1) {		/* say /tmp */
	      if(this=nextpart(NULL)) {	/*     /tmp/mac00000000002 */
	         p = this + strlen(this);	
		 while(*p != '/')
		    p--;			
		 p += 4;			/* 00000000002 */
#ifdef DEBUG
		 fprintf(stderr,"*ie_child(): found child %s\n",p);
#endif
		 return(atoi(p));
	      }
	      else
	         return(-1);
	   }
	}
	return(-1);
} 

/*
 * This procedure checks incoming pathnames
 * as soon as available during export. 
 * Returned:
 *	myname: path should be saved.
 * 	Null - path should be skipped.
 *
 * If path is an mld, set *mld & return 0.
 * If path terminates in an mld child, return 1.
 * If path contains an mld child, then
 *    strip the mac00..n component of the
 *    path, determine the tag value & return 0.
 *
 * The reason we do this is that if the user
 * has SEC_MULTILEVELDIR set during export, we will be seeing
 * the internal hidden mld child directories. These
 * give us the sensitivity level at which we must
 * run during import of the file, but we cannot
 * create the child directory or any path explicitly
 * containing it ( the file ends up in the correct
 * mld child directory because we change the process level
 * on import). Only the kernel can create directories
 * immediately subordinate to mlds.
 */
static char *myname = NULL;
char *
ie_stripmld(path,mld,child)
	char *path;		/* pathname */
	short *mld;		/* 1 -> mld */
	short *child;		/* 1 -> child of mld */
{
	tag_t tagval;
	char *this;
	char *p;
	char *q;

#ifdef DEBUG
	fprintf(stderr,"ie_stripmld: path %s\n",path);
#endif
	/* This kludge is our termination condition if we are using the
	 * mltape -M option to run find(1) within mltape.
	 */
	if (!strcmp(path,END_STRING)) return((char *)-1);

	*mld = 0;		/* assume not mld */
	*child = 0;		/* assume not child */
	iskid = 0; 

	if(myname)  free(myname);

	myname = Strdup(path);
	
	/*
 	 * If we are running w/o secmld, presumably find
	 * (or whatever is supplying the file list) is
	 * running the same way. W/o secmld, we wouldn't
	 * know an mld even if we fell over it.
	 */
	if(!hassysauth(SEC_MULTILEVELDIR))
	   return(myname);
	
	/*
	 * Now see if this is an mld itself.
	 */
	if(Ismultdir(path) == 1) {
#ifdef DEBUG
	   fprintf(stderr,"ie_stripmld: %s is an mld\n",path);
#endif
	   *mld = 1;
	   return(myname);
	}
	
	/*
	 * Now see if any part of path is an mld child.
	 * nextpart("/d1/d2/d3/d4/f") returns:
	 * /d1
	 * and successive calls of nextpart(NULL) return
	 * /d1/d2
	 * /d1/d2/d3...
	 * NULL
	 */
	this = nextpart(path);		/* Reset nextpart.	  */
	while(this) {
	   if(Ismultdir(this)==1) {	/* if component is an mld */
	      this = nextpart(NULL);	/* child must be next     */
	      if(!this)			/* shouldn't happen, but  */
	         return(myname);	/* someone will bitch.    */
	      if(strlen(this) == strlen(myname))
	         return(NULL);		/* child only, so skip    */
	      p = this + strlen(this);	/* end of string	  */
	      while(*p != '/')		/* get "/mac000...n"	  */
	         p--;
	      *child = 1;		/* flag child and get tag */
	      iskid = 1;
	      tagval = (tag_t)atoi(p+4);	/* int for now */
	      /* 
	       * Now shift levels so we can get at it w/o multdir.
	       */
	      if(!mand_tag_to_ir(tagval, kid_ir) || setslabel(kid_ir) < 0) {
	         	remark(MSG_CHGLEV);
			return(NULL);
	      }
#ifdef DEBUG
	      fprintf(stderr,"ie_stripmld: child level (%d) %s\n",
			tagval, mand_ir_to_er(kid_ir));
#endif
	      q = myname+strlen(this);	/* now strip mac000..n    */
	      p = q    - strlen(p);	/* from input pathname    */
	      while(*p++ = *q++);
	      *p = '\0';
#ifdef DEBUG
	      fprintf(stderr,"ie_stripmld: stripped to %s\n",myname);
#endif
	      return(myname);
	   }
	   this = nextpart(NULL);
	}
	return(myname);
}

void
ie_resetlev()
{
	int ret;

#ifdef DEBUG
	fprintf(stderr,"resetting slabel to %x\n",proc_ir);
#endif
	if (hassysauth(SEC_MULTILEVELDIR))
		ret = setslabel(proc_clr);
	else
		ret = setslabel(proc_ir);
	if (ret < 0) remark(MSG_CHGLEV);
}
#endif /*} SEC_MAC */

#if SEC_PRIV /*{*/
int
ie_getprivs(path)
	char *path;
{
	char extended_fs = islabeledfs(path);

	if (extended_fs) {
		if (statpriv(path, SEC_POTENTIAL_PRIV, ppriv) < 0) {
	   		remark(MSG_GETPPRIVS);
	   		return(-1);
		}
		if (statpriv(path, SEC_GRANTED_PRIV, gpriv) < 0) {
	   		remark(MSG_GETGPRIVS);
	   		return(-1);
		}
	}
	else {
		int i;

		for (i = 0; i < SEC_SPRIVVEC_SIZE; i++) {
			ppriv[i] = 0;
			gpriv[i] = 0;
		}
		return(0);
	}
			
	return(0);
}


int
ie_setprivs(path)
	char *path;
{
	/* If we don't want to set the privs, return without error. */
	if ((tape_format & DONT_SET_PRIVS) || !(tape_format & MLTAPE_PRIVS)) 
		return(0);

	/* If the user doesn't have CHPRIV, it is not an error, just return
	 * without actually setting the privs.
	 */
	if (!hassysauth(SEC_CHPRIV)) return(0);

	if (chpriv(path, SEC_POTENTIAL_PRIV, ppriv) < 0) {
#ifdef DEBUG
	   fprintf(stderr,"path = %s\nunable to set potential privs = %x\n",
			path,ppriv[0]);
#endif
	   remark(MSG_SETPPRIVS);
	   return(-1);
	}
	if (chpriv(path, SEC_GRANTED_PRIV, gpriv) < 0) {
#ifdef DEBUG
	   fprintf(stderr,"path = %s\nunable to set granted privs = %x\n",
			path,gpriv[0]);
#endif
	   remark(MSG_SETGPRIVS);
	   return(-1);
	}
	return(0);
}
#endif /*} SEC_PRIV */

/* ie_copy_attr()
 *
 * This procedure copies security attributes from src to dst.  It is used
 * by mltape in the pass mode.
 */ 
void
ie_copyattr(src,dst)
	char *src;			/* source file */
	char *dst;			/* destination file */
{
#if SEC_ACL_SWARE
	int n_acle;
	acle_t *acl_ir;
#endif
#if SEC_ACL_POSIX
        int n_pacle, n_paclew;
        acl_t *pacl_ir;
        int n_pdacle, n_pdaclew;
        acl_t *pdacl_ir;
#endif
	struct stat sb;
#if SEC_MAC
	int mld;
#endif
	int i;

#if SEC_MAC
	if(Ismultdir(src)) {			/* need this to see real */
	   mld = 1;				/* attributes of mld     */
	   enablepriv(SEC_MULTILEVELDIR);
	}
#endif

	if(stat(src,&sb)) {
	   remark(MSG_COPYATTR);
#if SEC_MAC
	   goto rm_mld;
#else
	   return;
#endif
	}

#if SEC_MAC
	/*
	 * destinations ending in '/.' will cause chslabel
	 * to fail.
	 */
	i = strlen(dst) - 1;
	if (((i==0) && (*dst == '.')) ||
	    ((i>0)  && (*(dst+i) == '.') && (*(dst+i-1) == '/'))) {
	   fprintf(stderr,MSGSTR(CANT_SET_SL, "%s: cannot set sensitivity label\n"),dst);
	   goto rm_mld;
	}
#endif

#if SEC_ACL_SWARE
	/* To avoid MAC access, writedown problems, we set all attributes
	 * before changing the sensitivity label.
	 */

	/* Copy the ACL */
	if (tape_format & MLTAPE_ACL) {
	   /* Do not copy the ACL if the ie_omit_acls flag is set and
	    * the user has the ALLOWDACACCESS kernel authorization.
	    */
	   if (!ie_omit_acls || !hassysauth(SEC_ALLOWDACACCESS)) {
		n_acle = statacl(src,NULL,0);		/* ACL */
		if(n_acle>0) {
	   		acl_ir = (acle_t *)m_alloc(n_acle * sizeof(acle_t));
	   		statacl(src,acl_ir,n_acle);
			if (chacl(dst,acl_ir,n_acle)) remark(MSG_SETACL);
	   		free(acl_ir);
		}
		else if (chacl(dst,ACL_DELETE)) remark(MSG_SETACL);
	   }
	}
#endif
#if SEC_ACL_POSIX
        /* To avoid MAC access, writedown problems, we set all attributes
         * before changing the sensitivity label.
         */

        /* Copy the access ACL */
        if (tape_format & MLTAPE_PACL) {
           /* Do not copy the ACL if the ie_omit_acls flag is set and
            * the user has the ALLOWDACACCESS kernel authorization.
            */
           if (!ie_omit_acls || !hassysauth(SEC_ALLOWDACACCESS)) {
                /* Allocate inital space for ACL */
                if (acl_alloc (&pacl_ir)== 0)  {
                        n_pacle =acl_read(src,ACL_TYPE_ACCESS,pacl_ir);
                        if (n_pacle > 3) {
                                n_paclew=acl_write(dst,ACL_TYPE_ACCESS,pacl_ir);
                                if (n_paclew < 0) remark(MSG_SETACL);
                        }
                        else if (n_pacle < 0) remark(MSG_SETACL);

                        acl_free(pacl_ir);
                }
                else remark(MSG_NOMEM);
           }
        }



        /* Copy the default ACL */

        if (tape_format & MLTAPE_PDACL) {
           /* Do not copy the ACL if the ie_omit_acls flag is set and
            * the user has the ALLOWDACACCESS kernel authorization.
            */
           if (!ie_omit_acls || !hassysauth(SEC_ALLOWDACACCESS)) {
                if ((sb.st_mode & S_IFMT) == S_IFDIR) {
                        /* Allocate inital space for ACL */
                        if (acl_alloc (&pdacl_ir)==0 ) {
                              n_pdacle=acl_read(src,ACL_TYPE_DEFAULT,pdacl_ir);
                                if (n_pdacle > 0) {
                                     n_pdaclew=acl_write(dst,ACL_TYPE_DEFAULT,pdacl_ir);
                                        if (n_pdaclew< 0) remark(MSG_SETACL);
                                        acl_free(pdacl_ir);
                                }
                                else if (n_pdacle<0) remark(MSG_SETACL);
                                acl_free(pdacl_ir);
                        }
                        else remark(MSG_NOMEM);
                }
           }
        }
#endif


#if SEC_PRIV
	/* Copy the privilege sets. The check for the chpriv kernel
	 * authorization is made in ie_setprivs()
	 */
	if ((tape_format&MLTAPE_PRIVS) && hassysauth(SEC_CHPRIV)) {
		if((sb.st_mode & S_IFMT) == S_IFREG) {
	   		if( ie_getprivs(src) || ie_setprivs(dst))
	     			remark(MSG_COPYATTR);
		}
	}
#endif

#if SEC_NCAV
	if (tape_format & MLTAPE_NCAV) {
		if(statncav(src,ncv_ir)==-1) {
	   		if (errno==EINVAL) {
	      			if (chncav(dst,NULL)) remark(MSG_SETCAV);
	   		}
	   		else remark(MSG_SETCAV);
		}
		else if(chncav(dst,ncv_ir)) remark(MSG_SETCAV);
	}
#endif


#if SEC_MAC
	if (tape_format & MLTAPE_SENS_LABEL) {
		if(statslabel(src,file_ir) == -1) {	/* label */
	   		if(errno==EINVAL) {
	      			if(chslabel(dst,NULL)) remark(MSG_SETLAB);
	   		}
	   		else remark(MSG_SETLAB);
		}
		else {
	   		if(chslabel(dst,file_ir)) {
	      			fprintf(stderr,MSGSTR(I_E_FILE, "file %s: "),dst);
	      			remark(MSG_SETLAB);
	   		}
		}
	}
#endif

#if SEC_ILB
	if (tape_format & MLTAPE_INFO_LABEL) {
		if (statilabel(src,ilb_ir)==-1) {
	   		if (errno==EINVAL) {
	      			if (chilabel(dst,NULL)) remark(MSG_SETILB);
	   		}
	   		else remark(MSG_SETILB);
		}
		else if (chilabel(dst,ilb_ir)) remark(MSG_SETILB);
	}
#endif

#if SEC_MAC
rm_mld:
	if(mld)
	   disablepriv(SEC_MULTILEVELDIR);
#endif
}
	
/* get_er(
 *	pathname of file to be exported.
 *	code signifying which label to convert)
 *
 * This procedure returns the external form of the security attribute 
 * described by attr.
 *
 * Returns:
 * 	NULL		error occured
 *	WildCard	wildcard 
 *	""		not an error, but not able to get label.  Usually
 *				do to a symbolic link.
 * 	else er.
 */
static char *
get_er(path,attr)
	char *path;			/* file */
	int attr;			/* attr */
{
	char *e;
#if SEC_MAC
	int mld;
#endif
#if SEC_ACL_SWARE
	int n_acle;
	acle_t *acl_ir;
#endif
#if SEC_ACL_POSIX
        struct stat sb;
        int n_pacle, n_pdacle;
        acl_t *pacl_ir;
        acl_t *pdacl_ir;
        char *acl_buffer = NULL;
#endif



#if SEC_MAC
	/* If the object is an mld, turn on multileveldir so we can access it */
	mld = 0;
	if(Ismultdir(path)) {
	   mld = 1;
	   enablepriv(SEC_MULTILEVELDIR);
	}
#endif

	switch (attr) {
#if SEC_MAC
	   case MLTAPE_SENS_LABEL:		/* Sensitivity label. */
		if (level_flags & SL_SLABEL) {
			/* Single-level device.  We won't actually be exporting
			 * this label.
		 	 */
			e = NULL;
		}
		else {	/* Multi-level device. */
			/* If file fits within range specified for the
			 * device, the file keeps its label.
			 */
		        if ((!slink?statslabel:lstatslabel)(path,mand_ir)==-1) {
	   			if(errno==EINVAL) e = WildCard;
	   			else e = NULL;
		    	}
		    	else e = mand_ir_to_er(mand_ir);
		}
		break;
#endif
#if SEC_ACL_SWARE
	   case MLTAPE_ACL:
		/* No lstatacl(), so only get the acl if not a symbolic link. */
		if (!slink) {
	                n_acle = statacl(path,NULL,0);
		        if(n_acle>0) {
	                   acl_ir = (acle_t *)m_alloc(n_acle * sizeof(acle_t));
	   		   statacl(path,acl_ir,n_acle);
	                   e = acl_ir_to_er(acl_ir,n_acle);
			}
			else e = WildCard;
		}
		else
			e = "";
		break;
#endif
#if SEC_ACL_POSIX
           case MLTAPE_PACL:
                if (!slink) {
                        if (acl_alloc(&pacl_ir) == 0) {
                                n_pacle =acl_read(path,ACL_TYPE_ACCESS,pacl_ir);
                                if (n_pacle > 3) {

                                        acl_buffer = (char *)malloc(8192);
    					if (acl_pack(pacl_ir,acl_buffer,8192,ACL_TEXT_PACKAGE) < 0)
                                           e = NULL;
                                        e = acl_buffer;
                                }
                                else if (n_pacle < 0) e = NULL;
                                else e = WildCard;
                                acl_free(pacl_ir);
                       }
                        else e = NULL;
                }
                else
                        e = "";
                break;
           case MLTAPE_PDACL:
                if (!slink) {

                        if (stat(path, &sb) != 0 ||
                           (sb.st_mode & S_IFMT) != S_IFDIR)  {
                                e = WildCard;
                                break;
                        }
                        if (acl_alloc(&pdacl_ir) == 0) {
            			n_pdacle =acl_read(path,ACL_TYPE_DEFAULT,pdacl_ir);
                                if (n_pdacle > 0) {
                                        if (acl_buffer == NULL)
                                           acl_buffer = (char *)malloc(8192);
                    		if (acl_pack(pdacl_ir,acl_buffer,8192,ACL_TEXT_PACKAGE) < 0)
                                           e = NULL;
                                        e = acl_buffer;
                                }
                                else if (n_pdacle < 0) e = NULL;
                                else e = WildCard;
                                acl_free(pdacl_ir);
                        }
                        else e = NULL;
                }
                else
                        e = "";
                break;

#endif

#if SEC_NCAV
	   case MLTAPE_NCAV:
		/* No lstatncav(), so only get if not a symbolic link. */
		if (!slink) {
		   if (level_flags & SL_NCAVLABEL) {
			/* Single-level device.  We won't be exporting this
			 * label.
		 	 */
			e = NULL;
		   }
		   else {	/* Multi-level device. */
			/* If file fits within range specified for the
			 * device, the file keeps its label.
			 */
			if(statncav(path,ncv_ir) == -1) {
	           		if(errno==EINVAL) e = WildCard;
		   		else e = NULL;
			}
			else e = ncav_ir_to_er(ncv_ir);
		   }
		}
		else
			e = "";
		break;
#endif
#if SEC_ILB
	   case MLTAPE_INFO_LABEL:
		if (level_flags & SL_ILABEL) {
			/* Single-level device.  We won't be exporting this
			 * label.
		 	 */
			e = NULL;
		}
		else {	/* Multi-level, send out the real label. */
		   if ((!slink ? statilabel : lstatilabel)(path,ilb_ir) == -1) {
		          if(errno==EINVAL) e = WildCard;
			  else e = NULL;
		   }
		   else e = ilb_ir_to_er(ilb_ir);
		}
		break;
#endif
	   default: e = NULL;
	}
#if SEC_MAC
	if(mld) disablepriv(SEC_MULTILEVELDIR);
#endif
	return( e ? Strdup(e) : NULL);
}



/* ie_new_label(
 *	attribute type,
 *	char **external representation);
 *
 * The system cannot convert the external representation found on the tape.
 * If the user is privileged, ask for a new er.  If not privileged, return
 * with no new er.
 *
 * Returns 0 - OK, new er loaded.
 *	   1 - finished.
 */
char *input_buffer = NULL;
char *acl_buff     = NULL;
char *sl_buff      = NULL;
char *il_buff      = NULL;
char *ncav_buff    = NULL;
char *pacl_buff    = NULL;
char *pdacl_buff    = NULL;
char *cnverr = "ERROR: Converting input file ";

int
ie_new_label(attr,er)
	int attr;
	char **er;
{
	int i;
	char buff[256];

	if (recover_fd == -1) return(1);

	switch (attr) {
#if SEC_MAC
		case MLTAPE_SENS_LABEL:
			if (!hassysauth(SEC_ALLOWMACACCESS)) return(1);
			input_buffer = sl_buff;
			sprintf(buff,MSGSTR(ERR_CONV_IF_SL, "ERROR: Converting input file Sensitivity Label = ")); 
			break;
#endif
#if SEC_ILB
		case MLTAPE_INFO_LABEL:
			if (!hassysauth(SEC_ALLOWILBACCESS)) return(1);
			input_buffer = il_buff;
			sprintf(buff,"ERROR: Converting input file Information Label = "); 
			break;
#endif
#if SEC_ACL_SWARE
		case MLTAPE_ACL:
			if (!hassysauth(SEC_ALLOWDACACCESS)) return(1);
			input_buffer = acl_buff;
			sprintf(buff,MSGSTR(ERR_CONV_IF_ACL, "ERROR: Converting input file ACL List = ")); 
			break;
#endif
#if SEC_ACL_POSIX
                case MLTAPE_PACL:
                        if (!hassysauth(SEC_ALLOWDACACCESS)) return(1);
                        input_buffer = pacl_buff;
                        sprintf(buff,"%sACL List = ",cnverr);
                        break;
                case MLTAPE_PDACL:
                        if (!hassysauth(SEC_ALLOWDACACCESS)) return(1);
                        input_buffer = pdacl_buff;
                        sprintf(buff,"%sDefault ACL List = ",cnverr);
                        break;
#endif
#if SEC_NCAV
		case MLTAPE_NCAV:
			if (!hassysauth(SEC_ALLOWNCAVACCESS)) return(1);
			input_buffer = ncav_buff;
			sprintf(buff, "ERROR: Converting input file Nat. Caveat set = "); 
			break;
#endif
		default: return(1);
	}
	strcat(buff,*er);
	strcat(buff,"\n\n");
	write(recover_fd,buff,strlen(buff));

	/* Get a buffer for the input string if it hasn't already been
	 * allocated.
	 */
	if (input_buffer == NULL) input_buffer = (char *)malloc(256);
	if (input_buffer == NULL) return(1);

	fprintf(stderr,MSGSTR(ENTER_CORRECT_LBL, "Please enter a correct label -> "));

	i = read(recover_fd, input_buffer, sizeof input_buffer);
	if (i <= 0)
		return(1);
	if (input_buffer[i - 1] == '\n')
		input_buffer[i - 1] = '\0';

	if (input_buffer[0]) {
		*er = input_buffer;
		return(0);
	}
	return(1);
}


/* set_er(
 *	pathname of file being imported,
 *	code for the label to be set,
 *	pointer to the external representation for the security attributes
 *			as found on the device
 *	
 * Returns 0 - if set OK
 *     non 0 - if error occurred.
 */
static int
set_er(path,attr,e)
	char *path;			/* file */
	int attr;			/* attr */
	char *e;			/* external form */
{
	int st;
	int done;
#if SEC_MAC
	mand_ir_t *m_ir;
#endif
#if SEC_ACL_SWARE
	acle_t  *a_ir;
	int n_acle;
#endif
#if SEC_ACL_POSIX
        acl_t *pacl_ir;
        acl_t *pdacl_ir;
        struct stat sb;
#endif

#if SEC_NCAV
	ncav_ir_t *n_ir;
#endif
#if SEC_ILB
	ilb_ir_t  *i_ir;
#endif
	int (*chfunc)();

	st = -1;	/* Assume error. */

#if SEC_MAC
	/* Do not try to set the attributes for the current directory. */
	if((attr==MLTAPE_SENS_LABEL) && !strcmp(path,".")) return(0);
#endif

	/* Switch on the type of security label to be set. */
	switch(attr) {
#if SEC_MAC
		case MLTAPE_SENS_LABEL:		/* Sensitivity label. */
		   if (slink)	chfunc = lchslabel;
		   else		chfunc = chslabel;
		   if (!ie_recovery_mode) {	/* If NOT trusted recovery */
			if (level_flags & SL_SLABEL) {
#ifdef DEBUG
				fprintf(stderr,"Sens. label single-level\n");
#endif
				/* Force label to the device label. */
 				st=(*chfunc)(path,tape_ir);
			}
			else {	/* Multi-level device. */
#ifdef DEBUG
			   	fprintf(stderr,"Sens. label multi-level\n");
#endif
			  	if (strcmp(e,WildCard)==0) m_ir = NULL;
			   	else if ((m_ir=mand_er_to_ir(e))==NULL) break;

				if (mld_kludge) {
				   /* Some sort of problem importing an
				    * mld.  If we have multileveldir, then
				    * we need to create the mld, but
				    * force it to tape_min_ir.  If we
				    * don't, then we created a normal
				    * directory and we force it to
				    * proc_ir.
				    */
				   if (hassysauth(SEC_MULTILEVELDIR))
					st = (*chfunc)(path,m_ir);
				   else st = (*chfunc)(path,proc_ir);
				}
				else {
			   		st = (*chfunc)(path,m_ir);
			   		if (m_ir) free(m_ir);	
				}
			}
		   }
		   else { 	/* Trusted recovery, set label to wildcard. */
			(*chfunc)(path,(mand_ir_t *)0);
		   }
		   break;
#endif
#if SEC_ACL_SWARE
		case MLTAPE_ACL:
		   if (tape_format & DONT_SET_ACL) return(0);
		   /* don't set on a symbolic link. */
		   if (!slink) {
		      if (!ie_recovery_mode) {	/* If not trusted recovery. */
			if (tape_format & MLTAPE_ACL) {
			   n_acle = 0;
			   /* We do some error trapping around the er_to_ir
			    * conversion.  If the user is privileged, we
			    * allow them to enter a new er if the first cannot
			    * be converted on this system.  We do the error
			    * trap here, unlike the other labels, for it is
			    * not checked first in ie_ml_import().
			    */
			   done = 1;
			   if (strcmp(e,WildCard)==0) a_ir = ACL_DELETE;
			   else do {
				a_ir = acl_er_to_ir(e,&n_acle);
				if (!a_ir) done = ie_new_label(attr,&e);
			   } while (!a_ir && !done);
			   if (a_ir == NULL) break;
		           st = chacl(path,a_ir,n_acle);
			}
			else st = 0;
		      }
		      else {	/* Trusted recovery. */
			chacl(path,ACL_DELETE,0);	/* Set Wildcard. */
		      }
		   }
		   else st = 0;
		   break;
#endif
#if SEC_ACL_POSIX
		case MLTAPE_PACL:
		   if (tape_format & DONT_SET_ACL) return(0);
		   /* don't set on a symbolic link. */
		   if (!slink) {
		      if (!ie_recovery_mode) {	/* If not trusted recovery. */
			if (tape_format & MLTAPE_PACL) {
			   /* We do some error trapping around the er_to_ir
			    * conversion.  If the user is privileged, we
			    * allow them to enter a new er if the first cannot
			    * be converted on this system.  We do the error
			    * trap here, unlike the other labels, for it is
			    * not checked first in ie_ml_import().
			    */
			   done = 1;
			   if (strcmp(e,WildCard)==0) {
				st = 0;
				break;
		   	   }	
			   else do {
				if (acl_alloc(&pacl_ir)) break;
				if (acl_unpack(e,ACL_TEXT_PACKAGE,pacl_ir)) {
					done = ie_new_label(attr,&e);
				}
			   } while (!pacl_ir && !done);
			   if (pacl_ir == NULL) break;
		           st = acl_write(path,ACL_TYPE_ACCESS,pacl_ir);
		           acl_free(pacl_ir);
			}
			else st = 0;
		      }
		      else st = 0;
		   }
		   else st = 0;
		   break;

		case MLTAPE_PDACL:
		   if (tape_format & DONT_SET_ACL) return(0);
		   /* don't set on a symbolic link. */
		   if (!slink) {
		      if (!ie_recovery_mode) {	/* If not trusted recovery. */
			if (tape_format & MLTAPE_PDACL) {
			   /* We do some error trapping around the er_to_ir
			    * conversion.  If the user is privileged, we
			    * allow them to enter a new er if the first cannot
			    * be converted on this system.  We do the error
			    * trap here, unlike the other labels, for it is
			    * not checked first in ie_ml_import().
			    */
			   done = 1;
			   if (strcmp(e,WildCard)==0) {
				st = 0;
				break;
		   	   }	
			   else do {
				if (acl_alloc(&pdacl_ir)) break;
				if (acl_unpack(e,ACL_TEXT_PACKAGE,pdacl_ir)) {
					done = ie_new_label(attr,&e);
				}
			   } while (!pdacl_ir && !done);
			   if (pdacl_ir == NULL) break;
		           st = acl_write(path,ACL_TYPE_DEFAULT,pdacl_ir);
		   	   acl_free(pdacl_ir);
			}
			else st = 0;
		      }
		      else st = 0;
		   }
		   else st = 0;
		   break;
#endif
#if SEC_NCAV
		case MLTAPE_NCAV:
		   /* Only set if not a symbolic link. */
		   if (!slink) {
		      if (!ie_recovery_mode) {	/* If not trusted recovery. */
			if (level_flags & SL_NCAVLABEL) {
#ifdef DEBUG
				fprintf(stderr,"NCAVs, single-level\n");
#endif
				/* Force label to the device label. */
				st = chncav(path,tape_ncav);
			}
			else {	/* Multi-level device. */
#ifdef DEBUG
				fprintf(stderr,"NCAVs, multi-level\n");
#endif
				if (strcmp(e,WildCard)==0) n_ir = NULL;
				else if ((n_ir=ncav_er_to_ir(e))==NULL) break;

				st = chncav(path,n_ir);
				if(n_ir) free(n_ir);	
			}
		      }
		      else {	/* Trusted recovery, set label to wildcard  */
			chncav(path,(ncav_ir_t *)0);
		      }
		   }
		   else st = 0;
		   break;
#endif
#if SEC_ILB
		case MLTAPE_INFO_LABEL:
		   if (slink)	chfunc = lchilabel;
		   else		chfunc = chilabel;
		   if (!ie_recovery_mode) {
			if (level_flags & SL_ILABEL) {
#ifdef DEBUG
			fprintf(stderr,"Info. labels, single-level\n");
#endif
				/* Single-level device. */
				st=(*chfunc)(path,tape_il);
			}
			else { /* Multi-level device. */
#ifdef DEBUG
			   fprintf(stderr,"Info. labels, multi-level\n");
#endif
			   /* We do some error trapping around the er_to_ir
			    * conversion.  If the user is privileged, we
			    * allow them to enter a new er if the first cannot
			    * be converted on this system.  We do the error
			    * trap here, unlike the other labels, for it is
			    * not checked first in ie_ml_import().
			    */
			   done = 1;
			   if(strcmp(e,WildCard)==0) i_ir = NULL;
			   else do {
				i_ir = ilb_er_to_ir(e);
				if (!i_ir) done = ie_new_label(attr,&e);
			   } while (!i_ir && !done);

			   st = (*chfunc)(path,i_ir);
			   if(i_ir) free(i_ir);	
			}
		   }
		   else {	/* Trusted recovery, set label to wildcard. */
			(*chfunc)(path,(ilb_ir_t *)0);
		   }
		   break;
#endif
	default: 
		   st = 0;
		   break;
	}
	if (ie_recovery_mode)
		return(0);	/* Always pass if in trusted recovery so we
				 * can recover as much as possible.
				 */
	return(st);
}


static char *
m_alloc(size)
	int size;
{
	char *b;

	/* For Aux, we get segmentation errors for malloc/frees of 4 and
	 * fewer bytes. 
	 */
	if (size < 8) size=8;
	if(b = (char *)malloc(size)) return(b);
	remark(MSG_NOMEM);
}
#if defined(SYMLINKS) || defined(_OSF_SOURCE)
/*
 * This procedure creates symbolic links.
 * They are created at the current sens. level.
 * We cannot currently do much to a link
 * after it's created; we are stuck with
 * using the current level so that links in
 * mld trees go into the correct child dir.
 */
ie_symlink(linkname,fullname)
	char *linkname;
	char *fullname;
{
	int st;

	st = symlink(linkname,fullname);
        return(st);
}
#endif

/*
 * This guy interprets the opt string for the given file.
 */
static void
do_opts(path,ostr)
	char *path;
	char *ostr;
{
	char cc;

#ifdef DEBUG
	fprintf(stderr,"do_opts(): option string %s for file %s\n",ostr,path);
#endif
	while(cc = *ostr++)
	   switch(cc) {
		case '2':				/* 2 person rule */
				break;
		default:
				break;
	}
}



/* ie_sl_set_attributes(
 *	char *filename)
 *
 * This routine sets the default security labels for a file imported through
 * the cpio and tar single-level devices.
 */
int
ie_sl_set_attributes(path) 
	char *path;
{
#if SEC_MAC
if (!ie_recovery_mode) {
	/* The device must be set up for single-level sensitivity labels. */
	if (!(level_flags & SL_SLABEL)) remark(MSG_SLSLABEL);

	/* If the system is running single-level, there is nothing to do here,
	 * for the process is running at the level of the device and the file
	 * that we created is at the correct level.
	 */
}
else lchslabel(path,(mand_ir_t *)0);
#endif

#if SEC_ILB
if (!ie_recovery_mode) {
	/* The device must be set up for single-level information labels. */
	if (!(level_flags & SL_ILABEL)) remark(MSG_SLILB);

	/* Set the information label. */
	if (lchilabel(path,tape_il)) return(0);
}
else lchilabel(path,(ilb_ir_t *)0);
#endif

#if SEC_NCAV
if (!ie_recovery_mode) {
	/* The device must be set up for single-level national caveats. */
	if (!(level_flags & SL_NCAVLABEL)) remark(MSG_SLNCAV);

	/* Set the information label. */
	if (chncav(path,tape_ncav)) return(0);
}
else chncav(path,(ncav_ir_t *)0);
#endif

	return(1);	/* Everything was successful. */
}



/* ie_recovery()
 *
 * This routine decides whether or not it is OK to allow the user to proceed
 * with trusted recovery of files.  This is a potentially large security hole,
 * so the checks are pretty stringent.
 *
 * Trusted recovery of files is to be used only when the system has crashed
 * severely enough to corrupt the security policies.  This will allow the
 * system administrator to restore the security databases, daemons, drivers,
 * etc..
 *
 * The trusted recovery mechanism ignores the security information associated
 * with each file in the input volume, and restores the file with all policy
 * tags set to the wildcard.
 */
static int
ie_recovery(dev)
	char *dev;
{
	char *path;
	char **alias;
	struct stat sb;
	struct dev_asg *d;
	int i;
	privvec_t	needed_privs;

	setprivvec(needed_privs, SEC_ALLOWDACACCESS, SEC_CHOWN, SEC_CHMODSUGID
				, SEC_MKNOD, SEC_SHUTDOWN, SEC_OWNER
#if SEC_MAC
				, SEC_ALLOWMACACCESS
#endif
#if SEC_PRIV
	    			, SEC_CHPRIV
#endif
#if SEC_ILB
				, SEC_ALLOWILBACCESS, SEC_ILNOFLOAT
#endif
#if SEC_NCAV
				, SEC_ALLOWNCAVACCESS
#endif
				, -1);

	/* Is the user authorized for trusted recovery?  We already checked 
	 * for the "tape" command authorization in ie_init().  We will now
	 * make sure that the user has a formidable set of privileges.
	 */
	if (checksysauths(needed_privs) != (priv_t *) 0)
		remark(MSG_RECPRVS);

#ifdef DEBUG
		fprintf(stderr,"Authorized for trusted recovery\n");
#endif

	recover_fd = open("/dev/tty", 2);

	/* Force the privileges that we checked and add them to the list of
	 * original privileges
	 */
	forceprivs(needed_privs, (priv_t *) 0);
	for (i = 0; i < SEC_SPRIVVEC_SIZE; i++)
		original_privs[i] |= needed_privs[i];

	/* Does the device assignment database file exist?  If not, trust
	 * the device.  If it does, there must be an entry for this device.
	 * We will ignore its settings, but it must exist.
	 */
	path = find_auth_file((char *)0, OT_DEV_ASG);	/* This routine is
							 * in libprot/authcap.c
	 						 */
	if (!stat(path,&sb)) {	
		/* File exists, so look for an entry matching the device. */
		while( (d = getdvagent()) != NULL) {
   			if(d->uflg.fg_devs) {
      				alias = d->ufld.fd_devs;
      				while(*alias != NULL) {
         				if(strcmp(dev,*alias) == 0) break;
	 				alias++; 
      				}
       			}
   			if(*alias != NULL) {
				break;
			}
		}
		if (d == NULL) remark(MSG_DEVDB);
	}
	else if (errno != EEXIST) remark(MSG_DEVSTAT);

	return(1);
}



int
ie_can_make_node()
{
	return(hassysauth(SEC_MKNOD));
}


int 
ie_chmod(path,mode,type)
char *path;
{
	int ret;

	if ((mode & (S_ISUID | S_ISGID)) && 
	    (!hassysauth(SEC_CHMODSUGID))) {
		if (type==1) rmdir(path);
		else unlink(path);
		return(-1);
	}
	else {
		if (!hassysauth(SEC_OWNER)) disablepriv(SEC_OWNER);
		ret = chmod(path,mode);
		if (running_mltape) forcepriv(SEC_OWNER);
	}
	return (ret);
}


/* ie_mkdir()
 *
 * This routine creates a needed intermediate directory at the lowest possible
 * device level so that all files allowed for import can be created within it.
 */
int
ie_mkdir(namep)
	char *namep;
{
	int ret = 0;		/* Assume OK */
#if SEC_MAC
	mand_ir_t *oldir;

	/* If we are mltape then we do not create any intermediate directories
	 * because we do not know the correct security attributes. We allow
	 * cpio to create them to support backwards compatability, the dirs
	 * we be created at the SL of the single-level device */
	if (running_mltape)
		return (1);

	/* Save our current level. */
	if ((oldir = mand_alloc_ir()) == 0) return(1);
	if (getslabel(oldir)) {
		free(oldir);
		return(1);
	}

	/* Lower our level to tape_min_ir */
	if (setslabel(tape_min_ir)) {
		remark(MSG_CHGLEV);
		ret = 1;
	}
	else {
#endif
		/* make the directory. */
        	if (mkdir(namep, 0777) < 0)
			if (errno != EEXIST) {
				fprintf(stderr, MSGSTR(I_E_MKDIR, "mkdir: "));
				perror(namep);
				ret = 1;
			}
#if SEC_MAC

		/* restore our original level. */
		if (setslabel(oldir)) remark(MSG_CHGLEV);
	}
	
	free(oldir);
#endif
        return(ret);
}


/* pr_intermediate()
 *
 * scan a pathname printing any intermediate directories to the standard
 * input of the main mltape process.
 */
static 
void pr_intermediates(path)
	char *path;
{
	int i;
	char pathname[256], *pos;
	char directory[256];

	if ((path[0] == '.') && (path[1] == '/')) path += 2;
	strncpy(pathname,path,255);
	for (i = strlen(pathname) - 1;
		(i > 0) && (pathname[i] != '/'); --i);
	if (i > 0) {
		pathname[i] = 0;
		pos = pathname;
		while (*pos == '/') ++pos;
		while (*pos) {
			while (*pos && (*pos != '/')) ++pos;
			if (*pos) {
				*pos = 0;
				printf("%s\n",pathname);
				*pos++ = '/';
			}
		}
		printf("%s\n",pathname);
	}
	fflush(stdout);
}


/* ie_mlfind(args)
 *
 * This routine forks off a process that first raises its base privilege
 * set, if authorized, for allowmacaccess and multileveldir, and then executes
 * a captive version of find to generate a file list for us based upon the
 * command line entered by the user.
 */
void
ie_ml_find(args)
	char *args;
{
	int i,fd[2];
	int have_print;
	char pathname[256];
	char *end,*pos,ch;
	privvec_t basep;

	/* First scan the argument string for find(1) to make sure that it
	 * is valid for mltape.  We do not allow the -cpio, -exec or
	 * -depth options, and there must be a -print option.  We also
	 * forbid the use of ';', '>', '<' and '|' in the string.
	 */
	have_print = 0;
	pos = args;
	while (*pos) {
		while (*pos && (*pos != '-')) {
		   switch (*pos) {
			case ';':
			case '>':
			case '<':
			case '|':
			   fprintf(stderr,MSGSTR(ILLEGAL_SYNTAX, "find: illegal syntax for mltape\n"));
			   exit(1);
		   }
		   ++pos;
		}
		if (*pos) {
		   ++pos;
		   sscanf(pos,"%s",pathname);
		   if (!strcmp(pathname,"print")) have_print = 1;
		   else if (!strcmp(pathname,"exec")) {
			fprintf(stderr,MSGSTR(NOT_VALID_EXEC, "find: -exec not valid with mltape\n"));
			exit(1);
		   }
		   else if (!strcmp(pathname,"depth")) {
			fprintf(stderr,MSGSTR(NOT_VALID_DEPTH, "find: -depth not valid with mltape\n"));
			exit(1);
		   }
		}
	}
	if (!have_print) {
		fprintf(stderr,MSGSTR(PRINT_REQUIRED, "find: -print required with mltape\n"));
		exit(1);
	}

	/* Now create a pipe with which to communicate with our slave version
	 * of find(1);
	 */
	if (pipe(fd)) remark(MSG_PIPE);

	/* Add all sorts of privileges into the base privilege
	 * set, if the user has the appropriate kernel authorization,
	 *  so that find(1) will generate ALL filenames in the 
	 * directory tree.  We let mltape(1) determine which files it 
	 * can export.
	 */

	/* Save our current base privilege set. */
	if (getpriv(SEC_BASE_PRIV,basep) < 0)
			remark(MSG_GETPRIVS);

	if (hassysauth(SEC_OWNER))
		ADDBIT(basep,SEC_OWNER);
	if (hassysauth(SEC_ALLOWDACACCESS))
		ADDBIT(basep,SEC_ALLOWDACACCESS);
#if SEC_MAC
	if (hassysauth(SEC_ALLOWMACACCESS))
		ADDBIT(basep,SEC_ALLOWMACACCESS);
	if (hassysauth(SEC_MULTILEVELDIR))
		ADDBIT(basep,SEC_MULTILEVELDIR);
#endif
#if SEC_ILB
	if (hassysauth(SEC_ALLOWILBACCESS))
		ADDBIT(basep,SEC_ALLOWILBACCESS);
	if (hassysauth(SEC_ILNOFLOAT))
		ADDBIT(basep,SEC_ILNOFLOAT);
#endif
#if SEC_NCAV
	if (hassysauth(SEC_ALLOWNCAVACCESS))
		ADDBIT(basep,SEC_ALLOWNCAVACCESS);
#endif
	if (setbaseprivs(basep))
			remark(MSG_SETPRIVS);

	/* Create the captive find(1) process to generate our filelist for
	 * backup.
	 */
	if (fork() == 0) {

		/* Connect the standard output to the main mltape program. */
		close(1);
		dup(fd[1]);

		/* Redirect any error output. */
		close(2);
		open("/dev/null",O_WRONLY);

		/* parse the command line to generate any intermediate
		 * directories.
		 */
		pos = args;

		while (*pos && (*pos != '-')) {
			while ((*pos==' ') || (*pos=='\t') || (*pos=='\n'))
				++pos;
			end = pos;
			while (*end && (*end != ' ') && (*end != '\t') &&
			      (*end != '\n') && (*end != '-')) ++end;
			ch = *end;
			*end = 0;
			pr_intermediates(pos);
			*end = ch;
			pos = end;	
		}

		/* Invoke find(1) to generate the remaining pathnames. */
		sprintf(pathname,"/bin/find %s",args);
		system(pathname);

		/* Close the pipe on the main mltape process */
		ch = 0;
		printf("%s\n",END_STRING);
		exit(0);
	}

	/* Connect our standard input to the find process. */
	close(0);
	dup(fd[0]);
}

#endif /*} SEC_FSCHANGE */
