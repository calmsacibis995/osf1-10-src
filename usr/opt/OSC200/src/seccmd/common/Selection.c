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
static char	*sccsid = "@(#)$RCSfile: Selection.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:00:47 $";
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
 *   All rights reserved
 */



#include <sys/secdefines.h>

#if SEC_BASE

/*
	entry points:
		SelectionCreateValidate
		SelectionUpdateValidate
		SelectionFreeTables 
		SelectionCreateFill 
		SelectionDisplayFill
		SelectionWriteFile 
		SelectionDeleteFile
		SelectionFileGet
		SelectionFileStart

	purpose:
		routines that manage selection parameter files

*/

#include "gl_defs.h"
#include "IfAudit.h"
#include <time.h>

#define MAX_CLASS_WIDTH 20
#define MAX_CAT_WIDTH   15
#define NONE        "*None*"

static char     *months[] = {
			"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
			"Aug", "Sep", "Oct", "Nov", "Dec" 
		};
#define NMONTHS (sizeof(months) / sizeof(*months))

static char 	**msg_aselection,
		*msg_aselection_text,
		*twozeros = "00";
char		**msg_selection,
		*msg_selection_text;

static int      mdays[] = {
		31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
		};
static FILE*    SelectionOpenFile();
static void     
#if SEC_MAC
		BuildMaxSL(),
		BuildMinSL(),
#endif /* SEC_MAC */
		ParseDate(),
		SelectionFillTime();
static char   	**GetEventTable(),
		**GetFilesTable(),
		**GetGroupTable(),
		**GetUserTable();
static int      GetTwoDigits(),
		SelectionEditFiles(),
		SelectionEditTime();
static time_t   SelectionGetTime();
				
void
ASelectionOpen ()
{
	LoadMessage("msg_isso_audit_aselection", 
	&msg_aselection, &msg_aselection_text);
}

int
SelectionCreateValidate(file, rpfill)
	char        *file;
	AUDIT_SELECTION_STRUCT  *rpfill;
{
	char    filename[sizeof(AUDIT_REDUCE_PARM_DIR) + ENTSIZ];
	struct stat sb;
	privvec_t s;
	int ret;

	/* check existence of file */
	sprintf (filename, "%s%s", AUDIT_REDUCE_PARM_DIR, file);
	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);
	ret = stat(filename, &sb);
	seteffprivs(s, NULL);

	if (ret == 0) {
		ErrorMessageOpen(3410, msg_aselection, 0, NULL);
		return (1);
	}

	strcpy (rpfill->filename, file);
	return (0);
}

int
SelectionUpdateValidate(file, rpfill)
	char        *file;
	AUDIT_SELECTION_STRUCT  *rpfill;
{
	char    filename[sizeof(AUDIT_REDUCE_PARM_DIR) + ENTSIZ];
	struct  stat    sb;
	int ret;
	privvec_t s;

	sprintf (filename, "%s%s", AUDIT_REDUCE_PARM_DIR, file);
	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);
	ret = stat(filename, &sb);
	seteffprivs(s, NULL);

	/* check file exists */
	if (ret < 0 ) {
		ErrorMessageOpen(3420, msg_aselection, 15, NULL);
		/* "Reduction parameter file does not exist." */
		/* Please specify an existing reduction parameter file" */
		return (1);
	}

	strcpy (rpfill->filename, file);
	return (0);
}

void
SelectionFreeTables (rpfill)
	AUDIT_SELECTION_STRUCT  *rpfill;
{

	if (rpfill->events != (char **) 0) {
		free_cw_table (rpfill->events);
		rpfill->events = (char **) 0;
	}
	if (rpfill->users != (char **) 0) {
		free_cw_table (rpfill->users);
		rpfill->users = (char **) 0;
	}
	if (rpfill->groups != (char **) 0) {
		free_cw_table (rpfill->groups);
		rpfill->groups = (char **) 0;
	}
	if (rpfill->files != (char **) 0) {
		free_cw_table (rpfill->files);
		rpfill->files = (char **) 0;
	}
#if SEC_MAC
	/* Freeing this memory causes a problem because we need this space
	 * to store the outcome. This is a potential memory leak 
	 * one byte at a time ... XXXXX Need to look at this later ...
	if (rpfill->slevel_min != (mand_ir_t *) 0) {
		mand_free_ir(rpfill->slevel_min);
		rpfill->slevel_min = (mand_ir_t *) 0;
	}
	if (rpfill->slevel_max != (mand_ir_t *) 0) {
		mand_free_ir(rpfill->slevel_max);
		rpfill->slevel_max = (mand_ir_t *) 0;
	}
	if (rpfill->olevel_min != (mand_ir_t *) 0) {
		mand_free_ir(rpfill->olevel_min);
		rpfill->olevel_min = (mand_ir_t *) 0;
	}
	if (rpfill->olevel_max != (mand_ir_t *) 0) {
		mand_free_ir(rpfill->olevel_max);
		rpfill->olevel_max = (mand_ir_t *) 0;
	}
	*/
#endif /* SEC_MAC */
}

/* make sure that file doesn't already exist. */

int
SelectionCreateFill (rpfill)
	register  AUDIT_SELECTION_STRUCT  *rpfill;
{
	int     i;
	void    SelectionFreeTables();
	int	ret;

	rpfill->as.sel_size = sizeof (rpfill->as);
	rpfill->as.uid_count = 0;
	rpfill->as.gid_count = 0;
	rpfill->as.object_count = 0;
	for (i = 0; i < AUDIT_MASK_SIZE; i++)
		rpfill->as.event_mask[i] = 0;
	rpfill->as.uid_offset = 0;
	rpfill->as.gid_offset = 0;
	rpfill->as.object_offset = 0;
	rpfill->as.start = 0;
	rpfill->as.stop = 0;
	rpfill->users = rpfill->groups = rpfill->files = (char **) 0;
#if SEC_MAC 
	rpfill->as.slevel_min = (tag_t) 0;
	rpfill->as.slevel_max = (tag_t) 0;
	rpfill->as.olevel_min = (tag_t) 0;
	rpfill->as.olevel_max = (tag_t) 0;
#endif /* SEC_MAC */
	
	rpfill->events = GetEventTable(rpfill);
	if (rpfill->events == (char **) 0)
		goto bad;
	rpfill->users = (char **) 0;
	rpfill->nusers = 0;
	rpfill->groups = (char **) 0;
	rpfill->ngroups = 0;
	rpfill->as.object_count = 0;
	rpfill->files = GetFilesTable(rpfill, 1, (FILE *) 0);
	if (rpfill->files == (char **) 0)
		goto bad;
	rpfill->nfiles = 1;
	(void) strcpy (rpfill->s_hour, twozeros);
	(void) strcpy (rpfill->s_min, twozeros);
	strncpy (rpfill->s_month, "", sizeof (rpfill->s_month));
	(void) strcpy (rpfill->s_day, twozeros);
	(void) strcpy (rpfill->s_year, twozeros);
	(void) strcpy (rpfill->e_hour, twozeros);
	(void) strcpy (rpfill->e_min, twozeros);
	strncpy (rpfill->e_month, "", sizeof (rpfill->e_month));
	(void) strcpy (rpfill->e_day, twozeros);
	(void) strcpy (rpfill->e_year, twozeros);

#if SEC_MAC 
	BuildMinSL(rpfill);
	BuildMaxSL(rpfill);
#endif /* SEC_MAC */

	return (0);
bad:
	SelectionFreeTables(rpfill);
	return (1);
}

int
SelectionDisplayFill (rpfill)
AUDIT_SELECTION_STRUCT  *rpfill;
{
	FILE    *fp;
	void    SelectionFreeTables();

	fp = SelectionOpenFile(rpfill->filename, &rpfill->as);
	if (fp == (FILE *) 0)
		return (1);
	rpfill->users = (char **) 0;
	rpfill->groups = (char **) 0;
	rpfill->files = (char **) 0;

#if SEC_MAC
	rpfill->slevel_min = (mand_ir_t *) 0;
	rpfill->slevel_max = (mand_ir_t *) 0;
	rpfill->olevel_min = (mand_ir_t *) 0;
	rpfill->olevel_max = (mand_ir_t *) 0;
	BuildMinSL(rpfill);
	BuildMaxSL(rpfill);
#endif /* SEC_MAC */

	rpfill->events = GetEventTable (rpfill);
	if (rpfill->events == (char **) 0)
		goto bad;
	rpfill->users = GetUserTable(rpfill,rpfill->as.uid_count, fp);
	if (rpfill->users == (char **) 0)
		goto bad;
	rpfill->nusers = rpfill->as.uid_count;
	rpfill->groups = GetGroupTable(rpfill, rpfill->as.gid_count, fp);
	if (rpfill->groups == (char **) 0)
		goto bad;
	rpfill->ngroups = rpfill->as.gid_count;
	rpfill->files = GetFilesTable (rpfill,
			rpfill->as.object_count, fp);
	if (rpfill->files == (char **) 0)
		goto bad;
	rpfill->nfiles = rpfill->as.object_count;
	fclose (fp);
	SelectionFillTime (rpfill);
	return 0;
bad:
	SelectionFreeTables(rpfill);
	fclose(fp);
	
}
int
SelectionWriteFile(rpfill)
	AUDIT_SELECTION_STRUCT  *rpfill;
{
	FILE    *fp;
	char    filename[sizeof(AUDIT_REDUCE_PARM_DIR) + ENTSIZ];
	int     bytecount,
		count, 
		i,
		tag_status;
	privvec_t s;
	struct stat sb;

	if (SelectionEditTime(rpfill->s_hour, rpfill->s_min, rpfill->s_day, 
			rpfill->s_month, rpfill->s_year))
		return(1);         
	if (SelectionEditTime(rpfill->e_hour, rpfill->e_min, rpfill->e_day, 
			rpfill->e_month, rpfill->e_year))
		return(1);
	if (SelectionEditFiles(rpfill))
		return(1);

	sprintf (filename, "%s%s", AUDIT_REDUCE_PARM_DIR, rpfill->filename);

	/* raise privileges necessary to create and open file for writing */

	forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_CHOWN,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);

	/* create file if it doesn't exist */

	if (stat(filename, &sb) < 0)
		create_file_securely(filename, AUTH_VERBOSE);

	/* open file for writing */

	fp = fopen(filename, "w");

	/* drop privileges because now have open file descriptor */

	seteffprivs(s, NULL);

	if (fp == (FILE *) 0)  {
		ErrorMessageOpen(3450, msg_aselection, 41, NULL);  
		/* AUDIT open new parameter file failure */
		audit_no_resource (filename, OT_SUBSYS,
		  msg_aselection[44], ET_SYS_ADMIN);
		return(1);
	}

	count = 0;
	for (i = 0; i < rpfill->nusers; i++)
		if (rpfill->users[i][0] != '\0')
			count++;
	rpfill->as.uid_count = count;
	rpfill->as.uid_offset = sizeof (rpfill->as);
	count = 0;
	for (i = 0; i < rpfill->ngroups; i++)
		if (rpfill->groups[i][0] != '\0')
			count++;
	rpfill->as.gid_count = count;
	rpfill->as.gid_offset =
	  rpfill->as.uid_offset + (rpfill->as.uid_count * sizeof (uid_t));
	count = bytecount = 0;
	for (i = 0; i < rpfill->nfiles; i++)
		if (rpfill->files[i][0] != '\0') {
			count++;
			bytecount += strlen (rpfill->files[i]) + 1;
		}
	for (i = 1; i <= AUDIT_MAX_EVENT; i++)
		if (rpfill->events[i-1][0] == YESCHAR)
			ADDBIT (rpfill->as.event_mask, i);
		else    RMBIT  (rpfill->as.event_mask, i);
	rpfill->as.object_count = count;
	rpfill->as.object_offset =
	  rpfill->as.gid_offset + (rpfill->as.gid_count * sizeof (gid_t));
	rpfill->as.sel_size = rpfill->as.object_offset + bytecount;
	rpfill->as.start = SelectionGetTime (rpfill->s_hour, rpfill->s_min,
	  rpfill->s_month, rpfill->s_day, rpfill->s_year);
	rpfill->as.stop = SelectionGetTime (rpfill->e_hour, rpfill->e_min,
	  rpfill->e_month, rpfill->e_day, rpfill->e_year);
	if (rpfill->as.start > rpfill->as.stop) {
		ErrorMessageOpen(3451, msg_aselection, 76, NULL);  
		/* "End Time must be later than Start Time." */
		return(1);
	}    
#if SEC_MAC
	forceprivs(privvec(SEC_ALLOWMACACCESS, -1), s);

	if ( rpfill->slevel_min != (mand_ir_t *) 0 ) {
		tag_status = mand_ir_to_tag(rpfill->slevel_min,
						&rpfill->as.slevel_min);
		if ( tag_status == 0 ) {
			rpfill->as.slevel_min = (tag_t) 0;
		}
	}

	if ( rpfill->slevel_max != (mand_ir_t *) 0 ) {
		tag_status = mand_ir_to_tag(rpfill->slevel_max,
						&rpfill->as.slevel_max);
		if ( tag_status == 0 ) {
			rpfill->as.slevel_max = (tag_t) 0;
		}
	}

	if ( rpfill->olevel_min != (mand_ir_t *) 0 ) {
		tag_status = mand_ir_to_tag(rpfill->olevel_min,
						&rpfill->as.olevel_min);
		if ( tag_status == 0 ) {
			rpfill->as.olevel_min = (tag_t) 0;
		}
	}

	if ( rpfill->olevel_max != (mand_ir_t *) 0 ) {
		tag_status = mand_ir_to_tag(rpfill->olevel_max,
						&rpfill->as.olevel_max);
		if ( tag_status == 0 ) {
			rpfill->as.olevel_max = (tag_t) 0;
		}
	}
	seteffprivs(s, NULL);

#endif /* SEC_MAC */

	if ( fwrite (&rpfill->as, sizeof (rpfill->as), 1, fp) != 1 )
		goto bad;
	for ( i = 0; i < rpfill->nusers; i++)
		if (rpfill->users[i][0] != '\0') {
			uid_t uid = pw_nametoid(rpfill->users[i]);

			if (fwrite(&uid, sizeof(uid_t), 1, fp) != 1)
				goto bad;
		}
	for ( i = 0; i < rpfill->ngroups; i++)
		if ( rpfill->groups[i][0] != '\0' ) {
			gid_t gid = gr_nametoid(rpfill->groups[i]);

			if (fwrite(&gid, sizeof(gid_t), 1, fp) != 1 )
				goto bad;
		}
	for ( i = 0; i < rpfill->nfiles; i++ )
		if ( rpfill->files[i][0] != '\0' ) {
			if ( fputs(rpfill->files[i], fp) == EOF )
				goto bad;
			if ( putc('\0', fp) == EOF )
				goto bad;
		}
	fclose (fp);
	
	/* AUDIT successful write of new reduction parameter file */
	sa_audit_audit (ES_AUD_MODIFY, "Audit reduction parameter file write");
	return(0);
	
bad:
	ErrorMessageOpen(3452, msg_aselection, 46, NULL);
	fclose(fp);
	forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_CHOWN,
#if SEC_MAC
		   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
		   SEC_ILNOFLOAT,
#endif
		   -1), s);
	unlink(filename);
	seteffprivs(s, NULL);
	/* AUDIT failure to write reduction parameter file */
	sa_audit_audit (ES_AUD_ERROR, msg_aselection[49]);
	return(1);
}

void
SelectionDeleteFile(rpfill)
AUDIT_SELECTION_STRUCT *rpfill;
{
	char    filename[sizeof(AUDIT_REDUCE_PARM_DIR) + ENTSIZ];
	privvec_t s;
	int ret;

	sprintf (filename, "%s%s", AUDIT_REDUCE_PARM_DIR, rpfill->filename);

	/* raise privileges necessary to delete the file */

	forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_CHOWN,
#if SEC_MAC
		   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
		   SEC_ILNOFLOAT,
#endif
		   -1), s);
	ret = unlink(filename);
	seteffprivs(s);

	if (ret < 0 ) {
		ErrorMessageOpen(3460, msg_aselection, 51, NULL);  

		/* AUDIT unsuccessful remove of reduction file */

		sa_audit_audit (ES_AUD_ERROR, msg_aselection[54]); } 
	else {
		/* AUDIT successful remove of reduction file */

		sa_audit_audit (ES_AUD_MODIFY, msg_aselection[56]);
	}
}

/* open a select file and read in its header */
static FILE *
SelectionOpenFile(file, as)
	char        *file;
	struct      audit_select    *as;
{
	char    filename[sizeof(AUDIT_REDUCE_PARM_DIR) + ENTSIZ];
	FILE    *fp;
	privvec_t s;

	sprintf (filename, "%s%s", AUDIT_REDUCE_PARM_DIR, file);

	/* Raise privileges necessary to open the file for reading */

	forceprivs(privvec(SEC_ALLOWDACACCESS, -1), s);
	fp = fopen(filename, "r");
	seteffprivs(s, NULL);

	if (fp == (FILE *) 0) {
		ErrorMessageOpen(3425, msg_aselection, 21, NULL);  
		/* "Unable to open parameter file for reading." */
		return (FILE *) 0;
	}
	if (fread(as, sizeof (*as), 1, fp) != 1) {
		ErrorMessageOpen(3426, msg_aselection, 23, NULL);  
		fclose (fp);
		return (NULL);
	}
	return (fp);
}

static void
SelectionFillTime(rpfill)
	AUDIT_SELECTION_STRUCT  *rpfill;
{
	struct  tm  *tbuf;

	if (rpfill->as.start > 0) {
		tbuf = localtime (&rpfill->as.start);
		(void) sprintf (rpfill->s_hour, "%.2d",  tbuf->tm_hour);
		(void) sprintf (rpfill->s_min, "%.2d",  tbuf->tm_min);
		(void) sprintf (rpfill->s_day, "%.2d",  tbuf->tm_mday);
		(void) sprintf (rpfill->s_year, "%.2d",  tbuf->tm_year);
		(void) strcpy (rpfill->s_month, months[tbuf->tm_mon]);
	} else {
		(void) strcpy (rpfill->s_hour, twozeros);
		(void) strcpy (rpfill->s_min, twozeros);
		strncpy (rpfill->s_month, "", sizeof (rpfill->s_month));
		(void) strcpy (rpfill->s_day, twozeros);
		(void) strcpy (rpfill->s_year, twozeros);
	}
	if (rpfill->as.stop > 0) {
		tbuf = localtime (&rpfill->as.stop);
		(void) sprintf (rpfill->e_hour, "%.2d",  tbuf->tm_hour);
		(void) sprintf (rpfill->e_min, "%.2d",  tbuf->tm_min);
		(void) sprintf (rpfill->e_day, "%.2d",  tbuf->tm_mday);
		(void) sprintf (rpfill->e_year, "%.2d",  tbuf->tm_year);
		(void) strcpy (rpfill->e_month, months[tbuf->tm_mon]);
	} else {
		(void) strcpy (rpfill->e_hour, twozeros);
		(void) strcpy (rpfill->e_min, twozeros);
		strncpy (rpfill->e_month, "", sizeof (rpfill->e_month));
		(void) strcpy (rpfill->e_day, twozeros);
		(void) strcpy (rpfill->e_year, twozeros);
	}
}

#if SEC_MAC
static void
BuildMinSL(rpfill)
	AUDIT_SELECTION_STRUCT  *rpfill;
{
	char        *temp_label;
	int         tag_status;
	privvec_t   s;

	/* raise privileges needed to map tags to IRs */

	forceprivs(privvec(SEC_ALLOWMACACCESS, -1), s);

	/* Copy the min sl. If one not set then use minsl */

	rpfill->slevel_min = mand_alloc_ir();
	if (rpfill->slevel_min == (mand_ir_t *) 0) 
		MemoryError();

	tag_status = 0;
	if (rpfill->as.slevel_min != (tag_t) 0)
		tag_status = mand_tag_to_ir(rpfill->as.slevel_min,
						 rpfill->slevel_min);
	if (tag_status == 0)
#if SEC_ENCODINGS
		mand_copy_ir(mand_minsl, rpfill->slevel_min);
#else
		mand_copy_ir(mand_syslo, rpfill->slevel_min);
#endif

	/* Now do the object min */
	rpfill->olevel_min = mand_alloc_ir();
	if (rpfill->olevel_min == (mand_ir_t *) 0) 
	MemoryError();
	tag_status = 0;
	if (rpfill->as.olevel_min != (tag_t) 0)
		tag_status = mand_tag_to_ir(rpfill->as.olevel_min,
						 rpfill->olevel_min);
	if (tag_status == 0)
#if SEC_ENCODINGS
		mand_copy_ir(mand_minsl, rpfill->olevel_min);
#else
		mand_copy_ir(mand_syslo, rpfill->olevel_min);
#endif

	/* return privileges to initial state */
	
	seteffprivs(s, NULL);
}

static void
BuildMaxSL(rpfill)
	AUDIT_SELECTION_STRUCT  *rpfill;
{
	char        *temp_label;
	int         tag_status;
	privvec_t   s;

	/* raise privileges necessary to map tags */

	forceprivs(privvec(SEC_ALLOWMACACCESS, -1), s);

	/* Copy the max sl. If one not set then use syshi */
	rpfill->slevel_max = mand_alloc_ir();
	if (rpfill->slevel_max == (mand_ir_t *) 0) 
		MemoryError();
	tag_status = 0;
	if (rpfill->as.slevel_max != (tag_t) 0)
		tag_status = mand_tag_to_ir(rpfill->as.slevel_max,
				 rpfill->slevel_max);
	if (tag_status == 0)
		mand_copy_ir(mand_syshi, rpfill->slevel_max);

	/* Now do the object max */
	rpfill->olevel_max = mand_alloc_ir();
	if (rpfill->olevel_max == (mand_ir_t *) 0) 
		MemoryError();
	tag_status = 0;
	if (rpfill->as.olevel_max != (tag_t) 0)
		tag_status = mand_tag_to_ir(rpfill->as.olevel_max,
			 rpfill->olevel_max);
	if (tag_status == 0)
		mand_copy_ir (mand_syshi, rpfill->olevel_max);
	seteffprivs(s, NULL);
}

#endif /* SEC_MAC */

static char **
GetEventTable(rpfill)
	AUDIT_SELECTION_STRUCT  *rpfill;
{
	char    **table, 
		**alloc_cw_table();
	int     i;

	table = alloc_cw_table(AUDIT_MAX_EVENT, 2); 
	if (table != (char **) 0) {
		for (i = 1; i <= AUDIT_MAX_EVENT; i++)
			if (ISBITSET(rpfill->as.event_mask, i))
				table[i-1][0] = YESCHAR;
			else    table[i-1][0] = NOCHAR;
	}
	return (table);
}

static char **
GetUserTable(rpfill, count, fp)
	AUDIT_SELECTION_STRUCT  *rpfill;
	int         count;
	FILE        *fp;
{
	char	**table;
	int     i;
	uid_t  uid;
	char   *user_name;

	table = alloc_cw_table(count, 9); 
	if (table != (char **) 0 && rpfill->as.uid_count > 0) {
		fseek(fp, rpfill->as.uid_offset, 0);
		for (i = 0; i < rpfill->as.uid_count; i++)  {
			if (fread (&uid, sizeof (uid), 1, fp) != 1) {
			    ErrorMessageOpen(3430, msg_aselection, 26, NULL);
				/* Cannot read uid from parameter file. */
				/* Please check consistency and re-run program*/
				return (table);
			}
			user_name = pw_idtoname(uid);
			if (user_name == NULL) {
			    ErrorMessageOpen(3431, msg_aselection, 29, NULL);  
				/* Cannot find userid in password file.*/
				/* Please check consistency and re-run program*/
				return (table);
			}
			strcpy (table[i], user_name);
		}
	}

	return (table);
}


static char **
GetGroupTable (rpfill, count, fp)
	AUDIT_SELECTION_STRUCT  *rpfill;
	int         count;
	FILE        *fp;
{
	int     i;
	char	**table;
	gid_t	gid;
	char	*group_name;

	table = alloc_cw_table(count, 9);
	if (table != (char **) 0 && rpfill->as.gid_count > 0) {
		fseek (fp, rpfill->as.gid_offset, 0);
		for (i = 0; i < rpfill->as.gid_count; i++)  {
			if (fread (&gid, sizeof (gid), 1, fp) != 1) {
			      ErrorMessageOpen(3432, msg_aselection, 32, NULL);
				/* Cannot read uid from parameter file." */
				return (table);
			}
			group_name = gr_idtoname(gid);
			if (group_name == NULL) {
			      ErrorMessageOpen(3433, msg_aselection, 35, NULL);
				/* "Cannot find group id in group file." */
				return (table);
			}
			strcpy (table[i], group_name);
		}
	}
	return (table);
}

static char **
GetFilesTable (rpfill, count, fp)
	AUDIT_SELECTION_STRUCT  *rpfill;
	int         count;
	FILE        *fp;
{
	int     c,
		i;
	char    *cp,
		**table, 
		**alloc_cw_table();

	table = alloc_cw_table (count, FILEWIDTH + 1); 

	/*
	 * If the table was successfully allocated and we are reading
	 * from a file there are filenames in the file, load them into
	 * the table.
	 */

	if (table != (char **) 0 && fp != (FILE *) 0 &&
			rpfill->as.object_count > 0) {
		fseek(fp, rpfill->as.object_offset, 0);
		for (i = 0; i < rpfill->as.object_count; i++)  {
			cp = table[i];
			do {
				c = getc (fp);
				*cp++ = c;
			} while (c != '\0' && c != EOF);
			if (c == EOF)  {
			      ErrorMessageOpen(3440, msg_aselection, 38, NULL);
				/* Unexpected EOF when reading parameter file */
				break;
			}
		}
	}
	return (table);
}

/* make sure no embedded spaces */

int
SelectionEditFiles (rpfill)
	AUDIT_SELECTION_STRUCT  *rpfill;
{
	int     i;
	char    *strchr();

	for (i = 0; i < rpfill->nfiles; i++) {
		if (rpfill->files[i][0] == '\0')
			continue;
		if (strchr (rpfill->files[i], ' ') != (char *) 0) {
   		   ErrorMessageOpen(3475, msg_aselection, 73, rpfill->files[i]);
			/* "Invalid file name: %s" */
			/* "File name contains spaces." */
			return (1);
		}
	}
	return (0);
}

int
SelectionEditTime (c_hour, c_min, c_day, c_month, c_year)
	char	*c_hour,
		*c_min,
		*c_day,
		*c_month,
		*c_year;
{
	int     p_hour, 
		p_min, 
		p_month, 
		p_day, 
		p_year;

	ParseDate (c_hour, c_min, c_month, c_day, c_year,
			&p_hour, &p_min, &p_month, &p_day, &p_year);

	if (p_hour < 0 || p_hour > 23) {
		ErrorMessageOpen(3470, msg_aselection, 58, NULL);
		/* "Hour value must be military time, between 0 and 23." */
		/* "Please re-enter correct hour value." */
		return(1);
	}

	if (p_min < 0 || p_min > 59) {
		ErrorMessageOpen(3471, msg_aselection, 61, NULL);
		/* "Minute value must be between 0 and 59." */
		/* "Please re-enter correct minute value." */
		return(1);
	}

	if (p_hour == 0 && p_min == 0 &&
		p_month == 0 && p_day == 0 && p_year == 0)
		return(0);
	if (p_month < 1 || p_month > 12) {
		ErrorMessageOpen(3472, msg_aselection, 64, NULL);
		/* "Month value must be a month name (e.g., Jan)." */
		/* "Please re-enter correct month value." */
	   return(1);
	}

	if (p_day <= 0 ||
		(p_month == 2 && (p_year % 4) != 0 && p_day > 28) ||
		p_day > mdays[p_month - 1])  {
		ErrorMessageOpen(3473, msg_aselection, 67, NULL);
		/* Day of month must be between 1 and the number of days" */
		/* in the month. Please re-enter correct day of month value." */
		return(1);
	}

	if (p_year < 70) {
		ErrorMessageOpen(3474, msg_aselection, 70, NULL);
		/* "Year must be greater than 70." */
		/* "Please enter a valid year value." */
		return(1);
	}
	return(0);
}

static void
ParseDate (chour, cmin, cmonth, cday, cyear, ihour, imin, imonth, iday, iyear)
	char	*chour, 
		*cmin, 
		*cmonth, 
		*cday, 
		*cyear;
	int	*ihour, 
		*imin, 
		*imonth, 
		*iday, 
		*iyear;
{
	int     i;

	*ihour = GetTwoDigits (chour);
	*imin = GetTwoDigits (cmin);
	*iday = GetTwoDigits (cday);
	*iyear = GetTwoDigits (cyear);
	*imonth = -1;
	if (cmonth[0] == '\0')
		*imonth = 0;
	else for (i = 0; i < NMONTHS; i++)
		/* allow upper case in second two digits */
		if (strncmp (cmonth, months[i], 3) == 0 ||
		   (cmonth[0] == months[i][0] &&
		   isupper (cmonth[1]) &&
			cmonth[1] == toupper (months[i][1]) &&
		   isupper (cmonth[2]) &&
			cmonth[2] == toupper (months[i][2]))) {
			*imonth = i + 1;
			break;
		}
}

/* convert a broken out time to seconds */

static time_t
SelectionGetTime (chour, cmin, cmonth, cday, cyear)
	char	*chour, 
		*cmin, 
		*cmonth, 
		*cday, 
		*cyear;
{
	int	hour, 
		min, 
		month, 
		day, 
		year,
		i;
	time_t  seconds = 0;

	ParseDate (chour, cmin, cmonth, cday, cyear,
				&hour, &min, &month, &day, &year);
	if (hour == 0 && min == 0 && month == 0 && day == 0 && year == 0)
		return ((time_t) 0);
	for (i = 70; i < year; i++) {
		seconds += 60 * 60 * 24 * 365;
		if ((i % 4) == 0)
			seconds += 60 * 60 * 24;
	}
	for (i = 1; i < month; i++) {
		if (i == 2)
			seconds += ((year % 4) == 0) ? 29 * 60 * 60 * 24 :
			                   28 * 60 * 60 * 24;
		else 
			seconds += mdays[i - 1] * 60 * 60 * 24;
	}        
	for (i = 1; i < day; i++)
		seconds += 24 * 60 * 60;
	for (i = 0; i < hour; i++)
		seconds += 60 * 60;
	for (i = 0; i < min; i++)
		seconds += 60;
	/* correct for difference between greenwich mean time */
	seconds += timezone;
	return (seconds);
}

/* get two digits from the field on the screen.
 * allow spaces, but do not allow any non-spaces.
 */

static int
GetTwoDigits (string)
	char        *string;
{
	char    c1 = string[0],
		c2 = string[1];

	if (c1 == '\0')
		return -1;
	if (isdigit(c1)) {
		if ( isdigit(c2) )
			return (c1 - '0') * 10 + (c2 - '0');
		if ( c2 != ' ' && c2 != '\0' )
			return -1;
		else
			return c1 - '0';
	}
	if ( c1 != ' ' )
		return -1;
	/* c1 is a space -> allow spaces or digits only for c2 */
	if (c2 == '\0' || c2 == ' ')
		return 0;
	if ( ! isdigit(c2) )
		return -1;
	return c2 - '0';
}

/*
 * retrieve the selection files in the reduction directory
 * return the selection files in a newly allocated table returned
 * through the parameters to the routine.
 * Returns 0 on success, 1 on permission erorr.
 */

int
SelectionFileGet(nfilep, file_tablep)
	int *nfilep;
	char ***file_tablep;
{

	char *cp;
	int file_len, file_count;
	int i;
	DIR *fp;
	struct dirent *d;
	struct a {
		char *filename;
		struct a *next;
	} *first = (struct a *) 0, *last, *ap;
	privvec_t s;
	char **file_names;
	char buf[sizeof(AUDIT_REDUCE_PARM_DIR) + NAME_MAX];
	struct audit_select au;
	struct stat sb;

	/* Open selection files directory */
	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);

	fp = opendir(AUDIT_REDUCE_PARM_DIR);

	if (fp == (DIR *) 0) {
		seteffprivs(s, NULL);
		ErrorMessageOpen(1730, msg_selection, 55, NULL);
		/* AUDIT failure to access AUDIT_REDUCE_PARM_DIR */
		audit_no_resource (AUDIT_REDUCE_PARM_DIR, OT_SUBSYS,
		  "Failure to open for read/write/search", ET_SYS_ADMIN);
		return (1);                                     
	}
	
	/* Scan selection files directory, count files as go */
	file_count = 0;
	file_len = 0;
	while (d = readdir(fp)) {
		FILE *tfp;

		cp = d->d_name;
#ifdef SCO
		/* There is a bug in readdir function on SCO platform */
		cp -= 2;
#endif

		/* skip the usual entries */

		if (strcmp(cp, ".") == 0 ||
		    strcmp (cp, "..") == 0 ||
		    d->d_fileno == 0)
			continue;

		/*
		 * Check whether the file is valid.
		 * The first word is the file size.
		 * The file must have at least an audit_select in it.
		 */

		sprintf(buf, "%s%s", AUDIT_REDUCE_PARM_DIR, cp);
		tfp = fopen(buf, "r");

		if (tfp == (FILE *) 0)
			continue;

		if (fstat(fileno(tfp), &sb) < 0) {
			fclose(tfp);
			continue;
		}

		if (fread((char *) &au, sizeof(au), 1, tfp) != 1) {
			fclose(tfp);
			continue;
		}

		fclose(tfp);

		if (au.sel_size != sb.st_size) {
			continue;
		}

		/* allocate a list entry for the filename for each valid file */

		ap = (struct a *) Malloc(sizeof(struct a));
		if (ap == (struct a *) 0)
			MemoryError();

		ap->filename = Malloc(strlen(cp) + 1);
		if (ap->filename == NULL)
			MemoryError();

		strcpy(ap->filename, cp);

		/* add to the end of the list */

		if (first == (struct a *) 0)
			first = ap;
		else
			last->next = ap;
		last = ap;

		/* save largest name length */

		if (strlen(cp) > file_len)
			file_len = strlen(cp);

		file_count++;
	}
	closedir(fp);

	/* only set up if we've found files */

	if (first != (struct a *) 0) {
		last->next = (struct a *) 0;

		/* allow space for the '\0' */

		file_len++;

		/* copy the names into a constant width table */

		file_names = alloc_cw_table(file_count, file_len);
		if (file_names == (char **) 0)
			MemoryError();

		/* walk the list, freeing as we go */

		for (ap = first, i = 0; ap; i++) {
			strcpy(file_names[i], ap->filename);
			first = ap->next;
			free(ap->filename);
			free((char *) ap);
			ap = first;
		}
	
		/* Alpha sort entries */

		sort_cw_table(file_names, file_len, file_count);
 
		*file_tablep = file_names;
	}
	else
		*file_tablep = (char **) 0;

	*nfilep = file_count;

	return 0;
}

void
SelectionFileStart()
{
	LoadMessage("msg_isso_audit_selection", &msg_selection,
			&msg_selection_text);
}

#endif /* SEC_BASE */
