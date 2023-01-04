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
static char	*sccsid = "@(#)$RCSfile: XAudit.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:16 $";
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
		XAudit.c
	
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		provide X user interface for audit subsystem administration
		
*/
		
#include "XAudit.h"                            

void StartAuditFunctionalUnits() {
    AuditDirListStart();
    AuditEvMaskStart();
    AuditListSessionsStart();
    AuditParametersStart();
#ifdef SEC_MAC
    AudColnslStart();
    AudColxslStart();
    AudSelnssStart();
    AudSelxssStart();
    AudSelnosStart();
    AudSelxosStart();
#endif /* SEC_MAC */
    AuditStatsStart();
    AuditUsersGroupsStart();
    BackupDeleteStart();
    ConfirmationStart();
    DirectoryListStart();
    ErrorMessageStart();
    FileDisplayStart();
    HelpDisplayStart();
    MaintenanceStart();
    ModifyEventsStart();
    ParameterFileListStart();
    ParametersStart();
    ReduceGenerateStart();
    RestoreStart();
    SelectionFileStart();
    ShowReportsStart();  
    UsersGroupsStart();  
}

void StopAuditFunctionalUnits() {

    /* Stop audit X pieces */
    AuditDirListStop();
    AuditEvMaskStop();
    AuditListSessionsStop();
    AuditParametersStop();
#ifdef SEC_MAC
    AudColnslStop();
    AudColxslStop();
    AudSelnssStop();
    AudSelxssStop();
    AudSelnosStop();
    AudSelxosStop();
#endif /* SEC_MAC */
    AuditStatsStop();
    AuditUsersGroupsStop();
    BackupDeleteStop();
    DirectoryListStop();
    MaintenanceStop();
    ModifyEventsStop();
    ParametersStop();
    ReduceGenerateStop();
    RestoreStop();
    SelectionFileStop();
    ShowReportsStop();
    UsersGroupsStop();
}
#endif /* SEC_BASE */
