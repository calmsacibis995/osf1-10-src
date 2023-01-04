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
static char	*sccsid = "@(#)$RCSfile: XAudUtils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:13 $";
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

#include <sys/secdefines.h>
#if SEC_BASE
#if SEC_MAC && ! SEC_SHW

/*
	filename:
		XAudUtils.c
        
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

 * Collection of utilities to help Audit sub-menu.
        
*/

/* Common C include files */
#include <sys/types.h>
#include <sys/security.h>
#include <sys/secioctl.h>

#include <stdio.h>
#include <mandatory.h>

#include "XMain.h"
#include "XAccounts.h"
#include "XAudit.h"

extern FILE *
	open_aud_parms();

extern void
	WorkingClose(),
	ErrorMessageOpen(),
	LoadMessage();

/* local routines */

/* Local variables */
static void
	clean_up();

/* Error messages */
static char 
	**msg_audit_error_this_future,
	**msg_cant_update_parms,
	**msg_cant_open_aud_parms,
	*msg_audit_error_this_future_text,
	*msg_cant_update_parms_text,
	*msg_cant_open_aud_parms_text;
	
int
XGetAuditSL(aufill)
AUDIT_SENSITIVITY_STRUCT *aufill ;
{
	FILE	*file_ptr ;
	int	tag_status;

	file_ptr = open_aud_parms(&aufill->au) ;
	if (file_ptr == (FILE *) 0) {
		WorkingClose();
		no_form_present = True;
		if (! msg_cant_open_aud_parms)
			LoadMessage ("msg_audit_error_cant_open_aud_parms",
				&msg_cant_open_aud_parms,
				&msg_cant_open_aud_parms_text);
		ErrorMessageOpen (-1, msg_cant_open_aud_parms, 0, NULL);
		return(FAILURE) ;
	}
	close(file_ptr) ;

	/* Store the min and max pointers */
	aufill->min_ir_ptr = (mand_ir_t *) 0;
	if (aufill->au.slevel_min != (tag_t) 0) {
		aufill->min_ir_ptr = mand_alloc_ir();
		if (aufill->min_ir_ptr == (mand_ir_t *) 0)
			MemoryError();
		tag_status = mand_tag_to_ir (aufill->au.slevel_min,
				aufill->min_ir_ptr);
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
		mand_copy_ir (mand_minsl, aufill->min_ir_ptr);
#else
		mand_copy_ir (mand_syslo, aufill->min_ir_ptr);
#endif
	}

	aufill->max_ir_ptr = (mand_ir_t *) 0;
	if (aufill->au.slevel_max != (tag_t) 0) {
		aufill->max_ir_ptr = mand_alloc_ir();
		if (aufill->max_ir_ptr == (mand_ir_t *) 0)
			MemoryError();
		tag_status = mand_tag_to_ir (aufill->au.slevel_max,
				aufill->max_ir_ptr);
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
		mand_copy_ir (mand_syshi, aufill->max_ir_ptr);
	}

	return(SUCCESS) ;
}

/* This is based on the static routine audit_sl_update_file from the file
 * aud_sl.c in ./userif
 */
int
XWriteAuditSL(aufill)

AUDIT_SENSITIVITY_STRUCT  	*aufill ;

{
	struct	audit_slevel	ioctl_data ;
	int			i ;
	int			character ;
	int			file_handle ;
	int			index ;
	int			tag_status ;
	int			transfer_count ;
	FILE			*old_file_ptr, *new_file_ptr ;
        priv_t effprivs[SEC_SPRIVVEC_SIZE];
        priv_t new_privs[SEC_SPRIVVEC_SIZE];

	if ((aufill-> this_session == 0) &&
	    (aufill-> future_sessions == 0)) {
		if (! msg_audit_error_this_future)
			LoadMessage ("msg_audit_error_this_or_future",
				&msg_audit_error_this_future,
				&msg_audit_error_this_future_text);
		ErrorMessageOpen (-1, msg_cant_open_aud_parms, 0, NULL);
		goto error;
	}

	tag_status = mand_ir_to_tag(aufill-> min_ir_ptr, 
		&ioctl_data.slevel_min) ;
	if (tag_status == 0) {
#ifdef OLD_CODE
		pop_msg("Unable to convert IR to tag", "for given minimum SL") ;
#endif
		sa_audit_audit(ES_AUD_MODIFY, "unable to change min SL\n") ;
		goto error;
	}

	tag_status = mand_ir_to_tag(aufill-> max_ir_ptr,
				    &ioctl_data.slevel_max) ;

	if (tag_status == 0) {
#ifdef OLD_CODE
		pop_msg("Unable to convert IR to tag", "for given maximum SL") ;
#endif
		sa_audit_audit(ES_AUD_MODIFY, "unable to change max SL\n") ;
		goto error;
	}

	if (aufill-> this_session) {
		file_handle = open(AUDIT_WDEVICE, O_WRONLY) ;
		transfer_count = 0 ;
		if (file_handle >= 0) {
			ioctl(file_handle, AUDIOC_SLEVEL, &ioctl_data) ;
			close(file_handle) ;
		}
		if ((file_handle < 0) || (transfer_count < 0)) {
#ifdef OLD_CODE
			pop_msg("Unable to change current session SL auditing",
				"Audit may not be enabled") ;
#endif
			sa_audit_audit(ES_AUD_MODIFY,
				"unsuccessful change of current session SL\n") ;
			goto error;
		}
		else {
			sa_audit_audit(ES_AUD_MODIFY,
				  "successful change of current session SL\n") ;
		}
	}

	if (aufill-> future_sessions) {
		aufill-> au.slevel_min = ioctl_data.slevel_min ;
		aufill-> au.slevel_max = ioctl_data.slevel_max ;

		new_file_ptr = new_audit_parms_file() ;
		if (new_file_ptr == (FILE *) 0)
			goto error;

		old_file_ptr = old_audit_parms_file() ;
		if (old_file_ptr == (FILE *) 0) {
			fclose(new_file_ptr) ;
			unlink_new_parms_file() ;
			goto error;
		}

		if (fwrite(&aufill-> au, sizeof(aufill-> au),
			   1, new_file_ptr) != 1) {
			clean_up(old_file_ptr, new_file_ptr,
			      "Write of audit_parms header failed",
			      "Please report problem and re-run program") ;	
			goto error;
		}

		fseek(old_file_ptr, aufill-> au.uid_offset, 0) ;
		for (index = 0 ; index < aufill-> au.uid_count *
				 sizeof(uid_t) ; index++)
			if ((character = getc(old_file_ptr)) == EOF ||
				    putc(character, new_file_ptr) == EOF) {
				clean_up(old_file_ptr, new_file_ptr,
			      	  "Write of UIDs failed",
			        "Check file permissions and file system space");
				goto error;
			}

		fseek(old_file_ptr, aufill-> au.gid_offset, 0) ;
		for (index = 0 ; index < aufill-> au.gid_count *
				 sizeof(gid_t) ; index++)
			if ((character = getc(old_file_ptr)) == EOF ||
				    putc(character, new_file_ptr) == EOF) {
				clean_up(old_file_ptr, new_file_ptr,
			      	  "Write of GIDs failed",
			       "Check file permissions and file system space") ;

				goto error;
			}

		fseek(old_file_ptr, aufill-> au.dir_offset, 0) ;
		for (index = 0 ; index < aufill-> au.dir_count ;
				 index++)
			do {
				if ((character = getc(old_file_ptr)) == EOF ||
				    	putc(character, new_file_ptr) == EOF) {
					clean_up(old_file_ptr, new_file_ptr,
			      	  	   "Write of directorys failed",
			        "Check file permissions and file system space");
					goto error;
				}
			} while (character != '\0') ;

		fclose(new_file_ptr) ;
		fclose(old_file_ptr) ;
		link_new_parms_file() ;
		sa_audit_audit(ES_AUD_MODIFY, "Successful update of audit parameter file") ;
	}
	return(SUCCESS) ;
error:
	if (! msg_cant_update_parms)
		LoadMessage ("msg_audit_error_cant_update_parms",
			&msg_cant_update_parms,
			&msg_cant_update_parms_text);
	ErrorMessageOpen (-1, msg_cant_update_parms, 0, NULL);
	return(FAILURE) ;

}

static void
clean_up(old_file_ptr, new_file_ptr, message_one, message_two)

FILE	*old_file_ptr, *new_file_ptr ;
char	*message_one, *message_two ;

{
	fclose(new_file_ptr) ;
	fclose(old_file_ptr) ;
	unlink_new_parms_file() ;
	/* On *if programs we display the error message. We leave this
	 * code for a second release
	 */
#ifdef OLD_CODE
	pop_msg(message_one, message_two) ;
#endif
	sa_audit_audit(ES_AUD_ERROR, message_one) ;
	return ;
}

#endif /* SEC_MAC */
#endif /* SEC_BASE */
