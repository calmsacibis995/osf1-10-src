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
static char	*sccsid = "@(#)$RCSfile: PFiles.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:38 $";
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

#if SEC_BASE

/*
	filename:
		aud_pfiles.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		get list of reduction parameter files
		
	entry points:
		void ParameterFileListFree(struct lpfile_fillin  *pfill)
		int  ParameterFileListGet(struct lpfile_fillin  *pfill)
*/

#include "XAudit.h"

static char 
	**msg_apfiles,
	 *msg_apfiles_text;
	
void 
ParameterFileListStart() {
	LoadMessage("msg_isso_audit_apfiles", &msg_apfiles, &msg_apfiles_text);
}

void 
ParameterFileListStop() {
}

void 
ParameterFileListFree(pfill)
	struct lpfile_fillin *pfill;
{
	if (pfill->files) {
		free_cw_table(pfill->files);
		pfill->files = NULL;
	}
}

int 
ParameterFileListGet(pfill)
	struct lpfile_fillin *pfill;
{
	char	buf[80],
		*cp;
	int	count,
		i;
	DIR     *fp;
	struct dirent *d;

	if (eaccess(AUDIT_REDUCE_PARM_DIR, 5) < 0) {
		ErrorMessageOpen(2210, msg_apfiles, 0, AUDIT_REDUCE_PARM_DIR);
		return 1;
	}

	fp = opendir(AUDIT_REDUCE_PARM_DIR);
	if (! fp) {
		ErrorMessageOpen(2220, msg_apfiles, 3, NULL);
		return 1;
	}
	count = 0;
	while ((d = readdir(fp)) != (struct dirent *) 0) {
		cp = d->d_name;
#ifdef SCO
		/* XXXXX The readdir function of SCO is broken */
		cp -= 2;
#endif
		if (! strcmp(cp, ".")
		||  ! strcmp (cp, "..") 
		||  ! d->d_fileno 
		)
			continue;
		count++;
	}

	pfill->files = (char **) alloc_cw_table(count, ENTSIZ + 1);
	if (! pfill->files)
		MemoryError();
		/* Program exits */
	
	pfill->nfiles = count;
	if (pfill->nfiles) {
#if defined(OSF)
	/* The rewinddir function is broken in OSF */
		closedir (fp);
		fp = opendir(AUDIT_REDUCE_PARM_DIR);
		if (! fp) {
			ErrorMessageOpen(2220, msg_apfiles, 3, NULL);
			return 1;
		}
#else
		rewinddir(fp);
#endif /* OSF */
		count = 0;
		while (d = readdir(fp)) {
		cp = d->d_name;
#ifdef SCO
		/* XXXXX The readdir function of SCO is broken */
		cp -= 2;
#endif
			if (! strcmp(cp, ".") 
			||  ! strcmp(cp, "..")
			||  ! d->d_fileno
			)
				continue;
			strncpy(pfill->files[count], d->d_name, ENTSIZ);
			count++;
		}
	}
	closedir(fp);
	
	/* Alpha sort entries */
	sort_cw_table(pfill->files, ENTSIZ + 1, count);
	return 0;
}  
#endif /* SEC_BASE */
