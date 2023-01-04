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
static char *rcsid = "@(#)$RCSfile: getdvagent.c,v $ $Revision: 4.2.9.4 $ (DEC) $Date: 1993/08/04 21:23:32 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All Rights Reserved.
 */


/*
 * Based on:

 */

/*LINTLIBRARY*/


/*
 * This file contains * routines to implement a device assignment database.
 * The routines parallel those of the getpwent(3) routines for
 * the Device Assignment database.
 */

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#include <prot.h>

/* static memory to hold components of the device assignment database */

static struct dev_asg *dev_asg = (struct dev_asg *) 0;
static char **old_dev_list;
static char **old_user_list;
static char **old_name_list;
#if SEC_MAC
/* device-specific buffers */
static mand_ir_t *min_sl_buffer;
static mand_ir_t *max_sl_buffer;
static mand_ir_t *cur_sl_buffer;
/* system default buffers */
static mand_ir_t *d_min_sl_buffer;
static mand_ir_t *d_max_sl_buffer;
static mand_ir_t *d_cur_sl_buffer;
#endif /* SEC_MAC */

#if SEC_ILB
/* device-specific buffer */
static ilb_ir_t *cur_il_buffer;
/* system default buffer */
static ilb_ir_t *d_cur_il_buffer;
#endif /* SEC_ILB */

static long filepos = 0L;
static FILE *fp = (FILE *) 0;


static int store_dv_fields();
static int read_dv_fields();
static int parse_dv_field();



/*
 * Read the next entry of the File Control database.  If there is an
 * error or there are no more entries, return 0.
 */
struct dev_asg *
getdvagent()
{
	return getdvagnam((char *)0);
}

/* match a particular name in the device assignment database.
 * if there is an error or there are no more entries, return 0.
 */

struct dev_asg *
getdvagnam (nam)
char *nam;
{
	struct dev_asg *status = (struct dev_asg *) 0;
	char *buf;

	check_auth_parameters();

	if (!fp || !!nam)
		setdvagent();

	buf = agetdvag (&filepos, fp, nam);
	if (nam && !buf && strcmp(nam, "*") && strlen(nam) < 6) {
		status = getdvagnam("*");
		if (status) {
			char devnam[5+3+1+2];	/* /dev/tty +\0 +2 */
			if (old_name_list)
				free((char *)old_name_list);
			old_name_list = agetstrlist(nam);
			if (old_name_list) {
				status->ufld.fd_name = *old_name_list;
				status->uflg.fg_name = 1;
			}
			if (old_dev_list)
				free((char *)old_dev_list);
			(void) strncat(strcpy(devnam,"/dev/"),
					nam, sizeof devnam - 1);
			devnam[sizeof devnam - 1] = '\0';
			old_dev_list = agetstrlist(devnam);
			if (old_dev_list) {
				status->uflg.fg_devs = 1;
				status->ufld.fd_devs = old_dev_list;
			}
		}
	}
	if (buf && read_dv_fields(&dev_asg->ufld, &dev_asg->uflg, buf, 0) == 0)
	{
		setprdfent();
		buf = agetdefault_buf();
		if (buf &&
		    read_dv_fields(&dev_asg->sfld, &dev_asg->sflg, buf, 1) == 0)
				status = dev_asg;
	}

	return status;
}

/*
 * Reset the position of the Device Assignment database so that the
 * next time getdvagent() is invoked, it will return the first entry
 * in the database.
 */
void
setdvagent()
{
	static time_t modify_time;
	struct stat sb;
	char *filename;
	int ret;

	check_auth_parameters();

	if (fp == (FILE *) 0) {
		open_auth_file((char *) 0, OT_DEV_ASG, &fp);
		if (fp != (FILE *) 0) {
			fstat (fileno(fp), &sb);
			modify_time = sb.st_mtime;
		}
	} else {
		filename = find_auth_file ((char *) 0, OT_DEV_ASG);
		ret = stat (filename, &sb);
		if (ret != 0 || sb.st_mtime > modify_time) {
			(void) fclose (fp);
			open_auth_file((char *) 0, OT_DEV_ASG, &fp);
			if (fp != (FILE *) 0) {
				fstat (fileno(fp), &sb);
				modify_time = sb.st_mtime;
			}
		}
		free (filename);
	}
	filepos = 0L;
	if (dev_asg == (struct dev_asg *) 0) {
		dev_asg = (struct dev_asg *) calloc(sizeof (*dev_asg), 1);
		if (dev_asg == (struct dev_asg *) 0) {
			enddvagent();
		}
	}
}

/*
 * Close the file related the to the Device Assignment database.
 */
void
enddvagent()
{
	check_auth_parameters();

	if (fp != (FILE *) 0)  {
		(void) fclose(fp);
		fp = (FILE *) 0;
	}
	filepos = 0L;
	if (dev_asg != (struct dev_asg *) 0) {
		free ((char *) dev_asg);
		dev_asg = (struct dev_asg *) 0;
#if SEC_MAC
		if (min_sl_buffer) {
			mand_free_ir(min_sl_buffer);
			min_sl_buffer = (mand_ir_t *) 0;
		}
		if (d_min_sl_buffer) {
			mand_free_ir(d_min_sl_buffer);
			d_min_sl_buffer = (mand_ir_t *) 0;
		}
		if (max_sl_buffer) {
			mand_free_ir(max_sl_buffer);
			max_sl_buffer = (mand_ir_t *) 0;
		}
		if (d_max_sl_buffer) {
			mand_free_ir(d_max_sl_buffer);
			d_max_sl_buffer = (mand_ir_t *) 0;
		}
		if (cur_sl_buffer) {
			mand_free_ir(cur_sl_buffer);
			cur_sl_buffer = (mand_ir_t *) 0;
		}
		if (d_cur_sl_buffer) {
			mand_free_ir(d_cur_sl_buffer);
			d_cur_sl_buffer = (mand_ir_t *) 0;
		}
#endif /* SEC_MAC */
#if SEC_ILB
                if (cur_il_buffer) {
                        ilb_free_ir(cur_il_buffer);
                        cur_il_buffer = (ilb_ir_t *) 0;
                }
                if (d_cur_il_buffer) {
                        ilb_free_ir(d_cur_il_buffer);
                        d_cur_il_buffer = (ilb_ir_t *) 0;
                }
#endif /* SEC_ILB */
		if (old_name_list) {
			free((char *) old_name_list);
			old_name_list = (char **) 0;
		}
		if (old_dev_list) {
			free((char *) old_dev_list);
			old_dev_list = (char **) 0;
		}
		if (old_user_list) {
			free((char *) old_user_list);
			old_user_list = (char **) 0;
		}
	}
	end_authcap (OT_DEV_ASG);
}

/*
 * Place an entry into the Device Assignment database under the given
 * file name.  Replace an existing entry if the names compare or add
 * this entry at the end.  (The entry is deleted if the fg_devname
 * is 0.)  Lock the entire Authentication database for this operation.
 * When done, the File Control database is closed.
 */
int
putdvagnam(nam, p)
	register char *nam;
	register struct dev_asg *p;
{
	register FILE *tempfile;
	register int replaced;
	register int status;
	register char *pathname;
	register int cfs_status;
	char *temppathname;
	char *oldpathname;
	char filebuf[BUFSIZ];
	char continuation;

	check_auth_parameters();

	status = 0;
	replaced = 0;

	setdvagent();

	pathname = find_auth_file(nam, OT_DEV_ASG);

	if (!make_transition_files(pathname, &temppathname, &oldpathname))  {
		enddvagent();
		free(pathname);
		return (0);
	}

	cfs_status = create_file_securely(temppathname, AUTH_VERBOSE,
			     MSGSTR(GETDVAGENT_1, "make new Device Assignment database"));
	if (cfs_status != CFS_GOOD_RETURN) {
		enddvagent();
		free(pathname);
		free(temppathname);
		free(oldpathname);
		return(0);
	}

	/* now file is locked.  Reference the current database */

	tempfile = fopen(temppathname, "w");
	if (tempfile == (FILE *) 0)  {
		free(temppathname);
		free(oldpathname);
	}
	else  {
		status = 1;
		fseek (fp, 0L, 0);
		while (status &&
		   (fgets (filebuf, sizeof (filebuf), fp) != (char *) 0)) {
			if (ackentname (nam, filebuf)) {
				(void) fseek (fp, filepos, 0);
				(void) agetdvag (&filepos, fp, nam);
				if(p->uflg.fg_name)
					status = store_dv_fields (tempfile, nam,
					 &p->ufld, &p->uflg);
				replaced = 1;
			} else do {
				status = fputs (filebuf, tempfile) >= 0;
				continuation =
				  (filebuf[strlen(filebuf)-1] != '\n') ||
				  (filebuf[strlen(filebuf) - 2] == '\\');
				if (continuation)
					continuation =
					!!fgets (filebuf, sizeof (filebuf), fp);
				filepos = ftell (fp);
			} while (continuation && status);
		}

		if (status && !replaced)
			if(p->uflg.fg_name)
			     status = store_dv_fields(tempfile, nam, &p->ufld,
						 &p->uflg);

		status = (fclose(tempfile) == 0) && status;


		if (status)
			status = replace_file(temppathname, pathname,
				oldpathname);
		else {
			(void) unlink(temppathname);
			free(temppathname);
			free(oldpathname);
		}

	}

	free(pathname);

	enddvagent();

	return status;
}

/*
 * allocate a new device assignment database entry, and copy an existing
 * entry into it.
 * For alignment, the unaligned strings are at the end of the dynamicall
 * allocated area, while the aligned structures are immediately after the
 * device assignment database entry.  During calculations, str_count
 * maintains the total string size, while align_count keeps the size
 * of the aligned object area.
 */

struct dev_asg *
copydvagent(dv)
register struct dev_asg *dv;
{
	register struct dev_asg *rdv;
	register char *cp;
	char *ocp;
	int i;
	int str_count = 0;
	char *str_cp;
	int align_count = 0;
	char *align_cp;
	int ndevs = 0;
	int nusers = 0;

	if (dv == (struct dev_asg *) 0)
		return (dv);

	/* count up the total string requirements for the new structure
	 */
	if (dv->uflg.fg_devs) {
		for (i = 0; cp = dv->ufld.fd_devs[i]; i++)
			str_count += strlen(cp) + 1;
		ndevs = i + 1;
		align_count += ndevs * sizeof(char *);
	}

	if (dv->uflg.fg_users) {
		for (i = 0; cp = dv->ufld.fd_users[i]; i++)
			str_count += strlen(cp) + 1;
		nusers = i + 1;
		align_count += nusers * sizeof(char *);
	}

	if (dv->uflg.fg_name)
		str_count += strlen(dv->ufld.fd_name) + 1;

#if SEC_MAC
	align_count += (dv->uflg.fg_min_sl +
			dv->uflg.fg_max_sl +
			dv->uflg.fg_cur_sl +
			dv->sflg.fg_min_sl +
			dv->sflg.fg_max_sl +
			dv->sflg.fg_cur_sl) * mand_bytes();
#endif

#if SEC_ILB
        align_count += (dv->uflg.fg_cur_il +
                        dv->sflg.fg_cur_il) * ilb_bytes();
#endif


	/* allocate a new structure and copy strings into it */

	rdv = (struct dev_asg *)
		calloc(sizeof *rdv + str_count + align_count, 1);
	if (rdv == (struct dev_asg *) 0) {
		return ((struct dev_asg *) 0);
	}

	/* set "aligned items" and "unaligned strings" pointers */

	align_cp = (char *) ((unsigned long) rdv + sizeof(struct dev_asg));
	str_cp = (char *) ((unsigned long) align_cp + align_count);
	*rdv = *dv;

	if (dv->uflg.fg_devs) {
		rdv->ufld.fd_devs = (char **) align_cp;
		align_cp += ndevs * sizeof(char *);
		for (i = 0; ocp = dv->ufld.fd_devs[i]; i++) {
			rdv->ufld.fd_devs[i] = str_cp;
			strcpy(str_cp, ocp);
			str_cp += strlen(str_cp) + 1;
		}
		rdv->ufld.fd_devs[i] = (char *) 0;
	}

	if (dv->uflg.fg_users) {
		rdv->ufld.fd_users = (char **) align_cp;
		align_cp += nusers * sizeof(char *);
		for (i = 0; ocp = dv->ufld.fd_users[i]; i++) {
			rdv->ufld.fd_users[i] = str_cp;
			strcpy(str_cp, ocp);
			str_cp += strlen(str_cp) + 1;
		}
		rdv->ufld.fd_users[i] = (char *) 0;
	}

	if (dv->uflg.fg_name) {
		rdv->ufld.fd_name = str_cp;
		strcpy(str_cp, dv->ufld.fd_name);
		str_cp += strlen(str_cp) + 1;
	}

/* macro to copy aligned data to the new structure */

#define COPY_DATA(flg, fld, flg_val, fld_val, type, len) \
	if (dv->flg.flg_val) { \
		rdv->fld.fld_val = (type *) align_cp; \
		align_cp += len; \
		memcpy((char *)rdv->fld.fld_val, \
			(char *)dv->fld.fld_val, len); \
	}

#if SEC_MAC
	COPY_DATA(uflg, ufld, fg_min_sl, fd_min_sl, mand_ir_t, mand_bytes());
	COPY_DATA(sflg, sfld, fg_min_sl, fd_min_sl, mand_ir_t, mand_bytes());
	COPY_DATA(uflg, ufld, fg_max_sl, fd_max_sl, mand_ir_t, mand_bytes());
	COPY_DATA(sflg, sfld, fg_max_sl, fd_max_sl, mand_ir_t, mand_bytes());
	COPY_DATA(uflg, ufld, fg_cur_sl, fd_cur_sl, mand_ir_t, mand_bytes());
	COPY_DATA(sflg, sfld, fg_cur_sl, fd_cur_sl, mand_ir_t, mand_bytes());
#endif

#if SEC_ILB
	COPY_DATA(uflg, ufld, fg_cur_il, fd_cur_il, ilb_ir_t, ilb_bytes());
	COPY_DATA(sflg, sfld, fg_cur_il, fd_cur_il, ilb_ir_t, ilb_bytes());
#endif
	return (rdv);
}

/* This enumerated type determines the order in the parsing table */

enum {
	DeviceOffset = 0,
	TypeOffset,
	UsersOffset
#if SEC_MAC
      , MaxSLOffset,
	MinSLOffset,
	CurSLOffset
#endif
#if SEC_ILB
      , CurILOffset
#endif
#if SEC_ARCH
      , AssignOffset
#endif
	};

static char *dev_table[] = {
	AUTH_V_DEVICES,
	AUTH_V_TYPE,
	AUTH_V_USERS
#if SEC_MAC
	, AUTH_V_MAX_SL
	, AUTH_V_MIN_SL
	, AUTH_V_CUR_SL
#endif
#if SEC_ILB
	, AUTH_V_CUR_IL
#endif
#if SEC_ARCH
	, AUTH_V_ASSIGN
#endif

};

/* offsets into the above array */

#define DEVICES		(int) DeviceOffset
#define TYPE		(int) TypeOffset
#define	USERS		(int) UsersOffset
#if SEC_MAC
#define MAX_SL		(int) MaxSLOffset
#define MIN_SL		(int) MinSLOffset
#define CUR_SL		(int) CurSLOffset
#endif
#if SEC_ILB
#define CUR_IL		(int) CurILOffset
#endif
#if SEC_ARCH
#define ASSIGN		(int) AssignOffset
#endif

#define DEV_TABLE_SIZE (sizeof (dev_table) / sizeof (dev_table[0]))

static int
read_dv_fields (fld, flg, cp, is_default)
struct dev_field *fld;
struct dev_flag  *flg;
register char *cp;
int is_default;		/* is this the system default entry? */
{
	register int i;
	char *valbuf;
	register int status;

	/* zero out the structures passed */
	memset((char *) fld, '\0', sizeof (*fld));
	memset((char *) flg, '\0', sizeof (*flg));

	/* store the name field */
	if (is_default || (*cp == ':')) {
		fld->fd_name = (char *) 0;
		flg->fg_name = 0;
	}
	else {
		char **names;
		names = agetnmlist();
		if (names) {
			fld->fd_name = *names;
			flg->fg_name = 1;
		}
		if (old_name_list)
			free((char *)old_name_list);
		old_name_list = names;
	}
	/* store the other fields */
	valbuf = 0;
	for (i = 0;  i < DEV_TABLE_SIZE;  i++) {
		if ((cp = agetstr(dev_table[i], &valbuf)) != NULL) {
			status = parse_dv_field(cp, i, fld, flg, is_default);
			free(cp);
			if (status) return 1;
			valbuf = 0;
		}
	}
	return (0);
}

/* load the fields into the structure dev_asg.
 * return 0 if OK, 1 if error.
 */

static int
parse_dv_field (field, index, fld, flg, is_default)
char *field;
int index;
struct dev_field *fld;
struct dev_flag  *flg;
int is_default;		/* is this the default entry? */
{
	char **list;
	char **user_list;
	int ret = 0;
#if SEC_MAC
	mand_ir_t *sl;
#endif
#if SEC_ILB
	ilb_ir_t *il;
#endif

	switch (index) {
	case DEVICES:
		/* system default device list not recognized */

		if (is_default)
			break;

		if (old_dev_list != (char **) 0)
			free ((char *) old_dev_list);
		list = agetstrlist (field);
		if (list != (char **) 0) {
			fld->fd_devs = list;
			flg->fg_devs = 1;
		} else
			ret = 1;
		old_dev_list = list;
		break;

	case USERS:
		/* system default user list not recognized */

		if (is_default)
			break;

		if (old_user_list != (char **) 0)
			free ((char *) old_user_list);
		user_list = agetstrlist(field);
		if (user_list != (char **) 0) {
			fld->fd_users = user_list;
			flg->fg_users = 1;
		} else
			ret = 1;
		old_user_list = user_list;
		break;

#if SEC_MAC
	case MAX_SL:

		/* copy into the existing max sl buffer, if already allocated */

		sl = mand_er_to_ir(field);
		if (sl != (mand_ir_t *) 0) {
			if (is_default)
				if (d_max_sl_buffer == (mand_ir_t *) 0)
					d_max_sl_buffer = sl;
				else {
					mand_copy_ir(sl, d_max_sl_buffer);
					mand_free_ir(sl);
				}
			else
				if (max_sl_buffer == (mand_ir_t *) 0)
					max_sl_buffer = sl;
				else {
					mand_copy_ir(sl, max_sl_buffer);
					mand_free_ir(sl);
				}
			fld->fd_max_sl = is_default ?
						d_max_sl_buffer : max_sl_buffer;
			flg->fg_max_sl = 1;
		}
		break;

	case MIN_SL:
		sl = mand_er_to_ir(field);
		if (sl != (mand_ir_t *) 0) {
			if (is_default)
				if (d_min_sl_buffer == (mand_ir_t *) 0)
					d_min_sl_buffer = sl;
				else {
					mand_copy_ir(sl, d_min_sl_buffer);
					mand_free_ir(sl);
				}
			else
				if (min_sl_buffer == (mand_ir_t *) 0)
					min_sl_buffer = sl;
				else {
					mand_copy_ir(sl, min_sl_buffer);
					mand_free_ir(sl);
				}
			fld->fd_min_sl = is_default ?
						d_min_sl_buffer : min_sl_buffer;
			flg->fg_min_sl = 1;
		}
		break;

	case CUR_SL:
		sl = mand_er_to_ir(field);
		if (sl != (mand_ir_t *) 0) {
			if (is_default)
				if (d_cur_sl_buffer == (mand_ir_t *) 0)
					d_cur_sl_buffer = sl;
				else {
					mand_copy_ir(sl, d_cur_sl_buffer);
					mand_free_ir(sl);
				}
			else
				if (cur_sl_buffer == (mand_ir_t *) 0)
					cur_sl_buffer = sl;
				else {
					mand_copy_ir(sl, cur_sl_buffer);
					mand_free_ir(sl);
				}
			fld->fd_cur_sl = is_default ?
						d_cur_sl_buffer : cur_sl_buffer;
			flg->fg_cur_sl = 1;
		}
		break;
#endif

#if SEC_ARCH
	case ASSIGN:
		loadnamepair (fld->fd_assign, AUTH_MAX_DEV_ASSIGN,
		  field, auth_dev_assign, AUTH_DEV_ASSIGN, OT_DEV_ASG,
		  fld->fd_name);
		flg->fg_assign = 1;
		break;
#endif
#if SEC_ILB
	case CUR_IL:
		fld-> fd_cur_il = ilb_er_to_ir(field);
		if (fld-> fd_cur_il != (ilb_ir_t *) 0)  {
			flg-> fg_cur_il = 1;
		}
		break;

#endif
	case TYPE:
		loadnamepair (fld->fd_type, AUTH_MAX_DEV_TYPE,
		  field, auth_dev_type, AUTH_DEV_TYPE, OT_DEV_ASG,
		  fld->fd_name);
		flg->fg_type = 1;
		break;
	}
	return (ret);
}

static int
store_dv_fields (f, name, fld, flg)
FILE *f;
char *name;
register struct dev_field *fld;
register struct dev_flag *flg;
{
	int error;
	int fields = 1;
	char	*namelist;
#if SEC_MAC || SEC_ILB
	char	*level_er;
#endif

	fields = aptentnameq(f, &error, fields, name);

	if (!error && flg->fg_devs) {
		fields = aptentslist(f, &error, fields, AUTH_V_DEVICES,
					fld->fd_devs);
	}

	if (!error && flg->fg_users) {
		fields = aptentslist(f, &error, fields, AUTH_V_USERS,
					fld->fd_users);
	}

#if SEC_MAC
	if (!error && flg->fg_max_sl)  {
		level_er = mand_ir_to_er(fld->fd_max_sl);
		if (level_er) {
			fields = aptentstr(f, &error, fields, AUTH_V_MAX_SL,
						level_er);
		}
		else
			error = 1;
	}

	if (!error && flg->fg_min_sl)  {
		level_er = mand_ir_to_er(fld->fd_min_sl);
		if (level_er) {
			fields = aptentstr(f, &error, fields, AUTH_V_MIN_SL,
						level_er);
		}
		else
			error = 1;
	}

	if (!error && flg->fg_cur_sl)  {
		level_er = mand_ir_to_er(fld->fd_cur_sl);
		if (level_er) {
			fields = aptentstr(f, &error, fields, AUTH_V_CUR_SL,
						level_er);
		}
		else
			error = 1;
	}
#endif

#if SEC_ARCH
	if (!error && flg->fg_assign) {
		fields = aptentnmpair(f, &error, fields, AUTH_V_ASSIGN,
					fld->fd_assign, AUTH_MAX_DEV_ASSIGN,
					auth_dev_assign, AUTH_DEV_ASSIGN);
	}
#endif
#if SEC_ILB
	if (!error && flg-> fg_cur_il)  {
		level_er = ilb_ir_to_er(fld-> fd_cur_il);
		if (level_er) {
			fields = aptentstr(f, &error, fields, AUTH_V_CUR_IL,
						level_er);
		}
		else
			error = 1;
	}
#endif

	if (!error && flg->fg_type)  {
		fields = aptentnmpair(f, &error, fields, AUTH_V_TYPE,
					fld->fd_type, AUTH_MAX_DEV_TYPE,
					auth_dev_type, AUTH_DEV_TYPE);
	}

	if (name)
		return aptentfin(f, &error);
	
	error = (fflush(f) != 0) || error;

	return !error;
}

/* #endif */ /*} SEC_BASE */
