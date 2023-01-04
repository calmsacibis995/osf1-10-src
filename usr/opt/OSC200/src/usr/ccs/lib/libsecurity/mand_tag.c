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
static char	*sccsid = "@(#)$RCSfile: mand_tag.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:17:22 $";
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
/*
 * Copyright (c) 1989 SecureWare, Inc.  All Rights Reserved.
 */




#include <sys/secdefines.h>
#include "libsecurity.h"

#if SEC_MAC && !defined(SEC_STANDALONE) /*{*/

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _OSF_SOURCE
#include <dirent.h>
#endif

#include <sys/security.h>
#include <sys/secpolicy.h>
#include <sys/audit.h>
#include <mandatory.h>
#include <prot.h>

#define	DAEMON		0
#define	CLIENT		1

#ifdef TAGDEBUG
static void compare();
#endif

extern char *calloc();

#ifdef TAGDEBUG
main(argc, argv)
	int argc;
	char *argv[];
{
	char er[BUFSIZ];
	char *erp;
	char num[BUFSIZ];
	mand_ir_t *sec_level;
	tag_t tag;

	set_auth_parameters(argc, argv);

	switch (argc)  {
		case 2:
		    if (strcmp(argv[1], "er") == 0)
			while (gets(er) != (char *) 0)  {
			  sec_level = mand_er_to_ir(er);
			  if (sec_level == (mand_ir_t *) 0)
				printf(MSGSTR(MAND_TAG_1, "%s is an unknown security level\n"),
					er);
			  else if (mand_ir_to_tag(sec_level, &tag))
				printf(MSGSTR(MAND_TAG_2, "tag for %s is %ld\n"), er, tag);
			  else
				printf(MSGSTR(MAND_TAG_3, "cannot find tag for %s\n"), er);
			}
		    else
			while (gets(num) != (char *) 0)  {
			  tag = atol(num);
			  sec_level = mand_alloc_ir();
			  if (mand_tag_to_ir(tag, sec_level))  {
				if (erp = mand_ir_to_er(sec_level))
					printf(MSGSTR(MAND_TAG_4, "ER for %ld is %s\n"),
						tag, erp);
				else
					printf(MSGSTR(MAND_TAG_5, "cannot convert IR for %ld to ER\n"),
						tag);
			  }
			  else
				printf(MSGSTR(MAND_TAG_6, "cannot find IR for %ld\n"), tag);
			  mand_free_ir(sec_level);
			}
		    break;

		case 3:
		    if (strcmp(argv[1], "er") == 0)  {
			sec_level = mand_er_to_ir(argv[2]);
			if (sec_level == (mand_ir_t *) 0)
				printf(MSGSTR(MAND_TAG_1, "%s is an unknown security level\n"),
					argv[2]);
			else if (mand_ir_to_tag(sec_level, &tag))
				printf(MSGSTR(MAND_TAG_2, "tag for %s is %ld\n"), argv[2], tag);
			else
				printf(MSGSTR(MAND_TAG_3, "cannot find tag for %s\n"), argv[2]);
		    }
		    else  {
			tag = atol(argv[2]);
			sec_level = mand_alloc_ir();
			if (mand_tag_to_ir(tag, sec_level))  {
				if (erp = mand_ir_to_er(sec_level))
					printf(MSGSTR(MAND_TAG_4, "ER for %ld is %s\n"),
						tag, erp);
				else
					printf(MSGSTR(MAND_TAG_5, "cannot convert IR for %ld to ER\n"),
						tag);
			}
			else
				printf(MSGSTR(MAND_TAG_6, "cannot find IR for %ld\n"), tag);
			mand_free_ir(sec_level);
		    }
		    break;

		case 4:
		    compare(argv[2], argv[3]);
		    break;

		default:
		    printf(MSGSTR(MAND_TAG_7, "Usage: %s tag|ir [seclevel]\n"), command_name);
		    break;
	}
}


static void
compare(sub_er, obj_er)
	char *sub_er;
	char *obj_er;
{
	register int decision;
	mand_ir_t *subject_ir;
	mand_ir_t *object_ir;

	subject_ir = mand_er_to_ir(sub_er);
	if (subject_ir == (mand_ir_t *) 0)  {
		printf(MSGSTR(MAND_TAG_1, "%s is an unknown security level\n"),
			sub_er);
		exit(1);
	}

	object_ir = mand_er_to_ir(obj_er);
	if (object_ir == (mand_ir_t *) 0)  {
		printf(MSGSTR(MAND_TAG_1, "%s is an unknown security level\n"),
			obj_er);
		exit(1);
	}

	decision = mand_ir_relationship(subject_ir, object_ir);

	if ((decision & MAND_SDOM) != 0)
		printf(MSGSTR(MAND_TAG_8, "subject dominates object\n"));
	if ((decision & MAND_ODOM) != 0)
		printf(MSGSTR(MAND_TAG_9, "object dominates subject\n"));
	if ((decision & MAND_EQUAL) != 0)
		printf(MSGSTR(MAND_TAG_10, "subject and object are equal\n"));
	if ((decision & MAND_INCOMP) != 0)
	      printf(MSGSTR(MAND_TAG_11, "subject and object are incomparable\n"));

	mand_free_ir(subject_ir);
	mand_free_ir(object_ir);
}
#endif


/*
 * This routine is given an IR for a security level and fills in the
 * tag for that IR.  It also returns 1 to note that the operation
 * succeeded or 0 to signify that the operation failed and the tag
 * is left unchanged.
 */
#if SEC_ILB
int
mand_ir_to_tag(ir, tag)
	mand_ir_t *ir;
	tag_t *tag;
{
	return macilb_ir_to_tag((ilb_ir_t *) ir, 0, tag);
}


int
macilb_ir_to_tag(ir, isilb, tag)
	ilb_ir_t *ir;
	int isilb;
	tag_t *tag;
#else
int
mand_ir_to_tag(ir, tag)
	mand_ir_t *ir;
	tag_t *tag;
#endif
{
	register int spdfd;
	register int ret;
	register int msg_size;
	register struct spd_map_tag *query;
	int got_tag = 0;
	struct spd_set_tag response;
	char spd_dev[sizeof(SP_DAEMON_DEVICE) + NAME_MAX + 2];

	if (mand_init() != 0)
		return got_tag;

#if SEC_ILB
	msg_size = sizeof(*query) + (isilb ? ilb_bytes() : mand_bytes());
#else
	msg_size = sizeof(*query) + mand_bytes();
#endif
	query = (struct spd_map_tag *) calloc(msg_size, 1);
	if (query != (struct spd_map_tag *) 0)  {
	    sprintf(spd_dev, "%s%d", SP_DAEMON_DEVICE,
		    mand_config.minor_device | CLIENT);
	    spdfd = open(spd_dev, O_RDWR);

	    if (spdfd >= 0)  {
		query->mhdr.msg_type = SPD_MAP_TAG;
#if SEC_ILB
		query->ir.ir_length = isilb ? ilb_bytes() : mand_bytes();
#else
		query->ir.ir_length = mand_bytes();
#endif
		memcpy(query + 1, ir, query->ir.ir_length);
		ret = write(spdfd, query, msg_size);

		if (ret == msg_size)  {
			ret = read(spdfd, &response, sizeof response);

			if (ret == sizeof response &&
			    response.mhdr.error_code == SPD_OK)  {
				*tag = response.tag;
				got_tag = 1;
			}
#ifdef DEBUG
			else if (response.mhdr.error_code != SPD_OK)
			    fprintf(stderr, MSGSTR(MAND_TAG_12, "SPD_MAP_TAG: error %d\n"),
				    response.mhdr.error_code);
#endif
		}
		(void) close(spdfd);
	    }
	    free(query);
	}

	return got_tag;
}

/*
 * This routine is given a tag for a security level and fills in the
 * IR supplied by the invoker.  It also returns 1 to note that the operation
 * succeeded or 0 to signify that the operation failed and the IR
 * is left unchanged.
 */
#if SEC_ILB
int
mand_tag_to_ir(tag, ir)
	tag_t tag;
	mand_ir_t *ir;
{
	return macilb_tag_to_ir(tag, ir, 0);
}


int
macilb_tag_to_ir(tag, ir, isilb)
	tag_t tag;
	ilb_ir_t *ir;
	int isilb;
#else
int
mand_tag_to_ir(tag, ir)
	tag_t tag;
	mand_ir_t *ir;
#endif
{
	register int spdfd;
	register int ret;
	register int msg_size;
	register struct spd_internal_rep *response;
	int got_ir = 0;
	struct spd_get_attribute query;
	char spd_dev[sizeof(SP_DAEMON_DEVICE) + NAME_MAX + 2];

	if (mand_init() != 0)
		return got_ir;

#if SEC_ILB
	msg_size = sizeof(*response) + ilb_bytes();
#else
	msg_size = sizeof(*response) + mand_bytes();
#endif
	response = (struct spd_internal_rep *) calloc(msg_size, 1);

	if (response != (struct spd_internal_rep *) 0)  {
	    sprintf(spd_dev, "%s%d", SP_DAEMON_DEVICE,
		    mand_config.minor_device | CLIENT);
	    spdfd = open(spd_dev, O_RDWR);

	    if (spdfd >= 0)  {
		query.mhdr.msg_type = SPD_GET_ATTRIBUTE;
		query.tag = tag;
		ret = write(spdfd, &query, sizeof(query));

		if (ret == sizeof(query))  {
			ret = read(spdfd, response, msg_size);
#if SEC_ILB
			if (!isilb) {
				if (ret == sizeof(*response) + mand_bytes())
					got_ir = 1;
				else if (ret == msg_size)
					got_ir = -1;
				if (got_ir)
					memcpy(ir, response + 1, mand_bytes());
			} else {
				if (ret == sizeof(*response) + mand_bytes()) {
					got_ir = -1;
					memcpy(ir, response + 1, mand_bytes());
				} else if (ret == msg_size) {
					got_ir = 1;
					memcpy(ir, response + 1, ilb_bytes());
				}
			}
#else
			if (ret == msg_size)  {
				memcpy(ir, response + 1, mand_bytes());
				got_ir = 1;
			}
#endif
		}
		(void) close(spdfd);
	    }
	    free(response);
	}

	return got_ir;
}


/*
 * This routine returns a decision word after comparing the subject
 * and object IRs.  If the word returned is 0, an error occurred.
 * Otherwise, exactly one bit is set to denote the relationship.
 */
#if SEC_ILB
int
macilb_ir_relationship(subject_ir, subj_is_ilb, object_ir, obj_is_ilb)
	ilb_ir_t *subject_ir, *object_ir;
	int subj_is_ilb, obj_is_ilb;
{
	int decision = 0;
	tag_t subject_tag;
	tag_t object_tag;

	if (macilb_ir_to_tag(subject_ir, subj_is_ilb, &subject_tag) &&
	    macilb_ir_to_tag(object_ir, obj_is_ilb, &object_tag))
		decision = mand_tag_relationship(subject_tag, object_tag);

	return decision;
}


mand_ir_relationship(subject_ir, object_ir)
	mand_ir_t *subject_ir;
	mand_ir_t *object_ir;
{
	return macilb_ir_relationship(subject_ir, 0, object_ir, 0);
}

#else

int
mand_ir_relationship(subject_ir, object_ir)
	mand_ir_t *subject_ir;
	mand_ir_t *object_ir;
{
	int decision = 0;
	tag_t subject_tag;
	tag_t object_tag;

	if (mand_ir_to_tag(subject_ir, &subject_tag) &&
	    mand_ir_to_tag(object_ir, &object_tag))
		decision = mand_tag_relationship(subject_tag, object_tag);

	return decision;
}
#endif


/*
 * This routine returns a decision word after comparing the subject
 * and object tags.  If the word returned is 0, an error occurred.
 * Otherwise, exactly one bit is set to denote the relationship.
 */
int
mand_tag_relationship(subject, object)
	tag_t subject;
	tag_t object;
{
	register int spdfd;
	register int ret;
	register int msg_size;
	register struct spd_decision *response;
	register mac_dec_t *mac_dec;
	register int decision = 0;
	struct spd_make_decision query;
	char spd_dev[sizeof(SP_DAEMON_DEVICE) + NAME_MAX + 2];

	if (mand_init() != 0)
		return decision;

	msg_size = sizeof(*response) + MAC_DECISION_SIZE;
	response = (struct spd_decision *) calloc(msg_size, 1);

	if (response != (struct spd_decision *) 0)  {
	    sprintf(spd_dev, "%s%d", SP_DAEMON_DEVICE,
		    mand_config.minor_device | CLIENT);
	    spdfd = open(spd_dev, O_RDWR);

	    if (spdfd >= 0)  {
		msg_size = sizeof(query);
		query.mhdr.msg_type = SPD_MAKE_DECISION;
		query.subject = subject;
		query.object = object;
		ret = write(spdfd, &query, msg_size);
		if (ret == msg_size)  {
			msg_size = sizeof(*response) + MAC_DECISION_SIZE;
			response->mhdr.msg_type = SPD_DECISION;
			mac_dec = (mac_dec_t *) &response->decision;
			mac_dec->dec_length = MAC_DECISION_SIZE;
			ret = read(spdfd, response, msg_size);
			if (ret == msg_size)  {
				if (mac_dec->dec.subj_dom)
					decision |= MAND_SDOM;
				if (mac_dec->dec.obj_dom)
					decision |= MAND_ODOM;
				if (mac_dec->dec.same)
					decision |= MAND_EQUAL;
				if (mac_dec->dec.incomp)
					decision |= MAND_INCOMP;
#if SEC_ILB
				if (mac_dec->dec.ilb_subj_dom)
					decision |= ILB_SDOM;
				if (mac_dec->dec.ilb_obj_dom)
					decision |= ILB_ODOM;
				if (mac_dec->dec.ilb_same)
					decision |= ILB_SAME;
#endif
			}
		}
		(void) close(spdfd);
	    }
	    free(response);
	}

	return decision;
}
#endif /*} SEC_MAC && !defined(SEC_STANDALONE) */
