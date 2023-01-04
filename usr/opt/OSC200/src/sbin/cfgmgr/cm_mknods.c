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
static char	*sccsid = "@(#)$RCSfile: cm_mknods.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/04/17 10:52:46 $";
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

#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <stdio.h>
#include <AFdefs.h>
#include <dirent.h>
#include <syslog.h>
#include <errno.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mode.h>

#include "cm.h"

/*
 * This constant defines the number of device special file names that can 
 * be associated with a particular device.  (Block and character are made
 * separately, so the total for a driver that uses both block and char is
 * twice that.  This constant then controls the allowable range of devices.
 * for example the range may be specified as [0-512].
 */
#define	MAX_DEVNAMES	513

#define	IS_DEVICE(x)	(S_ISCHR((x).st_mode) || S_ISBLK((x).st_mode))
#define	IS_DIRECTORY(x)	(S_ISDIR((x).st_mode))
#define IS_CHARDEV(x)   (S_ISCHR((x).st_mode))
#define IS_BLKDEV(x)    (S_ISBLK((x).st_mode))

int	mknod_cnt;
int	rmnod_cnt;


int
cm_mkdirpath ( char * dir, mode_t dmode )
{
	char *	dirp;
                                    		/* Skip any leading "/"'s */
	for (dirp = dir; *dirp == '/'; dirp++)
		continue;
						/* Iterate thru @ component */
	while ((dirp = (char *)NLstrchr(dirp, '/')) != '\0') {
		*dirp = '\0';
						/* mkdir, may err if exists */
		if (mkdir((char *)dir, dmode) != 0 && errno != EEXIST)
			return(MKNODS_EPERM);
						/* put "/" back, skip "/"'s */
		for (*dirp++ = '/'; *dirp == '/'; dirp++)
			continue;
	}
	return(0);
}


void
cm_mknods_destroy_major( cm_log_t * logp, char * entname, ulong op_flags, 
		char * dirname, int majno, mode_t mode, int depth )
{
	struct stat 	statbuf;
	struct dirent * dp;
	DIR *		dirp;
	char 		path[PATH_MAX];
	int		rc;

	if (majno < 0 || dirname == NULL || (dirp=opendir(dirname)) == NULL)
		return;

	for ( ; (dp=readdir(dirp)) != NULL; ) {

		strcpy(path, dirname);
		strcat(path, "/");
		strcat(path, dp->d_name);

		if (stat(path, &statbuf))
			continue;

		if (IS_DEVICE(statbuf) && majno == major(statbuf.st_rdev)) {
		    /*
		     * Only delete files of the same typel ie only delete
		     * block device special files when adding a block 
		     * driver.
		     */
		    if ((IS_CHARDEV(statbuf) && (mode == S_IFCHR)) ||
			    (IS_BLKDEV(statbuf) && (mode == S_IFBLK))) {
			rc = unlink(path);
			rmnod_cnt++;

			if (logp == NULL || !(op_flags & CM_RPT_RMNOD))
				continue;

			if (rmnod_cnt == 1 && (op_flags & CM_RPT_HEADER))
				cm_log(logp, LOG_ERR, cm_msg(MKNODS_REMOVING),
					entname);
			if (rc)
				cm_log(logp, LOG_ERR, cm_msg(MKNODS_EREMOVED),
					entname, path);
			else
				cm_log(logp, LOG_ERR, cm_msg(MKNODS_REMOVED),
					entname, path);
                    }

		} else if (IS_DIRECTORY(statbuf)) {
			if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,".."))
				continue;
			if (depth++ > 5)
				continue;
			cm_mknods_destroy_major(logp, entname, op_flags,
					path, majno, mode, depth);
		}
    	}
    	closedir(dirp);
    	return;
}

/*
 *
 */
int
cm_mknods( cm_log_t * logp, char * entname, ulong op_flags, 
		cm_devices_t * devices )
{
	char *	fname;
	char *	p;
	int	i;
	int	devindex;
	int	minno;
	int	majno;
	int	rc;
	int	num;
	int	mode;
	int	error;
	struct stat 	statbuf;
	char 		filepath[PATH_MAX];
	device_names_t  devnamelst;
	device_minors_t devminorlst;
	int     	margv_vec[MAX_DEVNAMES];
	char *		dargv_vec[MAX_DEVNAMES];
	mode_t  current_umask;
	mode_t  file_mode;

	/*
	 *	Initialize device name and minor list structures
	 */
	for (devindex=0; devindex < MAX_DEVNAMES; devindex++) {
		if ((dargv_vec[devindex] =
			(char *)calloc(NAME_MAX, sizeof(char))) == NULL) {
			error = MKNODS_ENOMEM;
			goto leave;
		}
	}
	devnamelst.dargv = dargv_vec;
	devnamelst.dargc = 0;
	devnamelst.dsiz = MAX_DEVNAMES;
	devnamelst.derr = 0;

	devminorlst.margv = margv_vec;
	devminorlst.margc = 0;
	devminorlst.msiz = MAX_DEVNAMES;
	devminorlst.merr = 0;

	/*
	 *	Expand device name and minor expressions into list structures
	 */
	(void) dbattr_mkdevnames(devices->devfiles, &devnamelst);
	(void) dbattr_mkdevminors(devices->devminors, &devminorlst);

	/*
	 *	Check for errors in expending expresssions
	 */
	if (devnamelst.derr) {
		error = devnamelst.derr;
		goto leave;
	}
	if (devminorlst.merr) {
		error = devminorlst.merr;
		goto leave;
	}

	if ((num=devnamelst.dargc) != devminorlst.margc) {
		error = MKNODS_EINVAL;
		goto leave;
	}
	if ((majno=devices->majno) < 0) {
		error = MKNODS_ENOENT;
		goto leave;
	}
						/* Generate basepath */
	filepath[0] = '\0';
	if (devices->dir != NULL) {
		strcpy(filepath, devices->dir);
		strcat(filepath, "/");
	}
	if (devices->subdir != NULL) {
		strcat(filepath, devices->subdir);
		strcat(filepath, "/");
	}
	if (filepath[0] == '\0') {
		error = MKNODS_ENOENT;
		goto leave;
	}

					/* Remove old MAJOR device files */
	mknod_cnt = 0;
	rmnod_cnt = 0;
	/*
	 * Pass the mode parameter to the cm_mknods_destroy_major
	 * routine so that it will only remove block files for a block driver
	 * and char files for a char driver.  This way if you have different
	 * drivers at the same major numbers for block/char they won't
	 * clobber each other's files.
	 */
	if (devices->type == DEVTYPE_BLK) {
		file_mode = S_IFBLK;
	}
	else if (devices->type == DEVTYPE_CHR) {
		file_mode = S_IFCHR;
	}
	else { 
		file_mode = 0;
	}
	if (op_flags & CM_RMNOD_MAJR) {
		if (op_flags & CM_MKNOD_FILE)
			cm_mknods_destroy_major(NULL, entname, op_flags,
				devices->dir, devices->majno, file_mode, 0);
		else
			cm_mknods_destroy_major(logp, entname, op_flags,
				devices->dir, devices->majno, file_mode, 0);
	}

					/* No device files to rm or mk */
	if ((op_flags & (CM_RMNOD_FILE|CM_MKNOD_FILE)) == 0) {
		error = 0;
		goto leave;
	}

					/* Make device path, if needed */
	if (op_flags & CM_MKNOD_FILE && stat(filepath, &statbuf)) {
		if (errno == ENOENT) {
			if (error=cm_mkdirpath(filepath, DIRMODE_DFLT))
				goto leave;
		}
	}
					/* Loop thru device file list */
	p = filepath +strlen(filepath);
	mode = devices->mode | devices->type;

	if (!(op_flags & CM_RMNOD_FILE))
		goto make;

    	for (i=0; i < num; i++) {
		fname = devnamelst.dargv[i];
		minno = devminorlst.margv[i];

		if (fname == NULL || *fname == '\0' || minno < 0)
			continue;

		strcpy(p, fname);

		if (stat(filepath, &statbuf))
			continue;

		rc = unlink(filepath);
		rmnod_cnt++;

		if (logp == NULL || !(op_flags & CM_RPT_RMNOD))
			continue;

		if (!(op_flags & CM_MKNOD_FILE))
			continue;

		if (rmnod_cnt == 1 && (op_flags & CM_RPT_HEADER)) 
			cm_log(logp, LOG_ERR, cm_msg(MKNODS_REMOVING),
				entname);
		if (rc)
			cm_log(logp, LOG_ERR, cm_msg(MKNODS_REMOVED),
				entname, filepath);
		else
			cm_log(logp, LOG_ERR, cm_msg(MKNODS_EREMOVED),
				entname, filepath);
	}

make:
	if (!(op_flags & CM_MKNOD_FILE))
		goto leave;

    	for (i=0; i < num; i++) {
		fname = devnamelst.dargv[i];
		minno = devminorlst.margv[i];

		if (fname == NULL || *fname == '\0' || minno < 0)
			continue;

		strcpy(p, fname);

		/*
		 * Bug fix: prior to doing the mknod set umask to 0 so
		 * that the permissions specified in the stanza entry
		 * take affect.  Otherwise those permissions would have
		 * been masked off by the umask of root at the time that
		 * cfgmgr had been started or restarted.
		 * Save off old umask, perform the mknod, then restore umask.
		 */
		current_umask = umask(0);
		rc = mknod(filepath, mode, makedev(majno, minno));
		umask(current_umask);

		mknod_cnt++;

		/* Set user and group per stanza entry */
		if (chown(filepath, devices->uid, devices->gid)) {
			   cm_log(logp, LOG_ERR, cm_msg(MKNODS_ECHOWN),
			   entname, filepath, devices->uid, devices->gid);
		}

		if (logp == NULL || !(op_flags & CM_RPT_MKNOD)) 
			continue;

		if (mknod_cnt == 1 && (op_flags & CM_RPT_HEADER)) 
			cm_log(logp, LOG_ERR, cm_msg(MKNODS_MAKING), entname);

		if (rc)
			cm_log(logp, LOG_ERR, cm_msg(MKNODS_ECREATE),
				entname, filepath, majno, minno);
		else
			cm_log(logp, LOG_ERR, cm_msg(MKNODS_CREATED),
				entname, filepath, majno, minno);
	}
	error = 0;

leave:
	for(i=0; i < devindex; i++)
		(void) free(dargv_vec[i]);
	return(error);
}
