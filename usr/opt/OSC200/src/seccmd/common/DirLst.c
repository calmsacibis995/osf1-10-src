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
static char	*sccsid = "@(#)$RCSfile: DirLst.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:59:27 $";
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


/*
	filename:
		DirLst.c
		
	function:
		provide routines to manage the audit directory list
		
	entry points:
		AuditDirListStart()
			Loads message file
		
		AuditDirListCheck() 
			scan dir list for valid list of directories
			returns 0 on success
			returns 1 on error after generating an error message
			
		AuditDirListGet()
			get directory list from system
			returns 0 on success
			returns 1 on error after generating an error message
			
		AuditDirListPut()
			return directory list to the system
			returns 0 on success
			returns 1 on error after generating an error message

		link_new_parms_file()
		new_audit_parms_file()
		open_aud_parms()    
		old_audit_parms_file()
		unlink_new_parms_file()
*/

#include <sys/secdefines.h>

#if SEC_BASE

#include "gl_defs.h"
#include "IfAudit.h"

static char 
	**msg_adirlst,
	 *msg_adirlst_text;
	
void 
AuditDirListStart() {
	LoadMessage("msg_isso_audit_adirlst", &msg_adirlst, &msg_adirlst_text);
}

void 
AuditDirListStop() {
}

int 
AuditDirListCheck(aufill)
	struct audir_fillin *aufill;
{
	int     i,
		j;
	register char *dir;
	struct stat sb;
	int	empty;

	/* Root directory must exist */
	dir = aufill->root_dir;
	if (! *dir) {
		ErrorMessageOpen(2005, msg_adirlst, 37, NULL);
		return 1;
	}

	/* Add trailing slash to the root directory if needed */
	if (dir[strlen(dir) - 1] != '/')
		strcat(dir, "/");

	empty = TRUE;
	/* Add trailing slash if needed */
	for (i = 0; i < aufill->ndirs; i++) {
		dir = aufill->dirs[i];
		if (! *dir)
			continue;
		empty = FALSE;
		if (dir[strlen(dir) - 1] != '/')
			strcat(dir, "/");
	}
	
	/* Directory must be specified */
	if ( empty ) { 
		ErrorMessageOpen(2035, msg_adirlst, 32, dir);
		return 1;
	}    
	
	dir = aufill->root_dir;

	/* Check single user directory first */
	/* Must be a directory, read, write, exec permissions */
	if (stat(dir, &sb) < 0) {
		ErrorMessageOpen(2010, msg_adirlst, 0, dir);
		return 1;
	}
	else if ((sb.st_mode & S_IFMT) != S_IFDIR) {
		ErrorMessageOpen(2020, msg_adirlst, 3, dir);
		return 1;
	}
	else if (
			 (sb.st_mode & ((S_IREAD | S_IWRITE | S_IEXEC) >> 6))
	) {
		ErrorMessageOpen(2030, msg_adirlst, 6, dir);
		return 1;
	}
		
	/* Confirm that name is a valid directory with appropriate permissions */
	for (i = 0; i < aufill->ndirs; i++) {
		dir = aufill->dirs[i];
		if (! *dir)
			continue;
		if (stat(dir, &sb) < 0) {
			ErrorMessageOpen(2010, msg_adirlst, 0, dir);
			return 1;
		}
		else if ((sb.st_mode & S_IFMT) != S_IFDIR) {
			ErrorMessageOpen(2020, msg_adirlst, 3, dir);
			return 1;
		}
		else if ((sb.st_mode & ((S_IREAD | S_IWRITE | S_IEXEC) >> 6))) {
			ErrorMessageOpen(2030, msg_adirlst, 6, dir);
			return 1;
		}
		
		/* Check for duplicate entries */
		for (j = 0; j < aufill->ndirs; j++) {
			if (j == i || ! *(aufill->dirs[j]))
				continue;
			if (! strcmp(dir, aufill->dirs[j])) {
				ErrorMessageOpen(2032, msg_adirlst, 34, dir);
				return 1;
			}
		}
	}
	
	return 0;
}

int 
AuditDirListGet(aufill)
	struct audir_fillin *aufill;
{
	int     c,
		i,
		j;
			
	char    buf[512],
		*cp;
	FILE    *fp;

	fp = open_aud_parms(&aufill->au);
	if (! fp)
		return 1;

	aufill->root_dir = (char *) strdup (aufill->au.root_dir);
#ifdef DEBUG
	printf ("Get audit data Root dir %s\n", aufill->au.root_dir);
	printf ("Get audit data Root dir %s\n", aufill->root_dir);
	printf ("Get audit data Root len %d\n", strlen(aufill->root_dir) );
#endif

	fseek(fp, aufill->au.dir_offset, 0);
	aufill->dirs = alloc_cw_table(aufill->au.dir_count, AUDIRWIDTH + 1);
	if (! aufill->dirs)
		MemoryError();
	
	for (i = 0; i < aufill->au.dir_count; i++) {
		cp = aufill->dirs[i];
		j = 0;
		do {
			c = getc(fp);
			cp[j++] = c;
		} while (c != EOF && c != '\0' && j <= AUDIRWIDTH);
		if (c == EOF || j == AUDIRWIDTH + 1) {
			if (c == EOF)
				strcpy(buf, 
			"Unexpected end of file in audit parameter file");
			else {
				aufill->dirs[i][AUDIRWIDTH] = '\0';
				sprintf(buf,
				    "Error on read of directory list \'%s\'.",
				    aufill->dirs[i]);
			}
			ErrorMessageOpen(2040, msg_adirlst, 9, NULL);
			free_cw_table(aufill->dirs);
			aufill->dirs = NULL;
			fclose(fp);
			
			/*AUDIT inconsistency of audit parameter file */
			audit_security_failure (OT_SUBSYS, AUDIT_PARMSFILE, 
				"Inconsistent file format", ET_SYS_ADMIN);
			return 1;
		}
	}
	aufill->ndirs = aufill->au.dir_count;
	fclose(fp);
	return 0;
}

int 
AuditDirListPut(aufill)
	struct audir_fillin *aufill;
{
	FILE    *fp,
		*ofp;
	int     c,
		count,
		i,
		j,
		rc;
	struct audit_init oauinit;

	/* Attempt to open old audit parms file */
	ofp = old_audit_parms_file();
	if (! ofp)
		return 1;
		
	/* Attempt to open new audit parms file */
	fp = new_audit_parms_file();
	if (! fp) {
		fclose(ofp);
		return 1;
	}

	/* Get rid of empty lines */
	for (i = 0; i < aufill->ndirs; i++) {
		if (aufill->dirs[i][0]) 
			continue;
		for( j = i + 1; j < aufill->ndirs && ! aufill->dirs[j][0]; j++ )
			;
		if (j == aufill->ndirs)
			continue;
		strcpy(aufill->dirs[i], aufill->dirs[j]);
		aufill->dirs[j][0] = '\0';
	}
	
	/* Build the audit directory name lengths */
	count = 0;
	aufill->au.dir_count = 0;
	for (i = 0; i < aufill->ndirs; i++) {
		if (! aufill->dirs[i][0])
			break;
		count += strlen(aufill->dirs[i]) + 1;
		aufill->au.dir_count++;
	}
	aufill->au.dir_count = i;
	aufill->au.uid_offset = sizeof(struct audit_init);
	aufill->au.gid_offset = aufill->au.uid_offset +
			            aufill->au.uid_count * sizeof(uid_t);
	aufill->au.dir_offset = aufill->au.gid_offset +
			            aufill->au.gid_count * sizeof(gid_t);
	aufill->au.buf_length = aufill->au.dir_offset + count;

	strcpy (aufill->au.root_dir, aufill->root_dir);

	rc = fwrite(&aufill->au, AUDITINIT_SIZE, 1, fp);
	if (rc != 1)  {
		ErrorMessageOpen(2050, msg_adirlst, 11, NULL);
		fclose(fp);
		fclose(ofp);
		unlink_new_parms_file();
		/* AUDIT failure to write audit header to new parameter file */
		audit_no_resource("New audit parameter file", OT_SUBSYS,
		          "Write of init record failed", ET_SYS_ADMIN);
		return 1;
	}
	
	/* Write uid's and gids from old file */
	fread(&oauinit, sizeof (oauinit), 1, ofp);
	fseek(ofp, oauinit.uid_offset, 0);
	for (i = 0; i < oauinit.uid_count * sizeof (uid_t); i++) {
		c = getc(ofp);
		if (c == EOF || putc(c, fp) == EOF)
			goto bad;
	}
	fseek(ofp, oauinit.gid_offset, 0);
	for (i = 0; i < oauinit.gid_count * sizeof (gid_t); i++) {
		c = getc(ofp);
		if (c == EOF || putc(c, fp) == EOF) 
			goto bad;
	}
	for (i = 0; i < aufill->au.dir_count; i++) {
		rc = fwrite(aufill->dirs[i], strlen (aufill->dirs[i]) + 1, 
			1, fp);
		if (rc != 1)
			goto bad;
	}
	fclose(fp);
	fclose(ofp);
	link_new_parms_file();
	/* AUDIT successful change of audit directory list */
	sa_audit_audit(ES_AUD_MODIFY,
		 "Successful change of audit directory list");
	return 0;
	
bad:
	ErrorMessageOpen(2060, msg_adirlst, 14, NULL);
		
	fclose(fp);
	fclose(ofp);
	unlink_new_parms_file();
	/* AUDIT failure to write new parameter file */
	audit_no_resource("New audit parameter file", OT_SUBSYS,
	          "Write failure", ET_SYS_ADMIN);
	return 1;
}

void 
link_new_parms_file()
{
	char    nparmsfile[sizeof(AUDIT_PARMSFILE) + 2 ];
	int     rc;

	sprintf(nparmsfile, "%s-t", AUDIT_PARMSFILE);
	rc = unlink(AUDIT_PARMSFILE);
	if (rc < 0) {
		ErrorMessageOpen(2070, msg_adirlst, 17, NULL);
		unlink(nparmsfile);
		/* AUDIT failure to unlink old parameter file */
		audit_security_failure(OT_SUBSYS, 
		     "Unlink old audit parameter file to make way for new one",
			"Unlink failed", ET_SYS_ADMIN);
		return;
	}
	rename(nparmsfile, AUDIT_PARMSFILE);
	return;
}

FILE 
*new_audit_parms_file()
{
	char    nparmsfile[ sizeof(AUDIT_PARMSFILE) + 2 ];
	FILE    *fp;
	struct stat sb;
	int	ret;

	sprintf(nparmsfile, "%s-t", AUDIT_PARMSFILE);
	/* We want this file to be created securely */
	if (access (nparmsfile, 0) == -1) {
		ret = create_file_securely (nparmsfile, AUTH_VERBOSE, 
			"temporary audit parameters file");
		if (ret != CFS_GOOD_RETURN) {
			ErrorMessageOpen(2080, msg_adirlst, 41, NULL);
			return NULL;
		}
	}

	fp = fopen(nparmsfile, "w");
	if (! fp) {
		/* Post visual error message */
		ErrorMessageOpen(2080, msg_adirlst, 20, NULL);
				  
		/* AUDIT failure to open new audit parameter file */
		audit_no_resource(AUDIT_PARMSFILE, OT_SUBSYS, 
			"Cannot open file for writing", ET_SYS_ADMIN);
	}
	
	return fp;
}

FILE 
*old_audit_parms_file()
{
	FILE    *fp;

	fp = fopen(AUDIT_PARMSFILE, "r");
	if (! fp) {
		/* Post visual error message */
		ErrorMessageOpen(2090, msg_adirlst, 23, NULL);
				 
		/* AUDIT failure to access AUDIT_PARMSFILE */
		audit_security_failure(OT_SUBSYS, AUDIT_PARMSFILE,
			"Failure to open for reading", ET_SYS_ADMIN);
	}
	return fp;
}

/*
 * Update the new audit_parms file with a new init structure.
 * Doesn't require rewriting any of the variable length data, 
 * so it's better to update it in place.
 * Returns 0 on success, 1 on failure.
 */

int
update_audit_parms(init)
struct audit_init *init;
{
	FILE *fp;
	int rc;
	privvec_t s;

	/* open the parameter file after raising appropriate privileges. */

	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);
	fp = fopen(AUDIT_PARMSFILE, "r+");
	seteffprivs(s, NULL);

	if (fp == (FILE *) 0) {
		/* Post visual error message */
		ErrorMessageOpen(2090, msg_adirlst, 23, NULL);
				 
		/* AUDIT failure to access AUDIT_PARMSFILE */
		audit_security_failure(OT_SUBSYS, AUDIT_PARMSFILE,
			"Failure to open for reading", ET_SYS_ADMIN);
		return 1;
	}

	/* write the init structure at the beginning of the file */

	fseek(fp, 0L, 0);
	rc = fwrite(init, AUDITINIT_SIZE, 1, fp);
	fclose(fp);

	if (rc != 1)  {
		ErrorMessageOpen(2050, msg_adirlst, 11, NULL);
		/* AUDIT failure to write audit header to new parameter file */
		audit_no_resource("Audit parameter file", OT_SUBSYS,
		          "Write of init record failed", ET_SYS_ADMIN);
		return 1;
	}
	fclose(fp);
	return 0;
}

/* 
	open_aud_parms();
	
	open the audit parameters file and read in the audit_init structure.
	
	Returns a read-only file pointer to the file, or NULL if can't open
	the file or read the audit_init record.
*/

FILE 
*open_aud_parms(auinitp)
	struct audit_init *auinitp;
{
	int     rc;
	FILE    *fp;

	fp = fopen(AUDIT_PARMSFILE, "r");
	if (! fp) {
		ErrorMessageOpen(2015, msg_adirlst, 26, NULL);
		/* AUDIT failure to open AUDIT_PARMSFILE */
		audit_security_failure(OT_SUBSYS, AUDIT_PARMSFILE,
			"Failure to open for reading", ET_SYS_ADMIN);
		return NULL;
	}
	
	rc = fread(auinitp, sizeof(*auinitp), 1, fp);

	if (rc != 1) {
		ErrorMessageOpen(2025, msg_adirlst, 29, NULL);
		fclose(fp);
		/* AUDIT sanity failure on AUDIT_PARMSFILE */
		audit_security_failure (OT_SUBSYS, AUDIT_PARMSFILE,
			"Inconsistent file format", ET_SYS_ADMIN);
		return NULL;
	}
	return fp;
}

void 
unlink_new_parms_file()
{
	char    nparmsfile[ sizeof (AUDIT_PARMSFILE) + 2 ];

	sprintf(nparmsfile, "%s-t", AUDIT_PARMSFILE);
	unlink(nparmsfile);
	return;
}
#endif /* SEC_BASE */
