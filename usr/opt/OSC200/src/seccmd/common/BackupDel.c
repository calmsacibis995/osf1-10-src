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
static char	*sccsid = "@(#)$RCSfile: BackupDel.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:59:17 $";
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



#include <sys/secdefines.h>

#if SEC_BASE

/* Common functions for backup and delete of audit sessions */

/*
	entry points:
		BackupDeleteStart()
		BackupDeleteOpen()
		BackupDeleteClose()
		BackupDeleteStop()
		
	notes:
		code supported a combined Backup/Delete option prior to 8/30/89
		this code to generate the combined button was pulled
			to allow greater safety with mltape and error passing
		I left remaining code in and also in message file in case decide
			to implement at a latter date
		
*/

#include "gl_defs.h"
#include "IfAudit.h"
#include <sys/secioctl.h>
#ifdef SCO
#include <unistd.h>
#endif

/* Default backup device should be configurable in future release !!!! */

#ifdef AUX
#define DEFAULT_DEVICE "/dev/rfloppy0"
#else
#define DEFAULT_DEVICE "/dev/rfloppy0"
#endif

#if SEC_ARCH
#define BACKUP_PROGRAM     "/bin/mltape"
#define BACKUP_ARGUMENTS   "-ovBO"
#else
#define BACKUP_PROGRAM     "/bin/cpio"
#define BACKUP_ARGUMENTS   "-ovB"
#endif
#define BACKUP_TEMPLATE    "/tmp/auXXXXXX"

#define BACKUP_DELETE 2
#define BACKUPREQ     1
#define DELETEREQ     0

static char   
	*devicename,
	**msg,
	*msg_text;

static int
	BackupFiles(),
	CheckDeviceWrite(),
	DeleteFiles();
		
void 
BackupDeleteStart()
{
	/* Load message text if not already in */
	if (! msg)
		LoadMessage("msg_isso_audit_backupdel", &msg, &msg_text);
		
}    
	
void 
BackupDeleteEnd()
{
	/* nothing to do */
}                                                        

/*
 * backup a list of files as described by the character string table file_list
 * builds the command that backs up the files and uses popen_all_output()
 * to send the command results to the screen.  The X and terminfo screens
 * implement different versions of popen_all_output() to provide
 * the appropriate screen display function of the backup command's output.
 */

int 
BackupFiles(file_list, nfiles, devicename)
	char **file_list;
	int nfiles;
	char *devicename;
{
	char    *argv[5],
		*cp,
		*executable_name;
	int     i, 
		n,
		pid,
		ret,
		wait_stat;
	struct stat sb;
	privvec_t s;
	int	pfd[2];
	void	(*old_sighup)(), (*old_sigint)(), (*old_sigquit)();
	
	/* Check existence of log directory */

	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);
	
	/* Check read/search permission on Audit log directory */    
	if (stat(AUDIT_LOGDIR, &sb) < 0 || !S_ISDIR(sb.st_mode)) {
		ErrorMessageOpen(1172, msg, 30, NULL);
		/* AUDIT permission failure on audit log directory */
		audit_no_resource(AUDIT_LOGDIR, OT_DIRECTORY,
					msg[42], ET_SYS_ADMIN);
		seteffprivs(s, NULL);
		return 1;
	}
	
	/* Prepare arguments for the backup program */
	cp = strrchr(BACKUP_PROGRAM, '/');
	if (cp)
		cp++;
	else
		cp = BACKUP_PROGRAM;
	executable_name = cp;
	
	n = 0;
	argv[n++] = executable_name;
	argv[n++] = BACKUP_ARGUMENTS;

	/* when using mltape (SEC_ARCH), device is argument to -O,
	 * when using cpio  (!SEC_ARCH), device is standard out
	 * in both cases, filenames are send to standard input of
	 * backup program.
	 */

#if SEC_ARCH
	argv[n++] = devicename;
#endif
	argv[n++] = NULL;

	/* Fork and run the backup program, piping filenames to the program */

	if (pipe(pfd) < 0) {
#ifdef NEW_ERROR
		ErrorMessageOpen(1172, msg, 30, NULL);
		/* AUDIT permission failure on audit log directory */
		audit_no_resource(BACKUP_PROGRAM, OT_PIPE,
					"Cannot allocate", ET_SYS_ADMIN);
#endif
		seteffprivs(s, NULL);
		return 1;
	}
		
	old_sighup = signal(SIGHUP, SIG_IGN);
	old_sigint = signal(SIGINT, SIG_IGN);
	old_sigquit = signal(SIGQUIT, SIG_IGN);
	switch (pid = fork())  {
		case    -1: /* error - can't fork sub-process */
			signal(SIGHUP, old_sighup);
			signal(SIGINT, old_sigint);
			signal(SIGQUIT, old_sigquit);
			ret = -1;
			break;
			
		case    0:  /* child */
		{
			int fd;
			/* Reset signals */
			signal(SIGHUP, SIG_DFL);
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);

#if !SEC_ARCH
			/* if !SEC_ARCH, open device as standard output */
			close(1);
			fd = open(devicename, O_WRONLY);
			if (fd < 0)
				exit(0x7f);
#endif

			/* open pipe as standard input */
			close(0);
			dup(pfd[0]);
			close(pfd[0]);
			close(pfd[1]);

			/* Run backup program */
			exit(DisplayCommand(BACKUP_PROGRAM, argv));
		}
		default:
			/* send the filenames to the pipe */

			close(pfd[0]);
			for (i = 0; i < nfiles; i++) {
				int len = strlen(file_list[i]);
				char *fn = Malloc(len+2);

				if (fn == NULL)
					break;
				strcpy(fn, file_list[i]);
				strcat(fn, "\n");
				len++;
				ret = write(pfd[1], fn, len);
				free(fn);
				if (ret != len)
					break;
			}
			close(pfd[1]);

			wait(&wait_stat);
			/* Put signals back */
			signal(SIGHUP, old_sighup);
			signal(SIGINT, old_sigint);
			signal(SIGQUIT, old_sigquit);
			if (! (wait_stat & 0xFF)) {
				/* terminated due to exit() */
				ret = (wait_stat >> 8) & 0xFF; /* exit status */
				if (ret == 0x7f)
					ret = -1;
			}
			else 
				/* terminated by signal */
				ret = -1;
			break;
	}

	/* 'ret' has result code here from exec */
	if (ret) {
		/* Terminal shell program failed */
		ErrorMessageOpen(1182, msg, 38, NULL);
		audit_no_resource(BACKUP_PROGRAM, OT_PROCESS,
		  msg[48], ET_SYS_ADMIN);
		return 1;
	}

	/* AUDIT run of backup program */
	sa_audit_audit(ES_AUD_ARCHIVE, msg[49]);
}

int 
DeleteFiles(file_list, nfiles)
	char **file_list;
	int nfiles;
{
	privvec_t s;
	int i;

	/* raise privileges to delete audit files */

	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);

	/* drop suspendaudit to audit file removal */

	disablepriv(SEC_SUSPEND_AUDIT);

	/* delete all files */

	for (i = 0; i < nfiles; i++)
		unlink(file_list[i]);
		
	seteffprivs(s, NULL);
	return 0;
}

static int 
CheckDeviceWrite(device)
	char    *device;
{
	struct stat sb;
	int     ret;

	ret = stat(device, &sb);
	if (ret < 0)  {
		/* Device does not exist. */
		ErrorMessageOpen(1190, msg, 55, NULL);
		return 1;
	}
	if ((sb.st_mode & S_IFMT) != S_IFCHR) {
		/* Device is not a character special device. */
		ErrorMessageOpen(1191, msg, 58, NULL);
		return 1;
	}
	if (eaccess(device, 2) < 0) {
		/* Device is not accessible. */
		ErrorMessageOpen(1192, msg, 61, NULL);
		return 1;
	}
	return 0;
}

/*
 * Retrieve all files related to the sessions specified in the session list.
 * The select_session argument is a character array whose non-zero elements
 * identify the sessions to retrieve.
 * The routine builds a linked list of files, and then collects filenames
 * into a table.
 */

#define add_to_list(fn) \
	ap = (struct a *) malloc(sizeof(struct a));	\
	if (ap == (struct a *) 0)			\
		MemoryError();				\
							\
	ap->filename = fn;				\
	if (first == NULL)				\
		first = ap;				\
	else						\
		last->next = ap;			\
	last = ap;					\
	ap->next = NULL;				\
	nfile_list++;

GetSessionFiles(aufill, file_listp, nfilep, select_session)
	struct ls_fillin *aufill;
	char ***file_listp;
	int *nfilep;
	char *select_session;
{
	struct a {
		char *filename;
		struct a *next;
	} *ap, *first = NULL, *last;
	char *filename;
	int i, j;
	privvec_t s;
	char comp_file[AUDIT_PATHSIZE];
	struct stat sb;
	char **file_list;
	int nfile_list = 0;
	struct  log_header  log;
	FILE *fp;

	/* raise privileges necessary to read log file */

	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);

	for (i = 0; i < aufill->nsessions; i++) {
		if (select_session[i]) {
			
			filename = Malloc(sizeof(AUDIT_LOGDIR) +
					      sizeof(AUDIT_LOG_FILENAME) - 1);
			if (filename == NULL)
				MemoryError();

			/* Build audit session name */
			sprintf(filename, "%s%.*s%0*ld", AUDIT_LOGDIR,
				sizeof(AUDIT_LOG_FILENAME) - AUDIT_DIGITS - 1,
				AUDIT_LOG_FILENAME, AUDIT_DIGITS,
				atol(aufill->sessions[i]));

			/* Check audit file and get associated filenames */
			fp = fopen(filename, "r");
			if (fp == (FILE *) 0) {
				ErrorMessageOpen(1120, msg, 18, NULL); 
				audit_no_resource(ap->filename, OT_REGULAR,
				  msg[44], ET_SYS_ADMIN);
				select_session[i] = (char) -1;
				free(filename);
				continue;
			}
	
			/* Read in magic collection file number */
			if (fread(&log, sizeof(log), 1, fp) != 1) {
				ErrorMessageOpen(1125, msg, 20, NULL); 
				fclose(fp);
				audit_no_resource(filename, OT_REGULAR,
				  msg[45], ET_SYS_ADMIN);
				select_session[i] = (char) -1;
				free(filename);
				continue;
			}
	
			/* Test that this is indeed an audit collection file */
			if (strncmp(log.id, AUDIT_LOGID, sizeof(log.id))) {
				ErrorMessageOpen(1130, msg, 20, NULL);
				fclose(fp);
				/* AUDIT inconsistent audit log file */
				audit_no_resource(filename, OT_REGULAR,
				  msg[45], ET_SYS_ADMIN);
				select_session[i] = (char) -1;
				free(filename);
				continue;
			}
	
			add_to_list(filename);
		 
			/* Now fetch associated file for backing up */
			for (j = 0; j < log.comp_files; j++) {
				if (fread(comp_file, sizeof(comp_file), 1, fp)
						!= 1) {
					/* AUDIT inconsistent audit log file */
					ErrorMessageOpen(1145, msg, 20, NULL);
					audit_no_resource(filename, OT_REGULAR,
					  msg[45], ET_SYS_ADMIN);
					fclose(fp);
					break;
				}

				if (stat(comp_file, &sb) < 0) {
					/* AUDIT missing collection file */
					audit_no_resource(comp_file,
					  OT_REGULAR, msg[46], ET_SYS_ADMIN);
					continue;
				}

				filename = Malloc(strlen(comp_file) + 1);
				if (filename == NULL)
					MemoryError();

				strcpy(filename, comp_file);
				add_to_list(filename);
			}
			fclose(fp);
		}
	}

	/* Reduce privilege -- remainder of routine does not require it */

	seteffprivs(s, NULL);

	/* Create a table with the filenames attached */

	file_list = (char **) calloc(sizeof(char *), nfile_list);
	if (file_list == (char **) 0)
		MemoryError();

	for (i = 0; i < nfile_list; i++) {
		file_list[i] = first->filename;
		ap = first;
		first = first->next;
		free(ap);
	}

	/* assign the return parameters */

	*file_listp = file_list;
	*nfilep = nfile_list;
	return 0;
}

/*
 * Clean up the data structures for the file list
 */

void
FreeSessionFiles(file_list, nfiles)
	char **file_list;
	int nfiles;
{
	int i;

	for (i = 0; i < nfiles; i++)
		if (file_list[i] != NULL)
			free(file_list[i]);
	free((char *) file_list);
}

#endif /* SEC_BASE */
