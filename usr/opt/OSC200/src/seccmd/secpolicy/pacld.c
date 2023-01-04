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
static char *rcsid = "@(#)$RCSfile: pacld.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/10/07 15:07:59 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	pacld.c,v $
 * Revision 1.1.1.2  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.1.2.3  1992/04/24  21:53:46  valin
 * 	Change to pacld to not map an explicit three entry ACL to
 * 	a WILDCARD ACL.
 * 	[1992/04/24  21:53:17  valin]
 *
 * Revision 1.1.2.2  1992/04/05  12:05:38  marquard
 * 	Initial revision of POSIX ACL daemon.
 * 	[1992/04/05  12:04:59  marquard]
 * 
 * $OSF_EndLog$
 */

/*
	Copyright (c) 1988-91 SecureWare, Inc.  All Rights Reserved.

	@(#)pacld.c	1.11 17:23:15 10/16/91 SecureWare

	Discretionary Access Control Security Policy Daemon

	This is the Access Control List security policy daemon that
	is responsible for handling all message requests directed
	to it by the ACL kernel module. Message types include 
	access decision requests, and requests to map internal
	representations to tags and vice versa.

	This daemon is initiated by the policy spawning process at
	system startup. The daemon is responsible for reading the
	minor device for ACL to obtain messages and to respond to
	the messages via the same minor device. Once initialized,
	the daemon continually reads the device and responds until
	it is requested to shutdown.

*/

#include	"sys/secdefines.h"
#include	"sys/types.h"

#include	"stdio.h"
#include	"errno.h"
#include	"fcntl.h"
#include	"pwd.h"
#include	"grp.h"

#include	"sys/signal.h"
#include	"sys/errno.h"
#include	"sys/mode.h"
#include	"sys/security.h"
#include	"sys/secpolicy.h"
#include	"sys/secioctl.h"
#include	"acl.h"
#include	"spdbm.h"
#ifndef _OSF_SOURCE
#include	"sys/lock.h"	/* no process locking on OSF/1 */
#else
#include	"sys/ioctl.h"	/* for TIOCNOTTY */
#endif

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "secpolicy_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SECPOLICY,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_ACL_POSIX

#define LINE_LENGTH		256
#define MAX_MESSAGE_SIZE	(1024 * 16)

#ifdef DEBUG
#define DEBUG_FIRST	"/tcb/files/ACL_LOG"
#define DEBUG_LAST	"/usr/tmp/ACL_LOG"

FILE *dbgfile;
#endif

void terminate();
void bad_syscall();
void debug_switch();
static int change_dac_perms();
static int set_decision ();
static int groupcheck();
static int validate_acl ();
static int get_new_obj_acl ();

/* Pointers and Buffers for IRs retrieved for Labels under comparison */

struct acl_ir	*aclir;
pacl_t 		*aclhdr;
#if SEC_GROUPS
dacid_t *subjir;
int ngroups;
#endif

pacl_t	*acl;		/* Buffer for ACL initial internal representation */
int	aclsize;	/* size of internal representation */

#if SEC_GROUPS
dacid_t	*id;		/* Buffer for Multiple Group Subject Id internal rep */
#endif

struct	sp_init sp_init;

/* Pointers to the Various Message Types */

struct	spd_map_tag *map_tag;
struct	spd_set_tag *set_tag;
struct	spd_make_decision *make_dec;
struct	spd_decision *decision;
struct	spd_get_attribute *get_attr;
struct	spd_internal_rep *int_rep;
struct	spd_change_discr *chg_discr;
struct	spd_new_discr *new_discr;
struct	spd_object_create *obj_create;
struct	spd_new_objtag *new_objtag;

struct	passwd *getpwnam();
struct	group *getgrnam();

char	dev_file[LINE_LENGTH];
char	in_line[LINE_LENGTH];

/* Message Buffer and Message Header Pointer */

mhdr_t	*mhdr;
char	msg_buffer[MAX_MESSAGE_SIZE];

/* Tag Buffer and Null Tag Definition */

#if SEC_GROUPS
tag_t	subj_tag;
#else
dacid_t	subj_tag;
#endif
tag_t	acl_acc_tag;
tag_t	acl_def_tag;
tag_t	acl_tag;
tag_t	obj_tag;
tag_t	null_tag =  SEC_ACL_NULL_TAG;

int	acl_device = 0;

#ifdef DEBUG
int	getcount = 0;
#endif

main(argc, argv)
int argc;
char **argv;
{
	struct passwd *pw;
	struct group *gr;
	int ret;
	dev_t minordev;

#ifdef NLS
	(void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
	catd = catopen(MF_SECPOLICY,NL_CAT_LOCALE);
#endif
	/* dissociate from terminal, if started from one */
	(void) close(0);
	(void) close(1);
	(void) close(2);
	(void) open("/", O_RDONLY);
	dup(0);
	dup(1);
#ifdef DEBUG
	if((dbgfile = fopen(DEBUG_FIRST,"w")) == NULL)
		error(MSGSTR(ACLD_1, "Error on open of ACL logfile"));

	if (dbgfile != NULL)
		fprintf(dbgfile,MSGSTR(ACLD_2, "ACL: daemon initiated\n"));
	else
		error(MSGSTR(ACLD_2, "ACL: daemon initiated\n"));
#endif

	/* The SIGTERM is for readonly shutdown mode and the SIGUSR1
	   is used for debug file switch after filesystem mounting.
	   The SIGSYS occurs when not running on the secure kernel. */

	signal(SIGUSR1,debug_switch);


	signal(SIGTERM,terminate);
	signal(SIGSYS,bad_syscall);

	/* Set signals here to avoid termination problems */

	signal(SIGUSR2,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGINT,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
#ifdef SIGTSTP
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
#endif

	/* Set the process group to the process id to divorce the daemon 
	from its parent in case it was started from an administrator shell. */

#ifdef _OSF_SOURCE
	{
		int t = open("/dev/tty", O_RDWR);
		if (t > 0) {
			ioctl(t, TIOCNOTTY, (char *) 0);
			(void) close(t);
		}
	}
#else
	setpgrp();
#endif
	
	/* Find the tcb entry in the /etc/passwd file and do the
	setluid() so that the setuid() and setgid() are allowed. */

	if((pw = getpwnam("tcb")) == NULL)
		error(MSGSTR(ACLD_3, "ACL: cannot find uid tcb.\n"));
	else {
		setluid(pw->pw_uid);
	}

	/* Find the tcb group entry in the /etc/group file and do the
	   setgid() for proper file permissions.		*/

	if((gr = getgrnam("tcb")) == NULL)
		error(MSGSTR(ACLD_4, "ACL: cannot find group tcb.\n"));
	else
		setgid(gr->gr_gid);

	/* Set the user id after group, otherwise setgid() will fail */

	setuid(pw->pw_uid);

	/* Call the discretionary init routine which will copy the init
	   parameters for the policy into acl_config and also will
	   build out the sp_init structure. The structure is built
	   during the ioctl(2) for SPIOC_INIT done in the library.
	   The routine returns 1 if the policy is configured, -1 if
	   an error occurred or 0 if the policy is not in effect.  */

	switch(acldaemon_init(&sp_init)) {

	   case 0:		/* Policy is configured */

		error(MSGSTR(ACLD_5, "POSIX Access control lists are configured.\n"));
		break;

	   case -1:		/* Policy not Configured or Error occurred */

#ifdef DEBUG
		if (dbgfile != NULL) {

			fprintf(dbgfile,
				"Policy not Configured or Error occurred\n");
			fflush(dbgfile);
		} else
			error("Policy not Configured or Error occurred\n");
#endif
		exit(0);
	}

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_6, "ACL: policy parameters: policy %d minor %d\n"),
			sp_init.policy,sp_init.spminor);
		fprintf(dbgfile,MSGSTR(ACLD_7, "ACL: policy parameters: subj_tag %d obj_tag %d\n"),
			sp_init.first_subj_tag,sp_init.first_obj_tag);
		fprintf(dbgfile,MSGSTR(ACLD_8, "ACL: policy parameters: subj_tags %d obj_tags %d\n"),
			sp_init.subj_tag_count,sp_init.obj_tag_count);
		fflush(dbgfile);
	} else {
		error(MSGSTR(ACLD_6, "ACL: policy parameters: policy %d minor %d\n"),
			sp_init.policy,sp_init.spminor);
		error(MSGSTR(ACLD_7, "ACL: policy parameters: subj_tag %d obj_tag %d\n"),
			sp_init.first_subj_tag,sp_init.first_obj_tag);
		error(MSGSTR(ACLD_8, "ACL: policy parameters: subj_tags %d obj_tags %d\n"),
			sp_init.subj_tag_count,sp_init.obj_tag_count);
	}
#endif

	/* Using the return value of the minor device in the sp_init
	   structure, build the device name and open the daemon
	   message device.					  */

	minordev = '0' + sp_init.spminor;
	sprintf(dev_file,"%s%c",SP_DAEMON_DEVICE,minordev);

	if((acl_device = open(dev_file,O_RDWR)) == -1) {
		error(MSGSTR(ACLD_9, "ACL: error on open of spd device"));
		exit(1);
	}

	/* Set up parameters not returned by the library GETCONF */

	sp_init.cache_size = acl_config.cache_size;
	sp_init.cache_width = ACL_CACHE_WIDTH;

	/* Perform the policy initialization for the cache */

#ifdef DEBUG
	ioctl(acl_device,SPIOC_INIT,&sp_init);
#else
	if(ioctl(acl_device,SPIOC_INIT,&sp_init) == -1) {
		perror ("init");
		error(MSGSTR(ACLD_10, "ACL: error on init ioctl of spd device"));
		close (dev_file);
		exit(1);
	}
#endif

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_11, "ACL: policy init routine called\n"));
		fflush(dbgfile);
	} else {
		error(MSGSTR(ACLD_11, "ACL: policy init routine called\n"));
	}
#endif

	/* Allocate a buffer to hold the Internal Representation of the
	   object ACL for the purpose of making an access decision.   */

	if((acl = (pacl_t *) malloc(SEC_MAX_IR_SIZE)) == (pacl_t *) 0) {
		error(MSGSTR(ACLD_12, "ACL: unable to malloc space for acl ir\n"));
		exit(1);
	}

#if SEC_GROUPS
	/* Allocate a buffer to hold the Internal Representation of the
	   Subject Uid and Group List for multiple groups. These systems
	   actually have a subject tag that is mapped by the databse. */

	if((id = (dacid_t *) malloc(sizeof(dacid_t))) == (dacid_t *) 0) {
		error(MSGSTR(ACLD_13, "ACL: unable to malloc space for id ir\n"));
		exit(1);
	}
#endif

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_14, "ACL: message device initialization done\n"));
		fprintf(dbgfile,MSGSTR(ACLD_8, "ACL: dbase:%s\n"),
			acl_config.dbase);
		fflush(dbgfile);
	} else {
		error(MSGSTR(ACLD_14, "ACL: message device initialization done\n"));
		error(MSGSTR(ACLD_8, "ACL: dbase:%s\n"),
			acl_config.dbase);
	}
#endif

	/* Open the ACL Database that contains the mapping of ACL Tags to
	   IRs and vice versa.						*/

	if(spdbm_open(acl_config.dbase,DBM_RDWR,acl_config.buffers) != 0) {
		error(MSGSTR(ACLD_15, "ACL: error on database open"));
		exit(1);
	}

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_16, "ACL: database file opened\n"));
		fflush(dbgfile);
	} else {
		error(MSGSTR(ACLD_16, "ACL: database file opened\n"));
	}
#endif

	/* Fork the Security Policy Daemon */

	if((ret = fork()) == -1) {
		error(MSGSTR(ACLD_17, "ACL: Error on fork"));
		exit(1);
	}
	else if(ret != 0)
		exit(0);

	/* Reset the daemon minor device to be exclusive (EXCL) */

	if(ioctl(acl_device,SPIOC_EXCL,0,0) == -1) {
		error(MSGSTR(ACLD_19, "ACL: error on EXCL ioctl"));
		exit(1);
	}

#ifndef _OSF_SOURCE
	/* Lock the Daemon Process Text Pages into Memory */

	if(plock(TXTLOCK) == -1) {
#ifdef DEBUG
		fprintf(dbgfile,MSGSTR(ACLD_20, "ACL: daemon process text lock fail-%d\n"),errno);
		fflush(dbgfile);
#endif
	}
	else {
#ifdef DEBUG
		fprintf(dbgfile,MSGSTR(ACLD_21, "ACL: daemon process text locked\n"));
		fflush(dbgfile);
#endif
	}
#endif

	/* Process requests from Message Driver until Shutdown */

	process_acl();
	exit(0);

}

/*
	process_acl()-Continually read the ACL message device retrieving
	message requests from the kernel module and possibly from other
	clients. These messages are acted upon and the return message is
	generated and sent to the kernel using a write(2) on the device.
*/

process_acl()
{
	int cnt;
	int i;
	ushort mode;
#if SEC_GROUPS
	int subjsize;
#endif

	mhdr = (mhdr_t *) msg_buffer;

	while(1) {

	   /* Read the next message from the ACL device for processing */

	   if((cnt = read(acl_device,msg_buffer,MAX_MESSAGE_SIZE)) == -1) {
#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_22,"ACL: error or device read %d\n"),
			errno);
		fflush(dbgfile);
	} else {
		error(MSGSTR(ACLD_22,"ACL: error or device read %d\n"),
			errno);
	}
#endif
		error(MSGSTR(ACLD_23, "ACL: error on read of acl device"));
		continue;
	   }

	   /* Check the validity of the message size just read */

	   if((cnt < 0) || (cnt < sizeof(mhdr_t)) || (cnt > MAX_MESSAGE_SIZE)) {
#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_24, "ACL: invalid msg size %d\n"),cnt);
		fflush(dbgfile);
	} else {
		error(MSGSTR(ACLD_24, "ACL: invalid msg size %d\n"),cnt);
	}
#endif
		continue;
	   }

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_25, "ACL: message type: %d sync id: %d\n"),
			mhdr->msg_type,mhdr->sync_id);
		fflush(dbgfile);
	} else {
		error(MSGSTR(ACLD_25, "ACL: message type: %d sync id: %d\n"),
			mhdr->msg_type,mhdr->sync_id);
	}
#endif

	   switch(mhdr->msg_type) {

	      case SPD_MAP_TAG:
	      {

		acle_t	*acl_entry;
		int entries;
		int which_tag;
		mode_t old_mode, new_mode;

		/* Map the IR to a Tag and Return it */

		map_tag = (struct spd_map_tag *) msg_buffer;
		set_tag = (struct spd_set_tag *) msg_buffer;

#if SEC_GROUPS
		/* for systems supporting multiple groups,
		 * subject is handled differently */
		if (map_tag->ir_type == SEC_SUBJECT)
			goto map_subject;
#endif
		/* point at the beginning of the ACL entries */

		acl_entry = (acle_t *) (map_tag + 1);
		aclsize = map_tag->ir.ir_length;
		entries = aclsize / ACL_IR_SIZE;
		which_tag = map_tag->which_tag;
		old_mode = map_tag->unixdac.mode;

		/* check an exact multiple of the entry size */

		if (entries * ACL_IR_SIZE != aclsize)
			goto bad_acl;


		/* Copy the ACL entries to the buffer and assign
		   the ACL object creator values to the
		   ACL entries for the IR.			*/

		memcpy((ulong) acl + PACL_HEADERSIZE, acl_entry, aclsize);

		aclhdr = (pacl_t *) acl;

		/* Set the creator uid and creator gid members. */

		aclhdr->acl_head.cuid = map_tag->unixdac.cuid;
		aclhdr->acl_head.cgid = map_tag->unixdac.cgid;

		/*
		 * Call validate_acl() to put the acl in evaluating
		 * order, check the entries, and adjust mode
		 * according to POSIX guidelines.
		 */

		if (validate_acl((acle_t *) (acl + 1), entries, which_tag,
					old_mode, &new_mode)) {
bad_acl:
			mhdr->msg_type = SPD_SET_TAG;
			mhdr->error_code = SPD_EBADIREP;
			mhdr->mh_flags = 0;
			set_tag->tag = null_tag;

			write(acl_device,mhdr,SPD_SET_TAG_SIZE);
			continue;
		}

		/* set return value for mode if access ACL caused change */

		if (which_tag == PACL_OBJ_ACCESS_TAG && old_mode != new_mode) {
			set_tag->obj_flags = SEC_NEW_MODE;
			set_tag->obj_discr.mode = new_mode;
#ifdef DEBUG
			if (dbgfile != NULL) {
				fprintf(dbgfile,
				  "ACL: mode change from 0%o to 0%o\n",
				  old_mode, new_mode);
				fflush(dbgfile);
			} else {
				error(
				  "ACL: mode change from 0%o to 0%o\n",
				  old_mode, new_mode);
			}
#endif
		} else
			set_tag->obj_flags = 0;

		/* map the tag */

		if (spdbm_tag_allocate(acl, aclsize + PACL_HEADERSIZE,
				&acl_tag, 0) == -1) {
			error(MSGSTR(ACLD_27,"ACL: unable to map ir to tag\n"));
			mhdr->msg_type = SPD_SET_TAG;
			mhdr->error_code = SPD_EFULLDB;
			mhdr->mh_flags = 0;
			set_tag->tag = null_tag;

			write(acl_device,mhdr,SPD_SET_TAG_SIZE);
			continue;
		}

		/* Build a message with new Tag and Return it */

		mhdr->msg_type = SPD_SET_TAG;
		mhdr->error_code = SPD_OK;
		mhdr->mh_flags = 0;

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_28, "ACL: mapped tag: %x\n"),acl_tag);
		fflush(dbgfile);
	} else {
		error(MSGSTR(ACLD_28, "ACL: mapped tag: %x\n"),acl_tag);
	}
#endif

		set_tag->tag = acl_tag;

		write(acl_device,mhdr,SPD_SET_TAG_SIZE);

		continue;


#if SEC_GROUPS

		/* Map a subject IR to a tag - multiple groups */
map_subject:
		subjir = (dacid_t *) (map_tag + 1);
		memcpy(id, subjir, map_tag->ir.ir_length);

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile, MSGSTR(ACLD_29,
			"ACL: map subject tag for uid %d gid %d groups"),
			subjir->uid, subjir->gid);
	} else {
		error(MSGSTR(ACLD_29,
			"ACL: map subject tag for uid %d gid %d groups"),
			subjir->uid, subjir->gid);
	}
	ngroups = (map_tag->ir.ir_length -
		    sizeof(subjir->uid) - sizeof(subjir->gid)) /
		    sizeof(subjir->groups[0]);
	for (i = 0; i < ngroups; i++) {
		if (dbgfile != NULL)
			fprintf(dbgfile, " %d", subjir->groups[i]);
		else
			error(" %d", subjir->groups[i]);
	}
	if (dbgfile != NULL) {
		putc('\n', dbgfile);
		fflush(dbgfile);
	} else
		error("\n");
#endif
		/* Map the IR to a Tag for the Subject */

		if (spdbm_tag_allocate(id, map_tag->ir.ir_length,
		    &subj_tag, 0) == -1) {
			error(MSGSTR(ACLD_30,
				"ACL: unable to map subj ir to tag\n"));
			mhdr->msg_type = SPD_SET_TAG;
			mhdr->error_code = SPD_EFULLDB;
			mhdr->mh_flags = 0;
			write(acl_device, mhdr, SPD_SET_TAG_SIZE);
			continue;
		}

		/* Build a message with new Tag and return it. */

		mhdr->msg_type = SPD_SET_TAG;
		mhdr->error_code = SPD_OK;
		mhdr->mh_flags = 0;

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_31,"ACL: set subject tag: %x\n"),subj_tag);
		fflush(dbgfile);
	} else
		error(MSGSTR(ACLD_31,"ACL: set subject tag: %x\n"),subj_tag);
#endif

		set_tag->tag = subj_tag;
		write(acl_device, mhdr, SPD_SET_TAG_SIZE);
		continue;

#endif /* SEC_GROUPS */

	      }

	      case SPD_GET_ATTRIBUTE:

		/* Map the Tag to its Internal Representation */

		get_attr = (struct spd_get_attribute *) msg_buffer;
		int_rep = (struct spd_internal_rep *) msg_buffer;

		/* Save the tag value from message for return */

		acl_tag = get_attr->tag;

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_32, "ACL: get attr for: %x\n"),acl_tag);
		fflush(dbgfile);
	} else
		error(MSGSTR(ACLD_32, "ACL: get attr for: %x\n"),acl_tag);
#endif

		if(spdbm_get_ir(&get_attr->tag,acl,
		   &int_rep->ir.ir_length) == -1) {
			error(MSGSTR(ACLD_33,"ACL: unable to map tag to ir\n"));
			mhdr->msg_type = SPD_INTERNAL_REP;
			mhdr->error_code = 0;
			mhdr->mh_flags = 0;
			int_rep->ir.ir_length = 0;

			int_rep->tag = acl_tag;
			write(acl_device,mhdr,SPD_INTERNAL_REP_SIZE);
			continue;
		}

		/* Strip off ACL header and copy entries to message */

		memcpy((ulong) int_rep + SPD_INTERNAL_REP_SIZE,
			(ulong) acl + PACL_HEADERSIZE,
			int_rep->ir.ir_length - PACL_HEADERSIZE);

		int_rep->ir.ir_length -= PACL_HEADERSIZE;

		/* Build a message with Mapped IR and Return it */

		mhdr->msg_type = SPD_INTERNAL_REP;
		mhdr->error_code = SPD_OK;
		mhdr->mh_flags = 0;
		int_rep->tag = acl_tag;

		write(acl_device,mhdr,SPD_INTERNAL_REP_SIZE + 
				int_rep->ir.ir_length);

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf (dbgfile,"successful read for tag:%x\n",acl_tag);
		fflush (dbgfile);
	} else
		error ("successful read for tag:%x\n",acl_tag);
#endif
		continue;


	      case SPD_MAKE_DECISION:

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf (dbgfile,"ACL: got MAKE_DECISION message\n");
		fflush(dbgfile);
	} else
		error ("ACL: got MAKE_DECISION message\n");
#endif

		/* Retrieve IRs and Make Access Decision */

		decision = (struct spd_decision *) msg_buffer;
		make_dec = (struct spd_make_decision *) msg_buffer;

		/* get the requested mode */

		mode = mhdr->mh_flags;

		/* Save the tag value from message for return */

#if SEC_GROUPS
		subj_tag = make_dec->subject;
#else
		subj_tag = *((dacid_t *) &make_dec->subject);
#endif
		obj_tag = make_dec->object;

#ifdef DEBUG
#if SEC_GROUPS
	if (dbgfile != NULL)
		fprintf(dbgfile,
			MSGSTR(ACLD_34, "ACL: decision on: subj tag: %x obj_tag: %x\n"),
			subj_tag,obj_tag);
	else
		error(MSGSTR(ACLD_34, "ACL: decision on: subj tag: %x obj_tag: %x\n"),
			subj_tag,obj_tag);
#else
	if (dbgfile != NULL)
		fprintf(dbgfile,MSGSTR(ACLD_35, "ACL: decision on: %d %d tag: %x\n"),
			subj_tag.uid,subj_tag.gid,obj_tag);
	else
		error(MSGSTR(ACLD_35, "ACL: decision on: %d %d tag: %x\n"),
			subj_tag.uid,subj_tag.gid,obj_tag);
#endif
	if (dbgfile != NULL)
		fflush(dbgfile);
#endif

#if SEC_GROUPS
		/* Lookup the IR for the Subject Id Internal Representation */

		if(spdbm_get_ir(&make_dec->subject,id,&subjsize) == -1) {
			error(MSGSTR(ACLD_36, "ACL: can't map subject tag\n"));
			mhdr->msg_type = SPD_DECISION;
			mhdr->error_code = SPD_ENOSUBJTAG;
			mhdr->mh_flags = 0;

		/* Restore tag values and write return error message */

			decision->subject = subj_tag;
			decision->object = obj_tag;
			decision->decision.dec_length = 0;
			write(acl_device,mhdr,SPD_DECISION_SIZE);
			continue;
		}
#endif

		if(spdbm_get_ir(&make_dec->object,acl,&aclsize) == -1) {
			error(MSGSTR(ACLD_37, "ACL: can't map object tag\n"));
			mhdr->msg_type = SPD_DECISION;
			mhdr->error_code = SPD_ENOOBJTAG;
			mhdr->mh_flags = 0;

		/* Restore tag values and write return error message */

#if SEC_GROUPS
			decision->subject = subj_tag;
#else
			decision->subject = *((tag_t *) &subj_tag);
#endif
			decision->object = obj_tag;
			decision->decision.dec_length = 0;
			write(acl_device,mhdr,SPD_DECISION_SIZE);
			continue;
		}

		aclsize -= PACL_HEADERSIZE;

		/* Make the Access Decision between the IRs and return it */

		mhdr->msg_type = SPD_DECISION;
		mhdr->error_code = SPD_OK;
		mhdr->mh_flags = 0;

		/* Multiple group access decision is based on the IR from the
		   for the Uid and Supplementary Groups whereas the System
		   V decision uses the subject tag as is with Uid/Gid.  */

#if SEC_GROUPS
		ngroups = (subjsize - sizeof(id->uid) - sizeof(id->gid))
			/ sizeof(id->groups[0]);

		make_decision(id,&decision->decision,subjsize,mode);
		decision->subject = subj_tag;
#else
		make_decision(&subj_tag,&decision->decision,0,mode);
		decision->subject = *((tag_t *) &subj_tag);
#endif

		decision->object = obj_tag;
		decision->decision.dec_length = sizeof(acache_dec_t);
		write(acl_device,mhdr,SPD_DECISION_SIZE + 
			sizeof(acache_dec_t));
		continue;

	      case SPD_CHANGE_DISCR:

		/* Owner, group, or mode on object changed */

		chg_discr = (struct spd_change_discr *) msg_buffer;
		new_discr = (struct spd_new_discr *) msg_buffer;

		/* Map the tag to an IR */

		if(spdbm_get_ir(&chg_discr->object_tag,acl,&aclsize) == -1) {
			error(MSGSTR(ACLD_37, "ACL: can't map object tag\n"));
			mhdr->error_code = SPD_ENOOBJTAG;
		}

		else {
			int ret;


			/*
		 	 * If mode changes are necessary, it will be
			 * reflected back in the return code from
			 * change_dac_perms().
			 * Map a new tag if changes were made to make
			 * the new acl different from the old. 
		 	 */

			aclsize -= PACL_HEADERSIZE;

			if ((ret = change_dac_perms()) == -1) {
				error(MSGSTR(ACLD_37,
					"ACL: can't map object tag\n"));
				mhdr->error_code = SPD_EBADIREP;
			}

			/* If changes are required, get a new tag. */
			else if (ret == 1) {

				aclsize += PACL_HEADERSIZE;
				if (spdbm_tag_allocate(acl, aclsize,
			    		&new_discr->object_tag, 0) == -1) {
					error(MSGSTR(ACLD_37,
						"ACL: can't map object tag\n"));
					mhdr->error_code = SPD_ENOOBJTAG;
				}
			}
			/* If return is 0 the tag will stay the same. */
		}

		/*
		 * POSIX ACL policy affects UNIX dac.
		 * The changes will have been made in
		 * change_dac_perms and a new tag will have
		 * been allocated.  If it becomes necessary
		 * to indicate the changes to the calling
		 * party, this flag should be updated with
		 * SEC_NEW_UID, SEC_NEW_GID  etc.  In this
		 * version of the draft it is not necessary.
		 */

		new_discr->object_flags = 0;

		mhdr->msg_type = SPD_NEW_DISCR;
		mhdr->error_code = SPD_OK;
		mhdr->mh_flags = 0;

		write(acl_device, mhdr, SPD_NEW_DISCR_SIZE);

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf (dbgfile,"Successful dac change.\n");
		fflush (dbgfile);
	} else
		error ("Successful dac change.\n");
#endif
		continue;


	      /*
	       * Retrieve a new access and default ACL tag for a newly
	       * created file in a directory that has a default ACL.
	       * Set the user and group of the default ACL to those
	       * specified in the message to create the new access
	       * ACL and map it to the new access ACL tag.
	       * Mask the permission bits of the USER_OBJ, OTHER_OBJ,
	       * and either MASK_OBJ or GROUP_OBJ with the appropriate mode
	       * bits specified to the object creation system call.  The new
	       * default ACL is directly inherited (the tag is copied).
	       */

	      case SPD_GET_NEW_OBJTAG:
	      {
		int	aclmode;	/* mode of newly created file */
		int	entries;	/* number of entries in the ACL */
		int	mode;		/* specified mode */
		tag_t	default_tag;

		/* Set pointers to the request and response message */

		obj_create = (struct spd_object_create *) msg_buffer;
		new_objtag = (struct spd_new_objtag *) msg_buffer;
		default_tag = obj_create->default_tag;

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,"ACL: got GET_NEW_OBJTAG message\n");
		fprintf(dbgfile,
		  "default tag 0x%x access tag 0x%x, euid %d egid %d mode %o\n",
		  obj_create->default_tag, obj_create->access_tag,
		  obj_create->edac.euid, obj_create->edac.egid, obj_create->mode);
		fflush(dbgfile);
	} else {
		error("ACL: got GET_NEW_OBJTAG message\n");
		error("default tag 0x%x access tag 0x%x, euid %d egid %d mode %o\n",
		  obj_create->default_tag, obj_create->access_tag,
		  obj_create->edac.euid, obj_create->edac.egid, obj_create->mode);
	}
#endif
		/* Retrieve the default ACL from the policy data base */

		if (spdbm_get_ir(&default_tag, acl, &aclsize) == -1) {
			error(MSGSTR(ACLD_33,
				"ACL: unable to map tag to ir\n"));
			mhdr->msg_type = SPD_SET_NEW_OBJTAG;
			mhdr->error_code = SPD_ENOOBJTAG;
			mhdr->mh_flags = 0;

			write(acl_device,mhdr,SPD_NEW_OBJTAG_SIZE);
			continue;
		}

		/*
		 * Create the new access ACL from the parent directory's
		 * default ACL, the user and group specified in the
		 * message, and the mode.
		 */
		
		entries = (aclsize - PACL_HEADERSIZE) / ACL_IR_SIZE;
		mode = obj_create->mode;

		aclmode = get_new_obj_acl(obj_create->edac.euid,
					  obj_create->edac.egid,
					  obj_create->mode,
					  acl + 1,	/* ptr to 1st entry */
					  entries);

		/* map the tag */

		if (spdbm_tag_allocate(acl, aclsize,
				&new_objtag->access_tag, 0) == -1) {
			error(MSGSTR(ACLD_27,
				"ACL: unable to map ir to tag\n"));
			mhdr->msg_type = SPD_SET_NEW_OBJTAG;
			mhdr->error_code = SPD_EFULLDB;
			mhdr->mh_flags = 0;

			write(acl_device,mhdr,SPD_NEW_OBJTAG_SIZE);
			continue;
		}

		/* If the object is a directory, inherit the tag */

		if (S_ISDIR(mode))
			new_objtag->default_tag = default_tag;
		else
			new_objtag->default_tag = SEC_WILDCARD_TAG_VALUE;

		/* the new file mode was returned from get_new_obj_acl() */

		new_objtag->mode = aclmode | (mode & ~0777);

#ifdef DEBUG
	if (dbgfile != NULL)
		fprintf(dbgfile,
		  "New default tag 0x%x, new access tag 0x%x, new mode 0%o\n",
		   new_objtag->access_tag, new_objtag->default_tag,
		   new_objtag->mode);
	else
		error("New default tag 0x%x, new access tag 0x%x, new mode 0%o\n",
		   new_objtag->access_tag, new_objtag->default_tag,
		   new_objtag->mode);
#endif

		/* Return the message to the policy module */

		mhdr->msg_type = SPD_SET_NEW_OBJTAG;
		mhdr->error_code = SPD_OK;
		mhdr->mh_flags = 0;

		write(acl_device,mhdr,SPD_NEW_OBJTAG_SIZE);

		continue;

	      }

	      case SPD_PREPARE_SHUTDOWN:

		/* Daemon to be terminated */

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_38, "ACL: shutdown notify message\n"));
		fflush(dbgfile);
	} else
		error(MSGSTR(ACLD_38, "ACL: shutdown notify message\n"));
#endif
		mhdr->msg_type = SPD_PREPARE_RESP;
		mhdr->mh_flags = 0;
		mhdr->error_code = SPD_OK;
		write(acl_device,mhdr,SPD_MHDR_SIZE);
		continue;

	      case SPD_SHUTDOWN:

		/* Terminate daemon execution */

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_39, "ACL: shutdown message\n"));
		fflush(dbgfile);
	} else
		error(MSGSTR(ACLD_39, "ACL: shutdown message\n"));
#endif
		spdbm_close();

		mhdr->msg_type = SPD_SHUTDOWN_RESP;
		mhdr->mh_flags = 0;
		mhdr->error_code = SPD_OK;
		write(acl_device,mhdr,SPD_MHDR_SIZE);

		close(acl_device);
		exit(0);

	      default:

		/* Invalid Message Type */

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_40, "ACL: invalid message type %d\n"),
			mhdr->msg_type);
		fflush(dbgfile);
	} else
		error(MSGSTR(ACLD_40, "ACL: invalid message type %d\n"),
			mhdr->msg_type);
#endif
		mhdr->msg_type = SPD_ERROR;
		mhdr->mh_flags = 0;
		mhdr->error_code = SPD_EBADMSG;
		write(acl_device,mhdr,SPD_MHDR_SIZE);
		continue;
	      }

	}
}

/*
 * FUNCTION:
 *	get_new_obj_acl()
 *
 * ARGUMENTS:
 *	none.
 *
 * DESCRIPTION:
 *	The get_new_obj_acl() function sets the object
 *	inheritance acl for a created object.
 *	The permission bits in the appropriate ACLs are masked by the
 *	specified mode.  The USER_OBJ and GROUP_OBJ qualifiers are set.
 *
 * RETURNS:
 *	The new mode of the object.
 */

static int
get_new_obj_acl(euid, egid, mode, ent, numents)
uid_t		euid;
gid_t		egid;
mode_t		mode;
acle_t		*ent;
int		numents;
{
	register int	i;
	mode_t		aclmode;

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,
		  "get_new_obj_acl: euid %d egid %d, specified mode 0%o, numents %d\n",
		  euid, egid, mode, numents);
		fflush(dbgfile);
	} else
		error("get_new_obj_acl: euid %d egid %d, specified mode 0%o, numents %d\n",
		  euid, egid, mode, numents);
#endif

	/* Initialize the return mode. */
	aclmode = 0;

	for (i = 0; i < numents; i++, ent++) {

		switch (ent->acl_tag) {

			/*
			 * Change the USER_OBJ entry permissions
			 * to the intersection with the owner
			 * bits from the mode parameter.
			 */
			case USER_OBJ:

				ent->acl_perm &= (mode >> 6) & 07;
				aclmode |= ent->acl_perm << 6;
				ent->acl_uid = euid;
				continue;

			/*
			 * If the ACL contains a MASK_OBJ entry,
			 * then change the MASK_OBJ entry permissions
			 * to their intersection of the group bits from
			 * the mode paramter.
			 */
			case MASK_OBJ:

				ent->acl_perm &= (mode >> 3) & 07;
				aclmode |= ent->acl_perm << 3;
				continue;

			/*
			 * If the ACL doesn't contain a MASK_OBJ entry
			 * (it has exactly 3 entries), then change the
			 * GROUP_OBJ entry permission bits to their
			 * intersection with the group bits from the
			 * mode parameter.
			 */
			case GROUP_OBJ:

				if (numents == 3) {
					ent->acl_perm &= (mode >> 3) & 07;
					aclmode |= ent->acl_perm << 3;
				}
				ent->acl_gid = egid;
				continue;

			/*
			 * Change the OTHER_OBJ entry permissions
			 * to their intersection of the other bits from
			 * the mode paramter.
			 */
			case OTHER_OBJ:

				ent->acl_perm &= mode & 07;
				aclmode |= ent->acl_perm;
				continue;
		}
	}
#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile, "get_new_obj_acl: return mode 0%o\n", aclmode);
		fflush(dbgfile);
	} else
		error("get_new_obj_acl: return mode 0%o\n", aclmode);
#endif
	return aclmode;
}

/*
 * FUNCTION:
 *	make_decision()
 *
 * ARGUMENTS:
 *	dacid_t		*subjid;   - the subject id value.
 *	acl_dec_t	*decision; - the decision returned.
 *	int		subjsize;  - 
 *	ushort		mode;      - the access mode requested. 
 *
 * DESCRIPTION:
 *
 *	The make_decision() function will determine the DAC access.
 *
 *	Each access mode requested is logically checked separately.
 *
 *	The process request is granted only if all individually
 *		requested modes are granted.
 *
 *	The MASK_OBJ is used as a mask to restrict the permissions of
 *		all entries in the file group class.
 *
 *	For each mode requested, The checking algorithm is basically:
 *		1. - USER_OBJ entry.
 *		2. - USER entries.
 *		3. - GROUP_OBJ entry (including any matching GROUP entry).
 *		4. - GROUP entries.
 *		5. - OTHER entry.
 *
 *	The algorithm:
 *
 *		if (EFF UID of the process == UID of the object owner) {
 *		  if (requested access mode is granted by USER_OBJ->perm)
 *			the access mode is granted.
 *		  else the access mode is denied.
 *		}
 *		
 *		if (EFF UID of the process == any USER->acl_uid) {
 *		  if (requested access mode is granted by MASK_OBJ->perm) &&
 *		     (requested access mode is granted by USER_OBJ->perm)
 *			the access mode is granted.
 *		  else the access mode is denied.
 *		}
 *
 *		if ((EFF GID or any groups in groupset) ==
 *				GID of the object group) {
 *		  if (no MASK_OBJ) {
 *		    if (requested access mode is granted by GROUP_OBJ->perm)
 *			the access mode is granted.
 *		    else the access mode is denied.
 *		  else
 *		    if ((requested access mode is granted by MASK_OBJ->perm)) &&
 *		       ((requested access mode is granted by GROUP_OBJ->perm) ||
 *		        (requested access mode is granted by any GROUP->perm
 *				that matches either the EFF GID or any
 *				supplementary GIDs of the process))
 *			the access mode is granted.
 *		    else the access mode is denied.
 *		}
 *
 *		if ((EFF GID or any groups in groupset) == any GROUP->acl_gid) {
 *		  if (requested access mode is granted by MASK_OBJ->perm) &&
 *		     (requested access mode is granted by GROUP->perm)
 *			the access mode is granted.
 *		  else the access mode is denied.
 *		}
 *
 *		if (requested access mode is granted by OTHER_OBJ->perm)
 *			the access mode is granted.
 *		else the access mode is denied.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - if entry_d does not refer to a valid ACL entry.
 */


int
make_decision(subjid, decision, subjsize,mode)
dacid_t		*subjid;
acl_dec_t 	*decision;
int 		subjsize;
ushort		mode;
{
	register int		i=0;
	int			gotmask = 0;
	register int		j;
	acl_permset_t		maskperms;
	acl_permset_t		gperms = 0;
	int			gotgroup = 0;
	register acle_t		*ent;
	register int		numents = aclsize / ACL_IR_SIZE;

	/* ACL must have at least USER_OBJ, GROUP_OBJ, OTHER_OBJ */
	if (numents < 3)
		goto out;

	/*
	 * Get pointer to acl entries from
	 * internal representation.
	 */
	ent = (acle_t *) (acl + 1);

	/*
	 * Look for an entry that matches
	 * the above criteria or until the entries are exhausted.
	 * The acl entries will be stored in evaluating
	 * order mandated by the call to validate_acl().
	 */

	/*
	 * Check USER_OBJ first.
	 */
	if (ent[i].acl_tag != USER_OBJ)
		goto out;

	if ((subjid->uid == ent[i].acl_uid) ||
	    (subjid->uid == acl->acl_head.cuid)){

		/*
		 * Set the decision based solely
		 * on the object owner entry.
		 */
		set_decision (decision,ent[i].acl_perm);
		return 0;
	}
	i++;
	
	/*
	 * Get MASK_OBJ perms next.
	 */
	if (numents > 3) {

		if (ent[i].acl_tag == MASK_OBJ) {
			gotmask++;
			maskperms = ent[i].acl_perm;
			i++;
		}
		else
			goto out;
	}

	/*
	 * Check USER entries next.
	 */
	while (ent[i].acl_tag == USER) {

 		/*
		 * The effective user id of the process equals
		 * the user id of the object owner.
		 */
		if(subjid->uid == ent[i].acl_uid) {

			/*
			 * Set the match decision based on the
			 * intersection of the object owner entry
			 * and the MASK_OBJ permissions.
			 */
			set_decision (decision,
				( maskperms & ent[i].acl_perm));

			return 0;
		}
		i++;
	}

	/*
	 * Check GROUP_OBJ next.
	 */
	if (ent[i].acl_tag != GROUP_OBJ)
		goto out;

	/* check against primary group */

	if (subjid->gid == ent[i].acl_gid ||
	    subjid->gid == acl->acl_head.cgid) {

		/* if a 3 entry ACL, decision is based on the GROUP_OBJ entry */

		if (!gotmask) {
			set_decision (decision,ent[i].acl_perm);
			return 0;
		}
		
		/*
		 * otherwise, begin forming the union of matching group
		 * entry permissions
		 */

		gotgroup++;
		gperms |= ent[i].acl_perm;
	}

	/* check supplementary groups */

	else for (j = 0; j < ngroups; j++) {

		/* if a 3 entry ACL, decision is based on the GROUP_OBJ entry */

		if (subjid->groups[j] == ent[i].acl_gid) {

			/*
			 * If we don't have a mask entry,
			 * access is based solely on the
			 * GROUP_OBJ entry's permissions.
			 */

			if (!gotmask) {
				set_decision (decision,ent[i].acl_perm);
				return 0;
			}
				
			gotgroup++;
			gperms |= ent[i].acl_perm;
			break;
		}
	}
	i++;

	/* Check GROUP entries */

	while (ent[i].acl_tag == GROUP) {

		/* check primary group */

		if (subjid->gid == ent[i].acl_gid) {
			gotgroup++;
			gperms |= ent[i].acl_perm;
		}
		else for (j = 0; j < ngroups; j++) {

			/* check supplementary groups */

			if (subjid->groups[j] == ent[i].acl_gid) {
				gotgroup++;
				gperms |= ent[i].acl_perm;
				break;
			}
		}
		i++;
	}

	/*
	 * If we matched on at least one group id, decision
	 * is based on the intersection of the mask perms
	 * and the group perms.
	 */
	if (gotgroup) {
		set_decision (decision,(gperms & maskperms));
		return 0;
	}

	/*
	 * Check OTHER_OBJ last.
	 */
	if (ent[i].acl_tag != OTHER_OBJ)
		goto out;

	set_decision (decision,ent[i].acl_perm);
	return 0;

out:

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_41, "ACL: no access permitted\n"));
		fflush(dbgfile);
	} else
		error(MSGSTR(ACLD_41, "ACL: no access permitted\n"));
#endif
	set_decision (decision,ACL_NOPERM);

	return 0;
}


/*
 * set_decision().
 *	Sets the return decision structure
 *	according to the perm argument.
 */
static int
set_decision (decision,perm)
acl_dec_t 	*decision;
acl_permset_t	perm;
{
	decision->dec.read = ((perm & ACL_PREAD) != 0);
	decision->dec.write = ((perm & ACL_PWRITE) != 0);
	decision->dec.exec = ((perm & ACL_PEXECUTE) != 0);
	decision->dec.delete = 0;
	decision->dec.rfu = 0;

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_42,
			"ACL: decision Read: %d Write: %d Exec: %d\n"),
			decision->dec.read,decision->dec.write,decision->dec.exec);
		fflush(dbgfile);
	} else
		error(MSGSTR(ACLD_42,
			"ACL: decision Read: %d Write: %d Exec: %d\n"),
			decision->dec.read,decision->dec.write,decision->dec.exec);
#endif

	return 0;
}


/*
 * FUNCTION:
 *	validate_acl()
 *
 * ARGUMENTS:
 *	none.
 *
 * DESCRIPTION:
 *	The validate_acl() function checks the ACL for validity.
 *	- The entries are placed in evaluating order.
 *	- The three required entries: USER_OBJ, GROUP_OBJ,
 *		and OTHER_OBJ shall exist exactly once.
 *	- If the ACL contains four or more entries then the MASK_OBJ
 *		entry shall exist exactly once.
 *
 * policy on mode change:
 *	- If a MASK_OBJ entry is not present in the object's ACL,
 *		the permissions associated with the GROUP_OBJ entry will
 *		be set equal to the file group class permission bits.
 *
 *	- If a MASK_OBJ is present in the object's ACL,
 *		the permissions associated with the MASK_OBJ entry will be
 *		set equal to the file group class permission bits and the
 *		permissions associated with the GROUP_OBJ entry will remain
 *		unchanged.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *			 
 */

static int
validate_acl(ent, numents, which_tag, curmode, newmodep)
acle_t *ent;
int numents;
short which_tag;
mode_t curmode;
mode_t *newmodep;
{
	mode_t		newmode = curmode &~ 0777;
	register acle_t *enti, *entj;
	register int	i, j;
	int gotmask = 0;

	/*
	 * Must be at least USER_OBJ,
	 * GROUP_OBJ, OTHER_OBJ.
	 */
	if (numents < 3)
		return -1;

	/*
	 * Sort the entries into evaluating order.
	 * The tag values correspond to the order
	 * in which they are evaluated.
	 * 
	 *	USER_OBJ entry
	 *	MASK_OBJ entry
	 *	USER entries (sorted)
	 *	GROUP_OBJ entry
	 *	GROUP entries (sorted)
	 *	OTHER entry
	 */

	for (i = 0, enti = ent; i < numents - 1; i++, enti++) {

		for (j = i + 1, entj = enti + 1; j < numents; j++, entj++) {

			int swap = 0;

			/* sort in increasing tag type order */

			if (entj->acl_tag > enti->acl_tag)
				continue;

			else if (entj->acl_tag < enti->acl_tag)
				swap++;

			/*
			 * sort USER entries in increasing qualifier order
			 * and ensure uniqueness
			 */

			else if (entj->acl_tag == USER &&
			    enti->acl_tag == USER) {
				if (entj->acl_uid == enti->acl_uid)
					return -1;
			    	if (entj->acl_uid < enti->acl_uid)
					swap++;
			}

			/*
			 * sort GROUP entries in increasing qualifier order
			 * and ensure uniqueness
			 */

			else if (entj->acl_tag == GROUP &&
			    enti->acl_tag == GROUP) {
				if (entj->acl_gid == enti->acl_gid)
					return -1;
				if (entj->acl_gid < enti->acl_gid)
					swap++;
			}

			if (swap) {

				acle_t	temp;

				temp = *enti;
				*enti = *entj;
				*entj = temp;
			}
		}
	}

	/* Check that all necessary entries are present.  */

	/* Check for USER_OBJ entry */
	i = 0;
	if (ent[i].acl_tag != USER_OBJ)
		return -1;

	newmode |= ((ent[i].acl_perm & 7) << 6);

	/*
	 * set the qualifier field of the USER_OBJ entry to the object UID
	 * for an access ACL, or a non-existent UID for a default ACL
	 */

	if (which_tag == PACL_OBJ_DFLT_TAG)
		ent[i++].acl_uid = BOGUS_UID;
	else
		ent[i++].acl_uid = map_tag->unixdac.uid;

	/* Check for MASK_OBJ entry */
	if (numents >= 4 && ent[i].acl_tag != MASK_OBJ)
		return -1;

	/*
	 * If mask mode is present, set file group
	 * permissions to correspond to MASK_OBJ value.
	 */

	if (ent[i].acl_tag == MASK_OBJ) {
		gotmask++;
		newmode |= ((ent[i++].acl_perm & 7) << 3);
	}

	/* Skip USER entries */

	while (ent[i].acl_tag == USER)
		i++;

	/* Check that the GROUP_OBJ entry is present */

	if (ent[i].acl_tag != GROUP_OBJ)
		return -1;

	/*
	 * If mask mode is not present, set file group
	 * permissions to correspond to GROUP_OBJ value.
	 */

	if (!gotmask)
		newmode |= ((ent[i].acl_perm & 7) << 3);

	/*
	 * set the qualifier field of the GROUP_OBJ entry to the object GID
	 * for an access ACL, or a non-existent GID for a default ACL
	 */

	if (which_tag == PACL_OBJ_DFLT_TAG)
		ent[i++].acl_gid = BOGUS_GID;
	else
		ent[i++].acl_gid = map_tag->unixdac.gid;

	/* Skip GROUP entries */

	while (ent[i].acl_tag == GROUP)
		i++;

	/* Check that the OTHER_OBJ entry is present */

	if (ent[i].acl_tag != OTHER_OBJ)
		return -1;

	/* set return mode */

	newmode |= (ent[i++].acl_perm & 7);

	/* Check that acl_num is in sync */

	if (i != numents)
		return -1;

	*newmodep = newmode;

	return 0;
}

/*
 * If chmod changes the group permission bits, and the
 * file has a MASK_OBJ entry, the MASK_OBJ entry will be
 * updated with the new permissions.
 */

static int
change_dac_perms()
{
	register acle_t *ent;
	acl_permset_t	mode,maskmode,groupmode;
	dac_t 		*dac = &chg_discr->object_dac;
	register int	gotuser,gotmask,gotgroup,gotother,change;
	register int	i,numents;

#ifdef DEBUG
	uid_t		olduid;
	gid_t		oldgid;
	mode_t		oldmode = 0;
#endif


	/* initialize flags */
	mode = change = gotuser = gotmask = gotgroup = gotother = 0;

	/*
	 * Set acl pointer to internal
	 * representation of acl and
	 * set the number of entries.
	 */

	ent = (acle_t *) (acl + 1);
	numents = aclsize / ACL_IR_SIZE;

	/*
	 * Go through all entries and see if
	 * the USER_OBJ, GROUP_OBJ, or OTHER_OBJ
	 * have changed.
	 * If the group has changed, reflect the changes
	 * in the MASK_OBJ entry and leave the GROUP_OBJ
	 * entry as is.  If the MASK_OBJ is ever removed,
	 * the permission bits are OR'd back in with the
	 * GROUP_OBJ entry.
	 */

	for (i=0; i < numents; i++, ent++) {

	   switch (ent->acl_tag) {

	      case USER_OBJ :

#ifdef DEBUG
		olduid = ent->acl_uid;
		oldmode |= (ent->acl_perm & 7) << 6;
#endif

		mode = ((dac->mode & 0700) >> 6);

		if ((ent->acl_perm & 7) ^ mode) {
			ent->acl_perm = mode;
			change |= SEC_NEW_MODE;
		}

		if (ent->acl_uid != dac->uid) {
			ent->acl_uid = dac->uid;
			change |= SEC_NEW_UID;
		}

		gotuser++;
		break;

	      case MASK_OBJ :

#ifdef DEBUG
		oldmode |= (ent->acl_perm & 7) << 3;
#endif
		mode = ((dac->mode & 0070) >> 3);

		/*
		 * Update the MASK_OBJ with the
		 * new permissions.
		 */
		if ((ent->acl_perm & 7) ^ mode) {
			ent->acl_perm = mode;
			change |= SEC_NEW_MODE;
		}

		/* Indicate that we have a MASK_OBJ entry */
		gotmask++;
		break;

	      case GROUP_OBJ :

#ifdef DEBUG
		oldgid = ent->acl_gid;
		if (!gotmask)
			oldmode |= (ent->acl_perm & 7) << 3;
#endif

		/*
		 * If we have a MASK_OBJ entry then
		 * the GROUP_OBJ does not change.
		 */
		if (!gotmask) {
			mode = ((dac->mode & 0070) >> 3);

			if ((ent->acl_perm & 7) ^ mode) {
				ent->acl_perm = mode;
				change |= SEC_NEW_MODE;
			}
		}

		if (ent->acl_gid != dac->gid) {
			ent->acl_gid = dac->gid;
			change |= SEC_NEW_GID;
		}

		gotgroup++;
		break;

	      case OTHER_OBJ :

#ifdef DEBUG
		oldmode |= ent->acl_perm & 7;
#endif

		mode = (dac->mode & 0007);

		if ((ent->acl_perm & 7) ^ mode) {
			ent->acl_perm = mode;
			change |= SEC_NEW_MODE;
		}
		gotother++;
		break;

	      default :

		break;
	   }
	}

	/* check that all entries have been found and only once */
	if ((gotuser != 1) || (gotgroup != 1) || (gotother != 1))
		return -1;

	/*
	 * If there were any changes to the current acl,
	 * return 1 to indicate that a new tag should
	 * be allocated.
	 */
	if (change) {

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf (dbgfile,"ACL: Changing dac attributes.\n");
		fprintf (dbgfile,"\told: uid:%d gid:%d mode:%o\n",
			olduid, oldgid, oldmode);
		fprintf (dbgfile,"\tnew: uid:%d gid:%d mode:%o\n",
			dac->uid, dac->gid, dac->mode);
		fflush (dbgfile);
	} else {
		error ("ACL: Changing dac attributes.\n");
		error ("\told: uid:%d gid:%d mode:%o\n",
			olduid, oldgid, oldmode);
		error ("\tnew: uid:%d gid:%d mode:%o\n",
			dac->uid, dac->gid, dac->mode);
	}
#endif

		return 1;
	}

	return 0;

}

/*
	terminate()-this routine handles the SIGTERM from the system
	shutdown code that forces the database manager into a readonly
	mode to protect the integrity of the database. Decisions are
	still possible but no writes may take place.
*/

void
terminate()
{

	signal(SIGTERM,SIG_IGN);
#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_43, "ACL: SIGTERM received for shutdown\n"));
		fflush(dbgfile);
	} else
		error(MSGSTR(ACLD_43, "ACL: SIGTERM received for shutdown\n"));
#endif
	if(spdbm_reopen(acl_config.dbase,DBM_RDONLY) != 0) {
		error(MSGSTR(ACLD_44, "ACL: error on database reopen"));
		exit(1);
	}

#ifdef DEBUG
	if (dbgfile != NULL) {
		fprintf(dbgfile,MSGSTR(ACLD_45, "ACL: daemon reopened dbase for RDONLY\n"));
		fflush(dbgfile);
	} else
		error(MSGSTR(ACLD_45, "ACL: daemon reopened dbase for RDONLY\n"));
#endif

}

/*
 * error()-daemon version of printf that will open /dev/console, print
 * out the error message, flush and close the terminal device to avoid
 * any stopio problems.
 */

error(format,arg1,arg2,arg3,arg4,arg5)
char *format;
int arg1, arg2, arg3, arg4, arg5;
{
	register FILE *fp;

	if((fp = fopen("/dev/console","w")) == NULL)
		return;

	setbuf(fp,NULL);
	fprintf(fp,format,arg1,arg2,arg3,arg4,arg5);
	fclose(fp);
}

/*
	debug_switch()-called by signal to switch debug output files
*/

void
debug_switch()
{
	signal(SIGUSR1,SIG_IGN);
#ifdef DEBUG
	if (dbgfile != NULL) {
		fflush(dbgfile);
		fclose(dbgfile);
	}

	if((dbgfile = fopen(DEBUG_LAST,"w")) == NULL) {
		error(MSGSTR(ACLD_46, "Error on re-open of ACL log"));
		return;
	}
	fprintf(dbgfile,MSGSTR(ACLD_47, "ACL: daemon opened new debug file\n"));
#endif
}

/*
	bad_syscall()-trap a bad system call in case setluid() not on system
*/

void
bad_syscall()
{
	error(MSGSTR(ACLD_48, "ACL: bad system call\n"));
	exit(1);
}

#endif /* SEC_ACL_POSIX */
