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
static char	*sccsid = "@(#)$RCSfile: AudColSL.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:59:03 $";
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
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved.
 */



/* Routines used to change audit collection sensitivity levels. */

#include <sys/secdefines.h>

#if SEC_MAC

#include "gl_defs.h"
#include "IfAudit.h"
#include <sys/secioctl.h>

/* Error messages */
static char 
	**msg_audit_error_this_future,
	**msg_cant_update_parms,
	**msg_cant_open_aud_parms,
	*msg_audit_error_this_future_text,
	*msg_cant_update_parms_text,
	*msg_cant_open_aud_parms_text;
	

/*
 * Return the minimum and maximum sensitivity labels into the aufill
 * structure.  Returns 0 on success, 1 on failure.
 */

int
GetAuditSL(aufill)
AudSL_fillin *aufill;
{
	FILE	*file_ptr;
	int	tag_status;
	privvec_t s;

	file_ptr = open_aud_parms(&aufill->au);
	if (file_ptr == (FILE *) 0) {
		if (! msg_cant_open_aud_parms)
			LoadMessage ("msg_audit_error_cant_open_aud_parms",
				&msg_cant_open_aud_parms,
				&msg_cant_open_aud_parms_text);
		ErrorMessageOpen (-1, msg_cant_open_aud_parms, 0, NULL);
		return(1);
	}
	fclose(file_ptr);

	/* Store the min and max pointers */
	aufill->min_ir_ptr = (mand_ir_t *) 0;
	if (aufill->au.slevel_min != (tag_t) 0) {
		aufill->min_ir_ptr = mand_alloc_ir();
		if (aufill->min_ir_ptr == (mand_ir_t *) 0)
			MemoryError();
		forceprivs(privvec(SEC_ALLOWMACACCESS, SEC_ALLOWDACACCESS,
#if SEC_ILB
				   SEC_ILNOFLOAT,
#endif
					-1), s);
		tag_status = mand_tag_to_ir(aufill->au.slevel_min,
				aufill->min_ir_ptr);
		seteffprivs(s, NULL);
		if (tag_status == 0) {
			mand_free_ir(aufill->min_ir_ptr);
			aufill->min_ir_ptr = (mand_ir_t *) 0;
		}
	}

	/* If none allocated use minsl */
	if (aufill->min_ir_ptr == (mand_ir_t *) 0) {
		aufill->min_ir_ptr = mand_alloc_ir();
		if (aufill->min_ir_ptr == (mand_ir_t *) 0)
			MemoryError();
#if SEC_ENCODINGS
		mand_copy_ir(mand_minsl, aufill->min_ir_ptr);
#else
		mand_copy_ir(mand_syslo, aufill->min_ir_ptr);
#endif
	}

	aufill->max_ir_ptr = (mand_ir_t *) 0;
	if (aufill->au.slevel_max != (tag_t) 0) {
		aufill->max_ir_ptr = mand_alloc_ir();
		if (aufill->max_ir_ptr == (mand_ir_t *) 0)
			MemoryError();
		forceprivs(privvec(SEC_ALLOWMACACCESS, SEC_ALLOWDACACCESS,
#ifdef SEC_ILB
				   SEC_ILNOFLOAT,
#endif
					-1), s);
		tag_status = mand_tag_to_ir (aufill->au.slevel_max,
				aufill->max_ir_ptr);
		seteffprivs(s, NULL);
		if (tag_status == 0) {
			mand_free_ir(aufill->max_ir_ptr);
			aufill->max_ir_ptr = (mand_ir_t *) 0;
		}
	}

	/* If none allocated use syshi */
	if (aufill->max_ir_ptr == (mand_ir_t *) 0) {
		aufill->max_ir_ptr = mand_alloc_ir();
		if (aufill->max_ir_ptr == (mand_ir_t *) 0)
			MemoryError();
		mand_copy_ir(mand_syshi, aufill->max_ir_ptr);
	}

	return(0);
}

/*
 * Change the sensitivity labels in the audit parameters file.
 * Assumes aufill->au was filled with valid data from before.
 */

int
WriteAuditSL(aufill)
	AudSL_fillin *aufill;
{
	struct	audit_slevel	ioctl_data;
	int			character;
	int			file_handle;
	int			index;
	int			tag_status;
	int			ret;
	FILE			*old_file_ptr, *new_file_ptr;
	privvec_t s;

	if ((aufill-> this_session == 0) &&
	    (aufill-> future_sessions == 0)) {
		if (! msg_audit_error_this_future)
			LoadMessage ("msg_audit_error_this_or_future",
				&msg_audit_error_this_future,
				&msg_audit_error_this_future_text);
		ErrorMessageOpen (-1, msg_cant_open_aud_parms, 0, NULL);
		goto error;
	}

	/* raise privileges and convert min SL to tag */

	forceprivs(privvec(SEC_ALLOWMACACCESS,
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
				-1), s);
	tag_status = mand_ir_to_tag(aufill->min_ir_ptr,
		&ioctl_data.slevel_min);
	seteffprivs(s, NULL);

	if (tag_status == 0) {
		sa_audit_audit(ES_AUD_MODIFY, "unable to change min SL");
		goto error;
	}

	tag_status = mand_ir_to_tag(aufill-> max_ir_ptr,
				    &ioctl_data.slevel_max);

	if (tag_status == 0) {
		sa_audit_audit(ES_AUD_MODIFY, "unable to change max SL\n");
		goto error;
	}

	if (aufill-> this_session) {
		forceprivs(privvec(SEC_ALLOWMACACCESS, SEC_ALLOWDACACCESS,
#if SEC_ILB
				   SEC_ILNOFLOAT,
#endif
				   -1), s);
		file_handle = open(AUDIT_WDEVICE, O_WRONLY);
		seteffprivs(s);
		if (file_handle >= 0) {
			ret = ioctl(file_handle, AUDIOC_SLEVEL, &ioctl_data);
			close(file_handle);
		}
		if (file_handle < 0 || ret < 0) {
			sa_audit_audit(ES_AUD_MODIFY,
				"unsuccessful change of current session SL");
			goto error;
		}
		else {
			sa_audit_audit(ES_AUD_MODIFY,
				  "successful change of current session SL");
		}
	}

	/* only change if future is requested and either min or max changed */

	if (aufill->future_sessions &&
	     (aufill->au.slevel_min != ioctl_data.slevel_min ||
	      aufill->au.slevel_max != ioctl_data.slevel_max)) {
		aufill->au.slevel_min = ioctl_data.slevel_min;
		aufill->au.slevel_max = ioctl_data.slevel_max;

		if (update_audit_parms(&aufill->au))
			goto error;
	}

	return(0);
error:
	if (! msg_cant_update_parms)
		LoadMessage("msg_audit_error_cant_update_parms",
		  &msg_cant_update_parms,
		  &msg_cant_update_parms_text);
	ErrorMessageOpen (-1, msg_cant_update_parms, 0, NULL);
	return(1);

}

#endif /* SEC_MAC */
