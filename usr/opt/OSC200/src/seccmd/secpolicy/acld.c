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
static char *rcsid = "@(#)$RCSfile: acld.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/10/07 15:07:13 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*	Copyright (c) 1988-90 SecureWare, Inc.  All Rights Reserved.

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


/*
 * Based on:

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

#if SEC_ACL_SWARE

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

/* Pointers and Buffers for IRs retrieved for Labels under comparison */

struct acl_ir	*aclir;
acl_t *aclhdr;
#if SEC_GROUPS
dacid_t *subjir;
#endif

acl_t	*acl;		/* Buffer for ACL internal representation */
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
tag_t	acl_tag;
tag_t	obj_tag;
tag_t	null_tag =  SEC_ACL_NULL_TAG;

int	acl_device = 0;

main(argc, argv)
int argc;
char **argv;
{
	struct passwd *pw;
	struct group *gr;
	int ret;
	dev_t minordev;

	if (!security_is_on())
		exit(0);

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

	fprintf(dbgfile,MSGSTR(ACLD_2, "ACL: daemon initiated\n"));
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

		error(MSGSTR(ACLD_5, "Access control lists are configured.\n"));
		break;

	   case -1:		/* Policy not Configured or Error occurred */

		exit(0);
	}

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(ACLD_6, "ACL: policy parameters: policy %d minor %d\n"),
		sp_init.policy,sp_init.spminor);
	fprintf(dbgfile,MSGSTR(ACLD_7, "ACL: policy parameters: subj_tag %d obj_tag %d\n"),
		sp_init.first_subj_tag,sp_init.first_obj_tag);
	fprintf(dbgfile,MSGSTR(ACLD_8, "ACL: policy parameters: subj_tags %d obj_tags %d\n"),
		sp_init.subj_tag_count,sp_init.obj_tag_count);
	fflush(dbgfile);
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

	if(ioctl(acl_device,SPIOC_INIT,&sp_init) == -1) {
		error(MSGSTR(ACLD_10, "ACL: error on init ioctl of spd device"));
		exit(1);
	}

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(ACLD_11, "ACL: policy init routine called\n"));
	fflush(dbgfile);
#endif

	/* Allocate a buffer to hold the Internal Representation of the
	   object ACL for the purpose of making an access decision.   */

	if((acl = (acl_t *) malloc(SEC_MAX_IR_SIZE)) == (acl_t *) 0) {
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
	fprintf(dbgfile,MSGSTR(ACLD_14, "ACL: message device initialization done\n"));
	fflush(dbgfile);
#endif

	/* Open the ACL Database that contains the mapping of ACL Tags to
	   IRs and vice versa.						*/

	if(spdbm_open(acl_config.dbase,DBM_RDWR,acl_config.buffers) != 0) {
		error(MSGSTR(ACLD_15, "ACL: error on database open"));
		exit(1);
	}

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(ACLD_16, "ACL: database file opened\n"));
	fflush(dbgfile);
#endif

	/* Fork the Security Policy Daemon */

	if((ret = fork()) == -1) {
		error(MSGSTR(ACLD_17, "ACL: Error on fork"));
		exit(1);
	}
	else if(ret != 0)
		exit(0);

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(ACLD_18, "ACL: daemon process forked-process id %d\n"),getpid());
	fflush(dbgfile);
#endif

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
	int cnt, aclsize;
	int i;
#if SEC_GROUPS
	int subjsize;
	int ngroups;
#endif

	mhdr = (mhdr_t *) msg_buffer;

	while(1) {

		/* Read the next message from the ACL device for processing */

		if((cnt = read(acl_device,msg_buffer,MAX_MESSAGE_SIZE)) == -1) {
#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(ACLD_22, "ACL: error or device read %d\n"),errno);
			fflush(dbgfile);
#endif
			error(MSGSTR(ACLD_23, "ACL: error on read of acl device"));
			continue;
		}

	/* Check the validity of the message size just read */

		if((cnt < 0) || (cnt < sizeof(mhdr_t)) ||
		   (cnt > MAX_MESSAGE_SIZE)) {
#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(ACLD_24, "ACL: invalid msg size %d\n"),cnt);
			fflush(dbgfile);
#endif
			continue;
		}

#ifdef DEBUG
		fprintf(dbgfile,MSGSTR(ACLD_25, "ACL: message type: %d sync id: %d\n"),
			mhdr->msg_type,mhdr->sync_id);
		fflush(dbgfile);
#endif

		switch(mhdr->msg_type) {

		   case SPD_MAP_TAG:

			/* Map the IR to a Tag and Return it */

			map_tag = (struct spd_map_tag *) msg_buffer;
			set_tag = (struct spd_set_tag *) msg_buffer;
#if SEC_GROUPS
			/* for systems supporting multiple groups,
			 * subject is handled differently */
			if (map_tag->ir_type == SEC_SUBJECT)
				goto map_subject;
#endif
			aclir = (struct acl_ir *) &map_tag->ir;

			/* Copy the ACL entries to the buffer and prepend
			   the ACL object owner and creator value to the
			   ACL entries for the IR.			*/

			memcpy((ulong) acl + ACL_HEADERSIZE,&aclir->ir,
			   aclir->ir_length);

			aclhdr = (acl_t *) acl;

			aclhdr->acl_head = map_tag->unixdac;

#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(ACLD_26, "ACL: map tag for: %d %d length: %d\n"),
			   aclhdr->acl_head.uid,aclhdr->acl_head.gid,
			   aclir->ir_length);
			fflush(dbgfile);
#endif

			/* Map the IR to a Tag for the Object */

			if(spdbm_tag_allocate(acl,aclir->ir_length +
			   ACL_HEADERSIZE,&acl_tag,0) == -1) {
				error(MSGSTR(ACLD_27, "ACL: unable to map ir to tag\n"));
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
			fprintf(dbgfile,MSGSTR(ACLD_28, "ACL: set tag: %x\n"),acl_tag);
			fflush(dbgfile);
#endif

			set_tag->tag = acl_tag;

			/*
			 * Object discretionary does not change in this
			 * implementation of access control lists
			 */

			write(acl_device,mhdr,SPD_SET_TAG_SIZE);
			continue;
#if SEC_GROUPS
			/* Map a subject IR to a tag - multiple groups */
map_subject:
			subjir = (dacid_t *) (map_tag + 1);
			memcpy(id, subjir, map_tag->ir.ir_length);
#ifdef DEBUG
			fprintf(dbgfile,
			  MSGSTR(ACLD_29, "ACL: map subject tag for uid %d gid %d groups"),
			  subjir->uid, subjir->gid);
			ngroups = (map_tag->ir.ir_length -
				    sizeof(subjir->uid) - sizeof(subjir->gid)) /
				    sizeof(subjir->groups[0]);
			for (i = 0; i < ngroups; i++) {
				fprintf(dbgfile, " %d", subjir->groups[i]);
			}
			putc('\n', dbgfile);
			fflush(dbgfile);
#endif
			/* Map the IR to a Tag for the Subject */

			if (spdbm_tag_allocate(id, map_tag->ir.ir_length,
			    &subj_tag, 0) == -1) {
				error(MSGSTR(ACLD_30, "ACL: unable to map subj ir to tag\n"));
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
			fprintf(dbgfile, MSGSTR(ACLD_31, "ACL: set subject tag: %x\n"),
			  subj_tag);
			fflush(dbgfile);
#endif
			set_tag->tag = subj_tag;
			write(acl_device, mhdr, SPD_SET_TAG_SIZE);
			continue;
#endif

		   case SPD_GET_ATTRIBUTE:

			/* Map the Tag to its Internal Representation */

			get_attr = (struct spd_get_attribute *) msg_buffer;
			int_rep = (struct spd_internal_rep *) msg_buffer;

			/* Save the tag value from message for return */

			acl_tag = get_attr->tag;

#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(ACLD_32, "ACL: get attr for: %x\n"),acl_tag);
			fflush(dbgfile);
#endif

			if(spdbm_get_ir(&get_attr->tag,acl,
			   &int_rep->ir.ir_length) == -1) {
				error(MSGSTR(ACLD_33, "ACL: unable to map tag to ir\n"));
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
				(ulong) acl + ACL_HEADERSIZE,
				int_rep->ir.ir_length - ACL_HEADERSIZE);

			int_rep->ir.ir_length -= ACL_HEADERSIZE;

			/* Build a message with Mapped IR and Return it */

			mhdr->msg_type = SPD_INTERNAL_REP;
			mhdr->error_code = SPD_OK;
			mhdr->mh_flags = 0;
			int_rep->tag = acl_tag;

			write(acl_device,mhdr,SPD_INTERNAL_REP_SIZE + 
				int_rep->ir.ir_length);
			continue;


		   case SPD_MAKE_DECISION:

		/* Retrieve IRs and Make Access Decision */

			decision = (struct spd_decision *) msg_buffer;
			make_dec = (struct spd_make_decision *) msg_buffer;

		/* Save the tag value from message for return */

#if SEC_GROUPS
			subj_tag = make_dec->subject;
#else
			subj_tag = *((dacid_t *) &make_dec->subject);
#endif
			obj_tag = make_dec->object;

#ifdef DEBUG
#if SEC_GROUPS
			fprintf(dbgfile,
			   MSGSTR(ACLD_34, "ACL: decision on: subj tag: %x obj_tag: %x\n"),
			   subj_tag,obj_tag);
#else
			fprintf(dbgfile,MSGSTR(ACLD_35, "ACL: decision on: %d %d tag: %x\n"),
			   subj_tag.uid,subj_tag.gid,obj_tag);
#endif
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

		/* Make the Access Decision between the IRs and return it */

			mhdr->msg_type = SPD_DECISION;
			mhdr->error_code = SPD_OK;
			mhdr->mh_flags = 0;


		/* Adjust the acl size to account for ACL header ids */

			aclsize -= ACL_HEADERSIZE;

		/* Multiple group access decision is based on the IR from the
		   for the Uid and Supplementary Groups whereas the System
		   V decision uses the subject tag as is with Uid/Gid.  */

#if SEC_GROUPS
			make_decision(id,acl,&decision->decision,
			  aclsize,subjsize);
			decision->subject = subj_tag;
#else
			make_decision(&subj_tag,acl,&decision->decision,
			  aclsize,0);
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

			if(spdbm_get_ir(&chg_discr->object_tag,acl,&aclsize)
			     == -1) {
				error(MSGSTR(ACLD_37, "ACL: can't map object tag\n"));
				mhdr->error_code = SPD_ENOOBJTAG;
			}

			/* Replace the appropriate components with the
			 * new values for the owner, group, and mode.
			 * if none changed, save the work.
			 */

			else {
				acl->acl_head.uid  = chg_discr->object_dac.uid;
				acl->acl_head.gid  = chg_discr->object_dac.gid;
				acl->acl_head.mode = chg_discr->object_dac.mode;

				if (spdbm_tag_allocate(acl, aclsize,
				    &new_discr->object_tag, 0) == -1)
					mhdr->error_code = SPD_ENOOBJTAG;
			}

			/* SecureWare ACL policy does not affect UNIX dac */

			new_discr->object_flags = 0;

			mhdr->msg_type = SPD_NEW_DISCR;
			mhdr->error_code = SPD_OK;
			mhdr->mh_flags = 0;

			write(acl_device, mhdr, SPD_NEW_DISCR_SIZE);
			continue;

		   case SPD_PREPARE_SHUTDOWN:

			/* Daemon to be terminated */

#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(ACLD_38, "ACL: shutdown notify message\n"));
			fflush(dbgfile);
#endif
			mhdr->msg_type = SPD_PREPARE_RESP;
			mhdr->mh_flags = 0;
			mhdr->error_code = SPD_OK;
			write(acl_device,mhdr,SPD_MHDR_SIZE);
			continue;

		   case SPD_SHUTDOWN:

			/* Terminate daemon execution */

#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(ACLD_39, "ACL: shutdown message\n"));
			fflush(dbgfile);
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
			fprintf(dbgfile,MSGSTR(ACLD_40, "ACL: invalid message type %d\n"),
				mhdr->msg_type);
			fflush(dbgfile);
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
	make_decision()-this routine is called passing the subject id values
	and the ACL entries. Each ACL entry is checked until a match is
	found or the end of the list is reached. A match occurs iff both
	the user and group components match.

	1. If entry uid is a specific user id, it must match process'
	   effective user id.
	2. If entry uid is ACL_OWNER, the object owner ids must match the
	   process' effective user id.
	3. If the entry is ACL_WILDCARD, a match exists.
	4. If entry gid is a specific group id, it must match process'
	   effective group id.
	5. If entry gid is ACL_OWNER, the object group ids must match the
	   process' effective group id. (or supplementary gids)
	6. If the entry is ACL_WILDCARD, a match exists.

	In this implementation, the self/group/public decision is made
	in the policy module in the kernel.
*/

make_decision(subjid,ir,decision,aclsize, subjsize)
dacid_t *subjid;
acl_t *ir;
acl_dec_t *decision;
int aclsize;
int subjsize;
{
	int i, acl_entries = aclsize / sizeof(acle_t);
#if SEC_GROUPS
	int j;
	int ngroups;
#endif
	register acle_t *acl_entry;

	/* Cycle through each ACL entry looking for one that matches
	   the above criteria or until the entries are exhausted. */

	acl_entry = (acle_t *) (ir + 1);

	for(i = 0; i < acl_entries; i++, acl_entry++) {

	/* Perform the access checks based on the User Id first */

	   switch(acl_entry->acl_uid) {

		case (uid_t) ACL_OWNER:		/* must match object owner */

			if((subjid->uid == ir->acl_head.uid) ||
			   (subjid->uid == ir->acl_head.cuid))
				break;
			else continue;

		case (uid_t) ACL_WILDCARD:

			break;		/* automatic match */

		default:		/* specific user id */

			if(subjid->uid == acl_entry->acl_uid)
				break;
			else continue;

	   }

#if SEC_GROUPS
	/* Compute the number of groups from the IR size */

	ngroups = (subjsize - sizeof(id->uid) - sizeof(id->gid))
			/ sizeof(id->groups[0]);
#endif

	/* Perform the access checks based on the Group Id */

	   switch(acl_entry->acl_gid) {

		case (gid_t) ACL_OWNER:		/* must match object group */

		    if((subjid->gid == ir->acl_head.gid) ||
		       (subjid->gid == ir->acl_head.cgid))
			goto found;

#if SEC_GROUPS
		    for(j=0; j < ngroups; j++)
			if((subjid->groups[j] == ir->acl_head.gid) ||
			   (subjid->groups[j] == ir->acl_head.cgid))
				goto found;
#endif
		    continue;

		case (gid_t) ACL_WILDCARD:

			goto found;		/* automatic match */

		default:		/* specific group id */

		    if(subjid->gid == acl_entry->acl_gid)
			goto found;

#if SEC_GROUPS
		    for(j=0; j < ngroups; j++)
			if(subjid->groups[j] == acl_entry->acl_gid)
				goto found;
#endif
		    continue;

	   }

	}

	/* No entries in the ACL permitted access for this user */

	decision->dec.read = 0;
	decision->dec.write = 0;
	decision->dec.exec = 0;
	decision->dec.delete = 0;
	decision->dec.rfu = 0;

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(ACLD_41, "ACL: no access permitted\n"));
	fflush(dbgfile);
#endif

	return(0);

found:

	/* Found an entry that matches the user and group id */

	decision->dec.read = ((acl_entry->acl_perm & ACL_READ) != 0);
	decision->dec.write = ((acl_entry->acl_perm & ACL_WRITE) != 0);
	decision->dec.exec = ((acl_entry->acl_perm & ACL_EXEC) != 0);
	decision->dec.delete = 0;
	decision->dec.rfu = 0;

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(ACLD_42, "ACL: decision Read: %d Write: %d Exec: %d\n"),
		decision->dec.read,decision->dec.write,decision->dec.exec);
	fflush(dbgfile);
#endif

	return(0);

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
	fprintf(dbgfile,MSGSTR(ACLD_43, "ACL: SIGTERM received for shutdown\n"));
	fflush(dbgfile);
#endif
	if(spdbm_reopen(acl_config.dbase,DBM_RDONLY) != 0) {
		error(MSGSTR(ACLD_44, "ACL: error on database reopen"));
		exit(1);
	}

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(ACLD_45, "ACL: daemon reopened dbase for RDONLY\n"));
	fflush(dbgfile);
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
	fflush(dbgfile);
	fclose(dbgfile);

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

#endif /* SEC_ACL_SWARE */
