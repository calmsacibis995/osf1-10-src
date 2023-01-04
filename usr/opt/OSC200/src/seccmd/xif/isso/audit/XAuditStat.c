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
static char	*sccsid = "@(#)$RCSfile: XAuditStat.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:19 $";
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
        XAuditStat.c
       
    copyright:
        Copyright (c) 1989-1990 SecureWare Inc.
        ALL RIGHTS RESERVED
    
    function:
        X windows front end for Audit System Statistics options
        
    entry points:
        AuditStatsStart()
        AuditStatsOpen()
        AuditStatsClose()
        AuditStatsStop()
*/

#include "XAudit.h"
#include "sys/secioctl.h"

static char
        **msg_auditstats,
        *msg_auditstats_text;
        
static int
        RunAuditStatsCommand();

void AuditStatsStart()
{
    /* Nothing needs to be done here */
}

void AuditStatsOpen()                        
{
    char        *fname;
                
    /* Desensitize and get working widget going */
    XtSetSensitive(main_menubar, False);
    WorkingOpen(main_shell);
    
    /* Load in message text if not already in */
    if (! msg_auditstats)
        LoadMessage("msg_isso_audit_stats", &msg_auditstats, &msg_auditstats_text);
    
    /* Generate a temporary file name */
    fname = tmpnam(NULL);
    if (! fname)
	MemoryError();
    
    WorkingClose();
    
    /* Get auditstats and put into temp file */
    if (RunAuditStatsCommand(fname)) 
        FileDisplayOpen(fname, msg_auditstats[24], msg_auditstats, 10, 
		NULL, 2910);
    /* Erase the temporary file */
    unlink(fname);

    /* Close up shop */
    XtSetSensitive(main_menubar, True);
}    
    
void AuditStatsClose() {
    /* Nothing needs to be done here */
}

void AuditStatsStop() {
    /* nothing needs to be done here */
}  

/* Get audit stats by talking directly to the daemon */
static int
RunAuditStatsCommand(fname)
    char        *fname;
{
	FILE *fd;
	int audit_fd;
	int ret;
	struct audit_stats audit_stats;

	/* Open the audit device */
	ret = OpenAuditDevice (&audit_fd);
	if (! ret) 
		return FALSE;

	/* Open the temporary file */
	fd = fopen (fname, "w");
	if (! fd) {
		ErrorMessageOpen (0, msg_auditstats, 26, NULL);
		close (audit_fd);
		return FALSE;
	}

	/* Signal to disable auditing. Trap errors here */
	if (ioctl (audit_fd, AUDIOC_STATS, &audit_stats, 0) == -1) {
		if (errno == EINVAL)
			ErrorMessageOpen (0, msg_auditstats, 28, NULL);
		else
			ErrorMessageOpen (0, msg_auditstats, 30, NULL);
		close (audit_fd);
		fclose (fd);
		return FALSE;
	}

	/* Close the audit daemon file */
	close (audit_fd);

	/* output the statistics to the temp file */
		/*	*** Audit Subsystem Statistics ***    */
	fprintf (fd,"\t\t%s\n\n", msg_auditstats[32]);
		/*	   Current Audit Session		*/
	fprintf (fd,"\t\t%s-%d\n\n", msg_auditstats[33], audit_stats.session);
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[34], 
		audit_stats.total_bytes);
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[35],
		audit_stats.total_recs);
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[36],
		audit_stats.appl_recs);
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[37],
		audit_stats.syscall_recs);
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[38],
		audit_stats.kernel_recs);
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[39],
		audit_stats.syscall_norecs);
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[40],
		((audit_stats.syscall_recs * 100) /
		 (audit_stats.syscall_recs + audit_stats.syscall_norecs)));
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[41],
		audit_stats.read_count);
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[42],
		audit_stats.write_count);
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[43],
		audit_stats.highest_offset);
	fprintf (fd,"\t\t%s:\t%d\n", msg_auditstats[44],
		audit_stats.buffer_sleep);

	/* finished with stats */
	fclose (fd);
	return TRUE;
}
#endif /* SEC_BASE */
