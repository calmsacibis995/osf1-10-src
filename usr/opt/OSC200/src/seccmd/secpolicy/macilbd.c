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
static char	*sccsid = "@(#)$RCSfile: macilbd.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 15:07:41 $";
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

/*	Copyright (c) 1989-90 SecureWare, Inc.

	This is proprietary source code of SecureWare, Inc.
	All rights reserved.
*/



/*
 * Based on:

 */

#include <sys/secdefines.h>

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

#if SEC_MAC /*{*/

#include	<sys/types.h>

#include	<stdio.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<pwd.h>
#include	<grp.h>

#include	<sys/signal.h>
#include	<sys/errno.h>
#include	<sys/security.h>
#include	<mandatory.h>
#include	<sys/secpolicy.h>
#include	<sys/secioctl.h>
#include	"spdbm.h"
#ifndef _OSF_SOURCE
#include	<sys/lock.h>
#endif

/* Mandatory Access Control and Information Labeling Security Policy Daemon
 *
 * This daemon is initiated by the policy spawning process at
 * system startup. The daemon is responsible for reading the
 * minor device for MAC to obtain messages and to respond to
 * the messages via the same minor device. Once initialized,
 * the daemon continually reads the device and responds until
 * it is requested to shutdown.
 *
 * The daemon is responsible for all Mandatory Access Control
 * policy decisions and for combining information labels. It
 * produces both a decision and a combination when asked for
 * an access determination. The decision/combination is returned
 * to the corresponding MAC kernel module to reflect the decision
 * to the caller of the access check.
 */

#define LINE_LENGTH		80
#define MAX_MESSAGE_SIZE	(1024 * 16)

#ifdef DEBUG
#define DEBUG_START		"/tcb/files/MAC_LOG"
#define DEBUG_LAST		"/usr/tmp/MAC_LOG"
#define DEBUG_ER_BUFFER		1024

FILE *dbgfile;
mcache_dec_t *macdec;
char dbg_er_buffer[DEBUG_ER_BUFFER];
#endif

#define Syslo_tag(src)		(src == SEC_MAC_SYSLO_TAG)
#define Syshi_tag(src)		(src == SEC_MAC_SYSHI_TAG)

void bad_syscall();
void terminate();
void debug_switch();

/* Pointers and Buffers for IRs retrieved for Labels under comparison */

struct	mac_ir *macir;

struct	mac_ir *ir1;
struct	mac_ir *ir2;
#if SEC_ILB
struct	mac_ir *ir3;
#endif

struct	sp_init sp_init;

/* Pointers to the Various Message Types */

struct	spd_map_tag *map_tag;
struct	spd_set_tag *set_tag;
struct	spd_make_decision *make_dec;
struct	spd_decision *decision;
struct	spd_get_attribute *get_attr;
struct	spd_internal_rep *int_rep;

struct	passwd *getpwnam();
struct	group *getgrnam();

char	dev_file[LINE_LENGTH];
char	in_line[LINE_LENGTH];

/* Message Buffer and Message Header Pointer */

mhdr_t	*mhdr;
char	msg_buffer[MAX_MESSAGE_SIZE];

/* Tag Buffer and Null Tag Definition */

tag_t	mac_tag;
tag_t	subj_tag;
tag_t	obj_tag;

tag_t	null_tag =  SEC_MAC_NULL_TAG;
tag_t	syslo_tag = SEC_MAC_SYSLO_TAG;
tag_t	syshi_tag = SEC_MAC_SYSHI_TAG;

/* Define the structures for System Hi and System Lo mac_irs */

struct	mac_ir	*System_Hi;
struct	mac_ir	*System_Lo;

/* Return defines for pattern matching for these System IRs */

#define SYSLO_IR	1
#define SYSHI_IR	2

int	ir_size = 0;
int	cat_length = 0;
int	mac_device = 0;
int	slb_length = 0;
#if SEC_ILB
int	mark_length = 0;
int	ilb_length = 0;
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

#ifdef DEBUG
	if((dbgfile = fopen(DEBUG_START,"w")) == NULL)
		error(MSGSTR(MACILBD_1, "Error on open of MAC logfile"));

	fprintf(dbgfile,MSGSTR(MACILBD_2, "MAC: daemon initiated\n"));
#endif

	/* Catch SIGUSR1 to indicate a debug file switch after fileystem
	   mounting; SIGTERM to indicate shutdown to reopen the dtabase
	   Read only, and SIGSYS to indicate not running secure kernel. */

	signal(SIGUSR1,debug_switch);
	signal(SIGSYS,bad_syscall);
	signal(SIGTERM,terminate);

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
		error(MSGSTR(MACILBD_3, "MAC daemon: cannot find uid tcb.\n"));
	else {
		setluid(pw->pw_uid);
	}

	/* Find the tcb group entry in the /etc/group file and do the
	   setgid() for proper file permissions.		*/

	if((gr = getgrnam("tcb")) == NULL)
		error(MSGSTR(MACILBD_4, "MAC daemon: cannot find group tcb.\n"));
	else
		setgid(gr->gr_gid);

	/* Set the user id after group, otherwise setgid() will fail */

	setuid(pw->pw_uid);

	/* Call the mandatory init routine which will copy the init
	   parameters for the policy into mand_config and also will
	   build out the sp_init structure. The structure is built
	   during the ioctl(2) for SPIOC_INIT done in the library.
	   The routine returns 1 if the policy is configured, -1 if
	   an error occurred or 0 if the policy is not in effect.  */

	switch(mand_init_daemon(&sp_init)) {

	   case 0:		/* Policy configured */

		error(MSGSTR(MACILBD_5, "Mandatory access control is configured.\n"));
#if SEC_ILB
		error(MSGSTR(MACILBD_6, "Information labels are configured.\n"));
#endif
		break;

	   default:		/* Policy not configured or Error occurred */

		exit(0);

	}

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(MACILBD_7, "MAC: policy parameters: policy %d minor %d\n"),
		sp_init.policy,sp_init.spminor);
	fprintf(dbgfile,MSGSTR(MACILBD_8, "MAC: policy parameters: subj_tag %d obj_tag %d\n"),
		sp_init.first_subj_tag,sp_init.first_obj_tag);
	fprintf(dbgfile,MSGSTR(MACILBD_9, "MAC: policy parameters: subj_tags %d obj_tags %d\n"),
		sp_init.subj_tag_count,sp_init.obj_tag_count);
	fflush(dbgfile);
#endif

	/* Using the return value of the minor device in the sp_init
	   structure, build the device name and open the daemon
	   message device.					  */

	minordev = '0' + sp_init.spminor;
	sprintf(dev_file,"%s%c",SP_DAEMON_DEVICE,minordev);

	if((mac_device = open(dev_file,O_RDWR)) == -1) {
		error(MSGSTR(MACILBD_10, "MAC daemon: error on open of spd device"));
		exit(1);
	}

	/* Fill in the values not returned from ioctl and init */

	sp_init.cache_size = mand_config.cache_size;
	sp_init.cache_width = MAC_CACHE_WIDTH;

	/* Do the ioctl to initialize the policy and the cache */

	if(ioctl(mac_device,SPIOC_INIT,&sp_init) == -1) {
		error(MSGSTR(MACILBD_11, "MAC daemon: error on SPIOC_INIT of policy"));
		exit(1);
	}

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(MACILBD_12, "MAC: policy init routine called\n"));
	fflush(dbgfile);
#endif

        mand_init();


	/* Allocate space for the Subject and Object IRs that are returned
	   from the database to perform the actual comparison of levels. */

#if SEC_ILB
	cat_length = CATWORDS * sizeof(mask_t);
	mark_length = MARKWORDS * sizeof(mask_t);
	slb_length = sizeof(long) + cat_length;
	ilb_length = slb_length + mark_length;
	ir_size = MAC_IR_SIZE + cat_length + mark_length;
#else
	cat_length = (mand_max_cat + 7) / 8;
	slb_length = sizeof(long) + cat_length;
	ir_size = MAC_IR_SIZE + cat_length;
#endif

	/* Declare the mac_ir buffers for IR decisions on SPD_MAKE_DECISION */

	if((ir1 = (struct mac_ir *) malloc(ir_size)) == (struct mac_ir *) 0) {
		error(MSGSTR(MACILBD_13, "MAC daemon: unable to alloc space for subject ir\n"));
		exit(1);
	}

	if((ir2 = (struct mac_ir *) malloc(ir_size)) == (struct mac_ir *) 0) {
		error(MSGSTR(MACILBD_14, "MAC daemon: unable to alloc space for object ir\n"));
		exit(1);
	}

#if SEC_ILB
	if((ir3 = (struct mac_ir *) malloc(ir_size)) == (struct mac_ir *) 0) {
		error(MSGSTR(MACILBD_15, "MAC daemon: unable to alloc space for combination ir\n"));
		exit(1);
	}
#endif

	/* Declare the mac_ir buffers for the System_Hi and System_Lo IRs */

	if((System_Lo = (struct mac_ir *) malloc(ir_size)) == 
	   (struct mac_ir *) 0) {
		error(MSGSTR(MACILBD_16, "MAC daemon: unable to malloc space for System Lo ir\n"));
		exit(1);
	}

	if((System_Hi = (struct mac_ir *) malloc(ir_size)) == 
	   (struct mac_ir *) 0) {
		error(MSGSTR(MACILBD_17, "MAC daemon: unable to malloc space for System Hi ir\n"));
		exit(1);
	}

	/* The internal representations for both System Lo and Hi are now
	 * available after the mand_init_daemon() call. Those labels are in
	 * mand_ir_t format and they need to be in mac_ir format. Convert
	 * them here so that they can be used in valid_label().		*/

#if SEC_ILB
	memcpy(&System_Lo->class, mand_syslo, ilb_length);
	memcpy(&System_Hi->class, mand_syshi, ilb_length);
#else
	memcpy(&System_Lo->class, mand_syslo, slb_length);
	memcpy(&System_Hi->class, mand_syshi, slb_length);
#endif

	/* Set the mac_ir structure sizes */

#if SEC_ILB
	System_Lo->ir_length = ilb_length;
	System_Hi->ir_length = ilb_length;
#else
	System_Lo->ir_length = slb_length;
	System_Hi->ir_length = slb_length;
#endif

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(MACILBD_18, "MAC: message device initialization done\n"));
	fflush(dbgfile);
#endif

	/* Open the MAC Database that contains the mapping of MAC Tags to
	   IRs and vice versa.						*/

	if(spdbm_open(mand_config.dbase,DBM_RDWR,mand_config.buffers) != 0) {
		error(MSGSTR(MACILBD_19, "MAC daemon: error on database open"));
		exit(1);
	}

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(MACILBD_20, "MAC: database file opened\n"));
	fflush(dbgfile);
#endif

	/* Fork the Security Policy Daemon */

	if((ret = fork()) == -1) {
		error(MSGSTR(MACILBD_21, "MAC daemon: Error on fork"));
		exit(1);
	}
	else if(ret != 0)
		exit(0);

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(MACILBD_22, "MAC: daemon process forked-process id %d\n"),getpid());
	fflush(dbgfile);
#endif

	/* Reset the daemon message minor to an exclusive device */

	if(ioctl(mac_device,SPIOC_EXCL,0,0) == -1) {
		error(MSGSTR(MACILBD_23, "MAC daemon: error on EXCL ioctl"));
		exit(1);
	}

#ifndef _OSF_SOURCE
	/* Lock the Daemon Process Text Pages into Memory */

	if(plock(TXTLOCK) == -1) {
#ifdef DEBUG
		fprintf(dbgfile,MSGSTR(MACILBD_24, "MAC: daemon process text lock fail-%d\n"),errno);
		fflush(dbgfile);
#endif
	}
	else {
#ifdef DEBUG
		fprintf(dbgfile,MSGSTR(MACILBD_25, "MAC: daemon process text locked\n"));
		fflush(dbgfile);
#endif
	}
#endif /* _OSF_SOURCE */

	/* Process requests from Message Driver until Shutdown */

	process_mac();
	exit(0);

}

/*
	process_mac()-Continually read the MAC message device retrieving
	message requests from the kernel module and possibly from other
	clients. These messages are acted upon and the return message is
	generated and sent to the kernel using a write(2) on the device.
*/

process_mac()
{
	int irtype, cnt;

	mhdr = (mhdr_t *) msg_buffer;

	while(1) {

		/* Read the next message from the Mac device for processing */

		if((cnt = read(mac_device,msg_buffer,MAX_MESSAGE_SIZE)) == -1) {
#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(MACILBD_26, "MAC: device read error %d\n"),errno);
			fflush(dbgfile);
#endif
			error(MSGSTR(MACILBD_27, "MAC daemon: error on read of mac device"));
			continue;
		}

	/* Validate the size of the message just read from the device */

		if((cnt < 0) || (cnt < sizeof(mhdr_t)) ||
		   (cnt > MAX_MESSAGE_SIZE)) {
#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(MACILBD_28, "MAC: read size error %d\n"),cnt);
			fflush(dbgfile);
#endif
			continue;
		}

#ifdef DEBUG
		fprintf(dbgfile,MSGSTR(MACILBD_29, "MAC message type: %d sync id: %d\n"),
			mhdr->msg_type,mhdr->sync_id);
		fflush(dbgfile);
#endif

		switch(mhdr->msg_type) {

		   case SPD_MAP_TAG:

			map_tag = (struct spd_map_tag *) msg_buffer;
			set_tag = (struct spd_set_tag *) msg_buffer;
			macir = (struct mac_ir *) &map_tag->ir;

			/* Check for SysHi or SysLo IRs */

			if((irtype = system_ir(macir)) != 0) {

				/* Build message with well-known tag */

				mhdr->msg_type = SPD_SET_TAG;
				mhdr->error_code = SPD_OK;
				mhdr->mh_flags = 0;
#ifdef DEBUG
				fprintf(dbgfile,MSGSTR(MACILBD_30, "MAC System IR map tag-%d\n"),
					irtype);
				fflush(dbgfile);
#endif
				if(irtype == SYSLO_IR)	
					set_tag->tag = syslo_tag;
				else
					set_tag->tag = syshi_tag;

				write(mac_device,mhdr,SPD_SET_TAG_SIZE);
				continue;
			}

			/* Check for label range validity */

			if(!valid_label(macir)) {
				mhdr->msg_type = SPD_SET_TAG;
				mhdr->error_code = SPD_EBADIREP;
				mhdr->mh_flags = 0;
#ifdef DEBUG
				fprintf(dbgfile,MSGSTR(MACILBD_31, "MAC set tag error EBADIREP\n"));
				fflush(dbgfile);
#endif
				set_tag->tag = null_tag;

				write(mac_device,mhdr,SPD_SET_TAG_SIZE);
				continue;
			}

			/* Map the IR to a tag */

			if(spdbm_tag_allocate(&macir->class,macir->ir_length,
			   &mac_tag,0) == -1) {
#ifdef ERROR_LOG
				error(MSGSTR(MACILBD_32, "MAC daemon: unable to map ir to tag\n"));
#endif
#ifdef DEBUG
				fprintf(dbgfile,MSGSTR(MACILBD_33, "MAC set tag error EFULLDB\n"));
				fflush(dbgfile);
#endif
				mhdr->msg_type = SPD_SET_TAG;
				mhdr->error_code = SPD_EFULLDB;
				mhdr->mh_flags = 0;
				set_tag->tag = null_tag;

				write(mac_device,mhdr,SPD_SET_TAG_SIZE);
				continue;
			}

			/* Build a message with new Tag and Return it */

			mhdr->msg_type = SPD_SET_TAG;
			mhdr->error_code = SPD_OK;
			mhdr->mh_flags = 0;

#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(MACILBD_34, "MAC set tag: %x\n"),mac_tag);
			fflush(dbgfile);
#endif

			set_tag->tag = mac_tag;
			write(mac_device,mhdr,SPD_SET_TAG_SIZE);
			continue;

		   case SPD_GET_ATTRIBUTE:

			/* Map the Tag to its Internal Representation */

			get_attr = (struct spd_get_attribute *) msg_buffer;
			int_rep = (struct spd_internal_rep *) msg_buffer;

			/* Check for well-known tags to avoid lookup */

			if((Syslo_tag(get_attr->tag)) || (Syshi_tag(get_attr->tag))) {
				/* Build a message and return known IR */

				mhdr->msg_type = SPD_INTERNAL_REP;
				mhdr->error_code = SPD_OK;
				mhdr->mh_flags = 0;
				int_rep->tag = get_attr->tag;
#ifdef DEBUG
				fprintf(dbgfile,MSGSTR(MACILBD_35, "MAC system attribute %x\n"),
					get_attr->tag);
				fflush(dbgfile);
#endif
				if(Syslo_tag(get_attr->tag))
					memcpy(&int_rep->ir, System_Lo, ir_size);
				else
					memcpy(&int_rep->ir, System_Hi, ir_size);

				write(mac_device,mhdr,SPD_INTERNAL_REP_SIZE + 
					int_rep->ir.ir_length);
				continue;
			}

			/* Save the tag value from message for return */

			mac_tag = get_attr->tag;

#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(MACILBD_36, "MAC get attr for: %x\n"),mac_tag);
			fflush(dbgfile);
#endif

			if(spdbm_get_ir(&get_attr->tag,&int_rep[1],
					&int_rep->ir.ir_length) == -1) {
#ifdef ERROR_LOG
				error(MSGSTR(MACILBD_37, "MAC daemon: unable to map tag to ir\n"));
#endif
				mhdr->msg_type = SPD_INTERNAL_REP;
				mhdr->error_code = SPD_EBADIREP;
				mhdr->mh_flags = 0;
				int_rep->ir.ir_length = 0;
#ifdef DEBUG
				fprintf(dbgfile,MSGSTR(MACILBD_31, "MAC set tag error EBADIREP\n"));
				fflush(dbgfile);
#endif

				int_rep->tag = mac_tag;
				write(mac_device,mhdr,SPD_INTERNAL_REP_SIZE);
				continue;
			}

			/* Build a message with Mapped IR and Return it */

			mhdr->msg_type = SPD_INTERNAL_REP;
			mhdr->error_code = SPD_OK;
			mhdr->mh_flags = 0;
			int_rep->tag = mac_tag;
#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(MACILBD_38, "MAC get attribute OK-size %d\n"),
				int_rep->ir.ir_length);
			fflush(dbgfile);
#endif

			write(mac_device,mhdr,SPD_INTERNAL_REP_SIZE + 
				int_rep->ir.ir_length);
			continue;


		   case SPD_MAKE_DECISION:

		/* Retrieve IRs and Make Access Decision */

			decision = (struct spd_decision *) msg_buffer;
			make_dec = (struct spd_make_decision *) msg_buffer;

		/* Save the tag value from message for return */

			subj_tag = make_dec->subject;
			obj_tag = make_dec->object;

#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(MACILBD_39, "MAC decision: subj: %x obj: %x\n"),
				subj_tag,obj_tag);
			fflush(dbgfile);
#endif

		/* Lookup the IRs for the Tags */

			if(Syslo_tag(subj_tag) || Syshi_tag(subj_tag)) {
			    if(Syslo_tag(subj_tag))
				memcpy(ir1, System_Lo, ir_size);
			    else
				memcpy(ir1, System_Hi, ir_size);
			}
			else if(spdbm_get_ir(&make_dec->subject,&ir1->class,
			   &ir1->ir_length) == -1) {
#ifdef ERROR_LOG
				error(MSGSTR(MACILBD_40, "MAC daemon: unable to map subject tag\n"));
#endif
				mhdr->msg_type = SPD_DECISION;
				mhdr->error_code = SPD_ENOSUBJTAG;
				mhdr->mh_flags = 0;
#ifdef DEBUG
				fprintf(dbgfile,MSGSTR(MACILBD_41, "MAC decision error ENOSUBJTAG\n"));
				fflush(dbgfile);
#endif

		/* Restore tag values and write return error message */

				decision->subject = subj_tag;
				decision->object = obj_tag;
				decision->decision.dec_length = 0;
				write(mac_device,mhdr,SPD_DECISION_SIZE);
				continue;
			}

			if(Syslo_tag(obj_tag) || Syshi_tag(obj_tag)) {
			    if(Syslo_tag(obj_tag))
				memcpy(ir2, System_Lo, ir_size);
			    else
				memcpy(ir2, System_Hi, ir_size);
			}
			else if(spdbm_get_ir(&make_dec->object,&ir2->class,
			   &ir2->ir_length) == -1) {
#ifdef ERROR_LOG
				error(MSGSTR(MACILBD_42, "MAC daemon: unable to map object tag\n"));
#endif
				mhdr->msg_type = SPD_DECISION;
				mhdr->error_code = SPD_ENOOBJTAG;
				mhdr->mh_flags = 0;
#ifdef DEBUG
				fprintf(dbgfile,MSGSTR(MACILBD_43, "MAC decision error ENOOBJTAG\n"));
				fflush(dbgfile);
#endif

		/* Restore tag values and write return error message */

				decision->subject = subj_tag;
				decision->object = obj_tag;
				decision->decision.dec_length = 0;
				write(mac_device,mhdr,SPD_DECISION_SIZE);
				continue;
			}

		/* Make the Access Decision between the IRs and return it */

			mhdr->msg_type = SPD_DECISION;
			mhdr->error_code = SPD_OK;
			mhdr->mh_flags = 0;

			make_decision(ir1,ir2,&decision->decision);

			decision->subject = subj_tag;
			decision->object = obj_tag;
			write(mac_device,mhdr,SPD_DECISION_SIZE + 
				sizeof(mcache_dec_t));
			continue;

		   case SPD_PREPARE_SHUTDOWN:

			/* Daemon to be terminated */

#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(MACILBD_44, "MAC: shutdown notify message\n"));
			fflush(dbgfile);
#endif
			mhdr->msg_type = SPD_PREPARE_RESP;
			mhdr->mh_flags = 0;
			mhdr->error_code = SPD_OK;
			write(mac_device,mhdr,SPD_MHDR_SIZE);
			continue;

		   case SPD_SHUTDOWN:

			/* Terminate daemon execution */

			spdbm_close();

#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(MACILBD_45, "MAC: shutdown message\n"));
			fflush(dbgfile);
#endif
			mhdr->msg_type = SPD_SHUTDOWN_RESP;
			mhdr->mh_flags = 0;
			mhdr->error_code = SPD_OK;
			write(mac_device,mhdr,SPD_MHDR_SIZE);

			close(mac_device);
			exit(0);

		   default:

			/* Invalid Message Type */

#ifdef DEBUG
			fprintf(dbgfile,MSGSTR(MACILBD_46, "MAC: invalid message type %d\n"),
				mhdr->msg_type);
			fflush(dbgfile);
#endif
			mhdr->msg_type = SPD_ERROR;
			mhdr->mh_flags = 0;
			mhdr->error_code = SPD_EBADMSG;
			write(mac_device,mhdr,SPD_MHDR_SIZE);
			continue;

		}
	}
}

/*
	make_decision()-fill in the Mandatory Decision table based on
	the two Internal Representations passed for comparison.
*/

make_decision(ir1,ir2,decision)
struct mac_ir *ir1, *ir2;
mac_dec_t *decision;
{
	decision->dec_length   = MAC_DECISION_SIZE;
	decision->dec.rfu      = 0;

	if (decision->dec.same = equivalence(ir1,ir2,slb_length)) {
		decision->dec.subj_dom = 0;
		decision->dec.obj_dom = 0;
		decision->dec.incomp = 0;
	} else if (decision->dec.subj_dom = dominates(ir1,ir2,slb_length)) {
		decision->dec.obj_dom = 0;
		decision->dec.incomp = 0;
	} else if (decision->dec.obj_dom = dominates(ir2,ir1,slb_length)) {
		decision->dec.incomp = 0;
	} else {
		decision->dec.incomp = 1;
	}
#if SEC_ILB
	if (decision->dec.ilb_same = equivalence(ir1,ir2,ilb_length)) {
		decision->dec.ilb_subj_dom = 0;
		decision->dec.ilb_obj_dom = 0;
	} else if (decision->dec.ilb_subj_dom = dominates(ir1,ir2,ilb_length))
		decision->dec.ilb_obj_dom = 0;
	else
		decision->dec.ilb_obj_dom = dominates(ir2,ir1,ilb_length);
	make_combination(ir1, ir2, decision);
#endif

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(MACILBD_47, "Subj IR (%d bytes): "), ir1->ir_length); dump_ir(ir1);
	fprintf(dbgfile,MSGSTR(MACILBD_48, "Obj IR (%d bytes):  "), ir2->ir_length); dump_ir(ir2);
	fprintf(dbgfile,MSGSTR(MACILBD_49, "MAC decision: SDOM: %d ODOM: %d SAME: %d INCOMP: %d\n"),
		decision->dec.subj_dom,decision->dec.obj_dom,
		decision->dec.same,decision->dec.incomp);
#if SEC_ILB
	fprintf(dbgfile,MSGSTR(MACILBD_50, "ILB decision: SDOM: %d ODOM: %d SAME: %d"),
		decision->dec.ilb_subj_dom,decision->dec.ilb_obj_dom,
		decision->dec.ilb_same);
	fprintf(dbgfile, MSGSTR(MACILBD_51, " NOCOMB: %d Tag: %x\n"),
		decision->dec.nocomb, decision->combination);
#endif
	fflush(dbgfile);
#endif

	return(0);
}

#if SEC_ILB
/*
 * make_combination()-if both internal representations are information labels,
 * combine them and fill in the decision structure with the corresponding tag.
 */

make_combination(ir1,ir2,decision)
struct mac_ir *ir1, *ir2;
mac_dec_t *decision;
{
	register int i;
	register unchar *cats1 = (unchar *) &ir1[1];
	register unchar *cats2 = (unchar *) &ir2[1];
	register unchar *cats3 = (unchar *) &ir3[1];

	/* Verify that both IRs are for information labels */

	if (ir1->ir_length != ilb_length || ir2->ir_length != ilb_length) {
		decision->dec.nocomb = 1;
		decision->combination = null_tag;
		return;
	}

	/* Pick the dominant classification */

	ir3->ir_length = ilb_length;
	if (ir1->class >= ir2->class)
		ir3->class = ir1->class;
	else
		ir3->class = ir2->class;
	
	/* Combine the compartments and markings */

	i = ir3->ir_length - sizeof ir3->class;
	while (--i >= 0)
		*cats3++ = *cats1++ | *cats2++;
	
	/* Map the combined label to a tag */

	if ((i = system_ir(ir3)) != 0 ||
	    spdbm_tag_allocate(&ir3->class, ir3->ir_length,
				&decision->combination, 0) != -1) {
		if (i == SYSLO_IR)
			decision->combination = syslo_tag;
		else if (i == SYSHI_IR)
			decision->combination = syshi_tag;

		/* else not a system IR; tag assigned by spdbm_tag_allocate */

		decision->dec.nocomb = 0;
	} else {
#ifdef ERROR_LOG
		error(MSGSTR(MACILBD_52, "MAC daemon: unable to map combined ILB to tag\n"));
#endif
#ifdef DEBUG
		fprintf(dbgfile, MSGSTR(MACILBD_53, "MAC can't map ILB combination to tag\n"));
		fflush(dbgfile);
#endif
		decision->dec.nocomb = 1;
		decision->combination = null_tag;
	}
}
#endif

/*
	dominates()-determine if the first level dominates the second.
	The function returns a 1 if it does dominate else a 0. The
	dominance relationship exists iff the classification is greater and
	all categories in the second ir are contained in the first.
*/

dominates(ir1,ir2,length)
register struct mac_ir *ir1, *ir2;
register int length;
{
	register int i;
	register unchar *cats1 = (unchar *) ((ulong) ir1 + MAC_IR_SIZE);
	register unchar *cats2 = (unchar *) ((ulong) ir2 + MAC_IR_SIZE);

#if SEC_ILB
	if(ir1->ir_length < length || ir2->ir_length < length)
		return(0);
#endif
	if(ir1->class < ir2->class)
		return(0);

	for (i = length - sizeof ir1->class; --i >= 0; )
		if (~cats1[i] & cats2[i])
			return(0);

	return(1);
}

/*
	equivalence()-determine if the first level is equal to the second.
	The function returns a 1 if they are equal else a 0. The levels
	are considered equal iff the classifications are equal and every
	category in the first is in the second and vice versa.
*/

equivalence(ir1,ir2,length)
struct mac_ir *ir1, *ir2;
register int length;
{
#if SEC_ILB
	if(ir1->ir_length < length || ir2->ir_length < length)
		return(0);
#endif

	return memcmp(&ir1->class, &ir2->class, length) == 0;
}

/*
	valid_label()-this routines determines that the specified label is
	in the range of System Hi and System Lo. Returns 1 if a valid label
	else a 0.
*/

valid_label(ir)
struct mac_ir *ir;
{
	if((dominates(System_Hi,ir,ir->ir_length) == 0) &&
	   (equivalence(System_Hi,ir,ir->ir_length)) == 0)
		return(0);

	if((dominates(ir,System_Lo,ir->ir_length) == 0) &&
	   (equivalence(System_Lo,ir,ir->ir_length)) == 0)
		return(0);

	/* The IR is in the range of System Hi and System Lo */

#if SEC_CMW || SEC_SHW
	/*
	 * If the IR is a sensitivity label, make sure it falls
	 * withing the system's accreditation range.
	 */
	if (ir->ir_length == slb_length &&
	    !l_in_accreditation_range(ir->class, (mask_t *) &ir[1]))
		return 0;
#endif

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(MACILBD_54, "MAC valid label range check succeeded\n"));
	fflush(dbgfile);
#endif
	return(1);
}

/*
	system_ir()-check the internal representation presented to see if
	it matches one of the well-known System Lo or System Hi IRs that
	are stored by the daemon. This avoids having to go the database for
	tag mapping and decisions for these.
*/

system_ir(ir)
struct mac_ir *ir;
{
	if (memcmp(&ir->class, &System_Lo->class, ir->ir_length) == 0)
		return SYSLO_IR;
	if (memcmp(&ir->class, &System_Hi->class, ir->ir_length) == 0)
		return SYSHI_IR;
	return 0;
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
	terminate()-on receipt of SIGTERM, reopen the database for
	Read only. Only decisions for exisiting tags may be after
	that has been done. No new tags may be created. This preserves
	the integrity of the database.
*/

void
terminate()
{

	signal(SIGTERM,SIG_IGN);
#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(MACILBD_55, "MAC daemon: SIGTERM received for shutdown\n"));
	fflush(dbgfile);
#endif
	if(spdbm_reopen(mand_config.dbase,DBM_RDONLY) != 0) {
		error(MSGSTR(MACILBD_56, "MAC daemon: error on database reopen"));
		exit(1);
	}

#ifdef DEBUG
	fprintf(dbgfile,MSGSTR(MACILBD_57, "MAC daemon: reopened database for RDONLY\n"));
	fflush(dbgfile);
#endif

}

/*
	debug_switch()-switch to the user filesystem debug file
*/

void
debug_switch()
{
	signal(SIGUSR1,SIG_IGN);
#ifdef DEBUG
	fflush(dbgfile);
	fclose(dbgfile);

	if((dbgfile = fopen(DEBUG_LAST,"w")) == NULL) {
		error(MSGSTR(MACILBD_58, "Error on re-open of MAC logfile"));
		return;
	}
	fprintf(dbgfile,MSGSTR(MACILBD_59, "MAC daemon: opened new debug file\n"));
#endif
}

/*
	bad_syscall()-trap a bad system call in case setluid() not on system
*/

void
bad_syscall()
{
#ifdef ERROR_LOG
	error(MSGSTR(MACILBD_60, "MAC daemon: bad system call\n"));
#endif
	exit(1);
}

#ifdef DEBUG
/*
	dump_ir()-dump an ir to the debug file
*/

dump_ir(ir)
	register struct mac_ir	*ir;
{
	register int i = ir->ir_length - sizeof ir->class;
	register mask_t *cp = (mask_t *) ((ulong) ir + MAC_IR_SIZE);

	fprintf(dbgfile, "%d ", ir->class);
	for ( ; i > 0; ++cp, i -= sizeof(mask_t))
		fprintf(dbgfile, "%08x", *cp);
	fprintf(dbgfile, "\n");
}
#endif

#endif /*} SEC_MAC */
