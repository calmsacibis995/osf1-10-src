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
static char *rcsid = "@(#)$RCSfile: system_load_structures.c,v $ $Revision: 1.1.2.7 $ (DEC) $Date: 1993/11/10 21:40:32 $";
#endif
/*
**++
**  FACILITY:  Ultrix Common Agent
**
**  Copyright (c) 1993  DIGITAL EQUIPMENT CORP
**
**  MODULE DESCRIPTION:
**
**      This module is part of the Managed Object Module (MOM)
**	for File System Management.
**
**	It provides the routines to retrieve file system information
**	from the system using the getmnt() system call, copy it to
**      an instance structure, and insert the structure into the mom.
**
**  AUTHORS:
**
**      Muhammad I. Ashraf
**
**      Adapted from Mike Densmore's test_snmp code used for regression.
**      This code was created for use with the C code produced
**      by the Ultrix MOM Generator
**
**  CREATION DATE:   25-Jan-1993
**
**  MODIFICATION HISTORY:
**
**
**--
*/

#include "moss.h"
#include "common.h"
#include "extern_common.h"
#include "iso_defs.h"
#include "inet_mom_specific.h"


/**
 ** The maximum buffer size 
 **/
#define MOM_C_MAX_BUFLEN  1024           

unsigned int 
    sys_obj_id[ULTRIX_LENGTH] = {ULTRIX_SEQ} ;

object_id
    sys_obj_id_oid = { ULTRIX_LENGTH, sys_obj_id };
 
octet_string sysid_octet;

int sysStartTime;   /* timestamp of when the Internet MOM (Agent) started */

/**
 ** FUNCTION:
 **  
 **
 ** PURPOSE:
 **  
 **  
 **/

man_status  load_system_structures()
{
    system_DEF *system_instance = NULL;
    man_status status;
    uid *temp_uid;


    system_instance =(system_DEF *)malloc(sizeof(system_DEF));
    if (system_instance == NULL)
	return MAN_C_INSUFFICIENT_RESOURCES;
      
    /* set up the instance name */

    system_instance->instance_name = (char *)NULL;
    system_instance->instance_name_length = 0; 
 
    system_instance->sysObjectID = &sys_obj_id_oid;

    /* get the system information from kinfo */
    get_system_info(system_instance);

    /* Amount of time since the last init of the system's Net Mgmt portion */
    system_instance->sysUpTime = 0 ;

    /* This can have the value of 76. As any DEC/OSF1
     * system has the "potential" to be a router.
     * i.e. system supports layers 3, 4 and 7 : 76.
     */
    system_instance->sysServices = 76;
	
    printf("system descr: %s\n",system_instance->sysDescr);
    printf("system contact: %s\n",system_instance->sysContact);
    printf("system location: %s\n",system_instance->sysLocation);
    printf("system name: %s\n",system_instance->sysName);

    memset( (void *)&new_system_header->instance_uid, '\0', sizeof(uid) ) ;
    system_instance->object_instance = (avl *)NULL ;

    /* insert new struct at end of list */
    status = system_add_new_instance( new_system_header, system_instance );
    if (ERROR_CONDITION(status)) return status;
 
    /*
     *  Set up the octet containing the system object identifier.
     */

    status = moss_oid_to_octet( &sys_obj_id_oid, &sysid_octet ) ;
    if ( status != MAN_C_SUCCESS )
        return( MAN_C_PROCESSING_FAILURE ) ;

    return MAN_C_SUCCESS;
}


get_system_info(system_instance)
system_DEF *system_instance ;
{
    FILE *fileid;               /* The config file */
    char buff[MOM_C_MAX_BUFLEN];/* buffer to read data into */
    int len;
    int line;
    int sDlth;
    struct strioctl str;
    static system_blk buf;
	

    bzero(&buf, sizeof(buf));
    str.ic_cmd = KINFO_GET_SYSTEM;
    str.ic_timout = 15;
    str.ic_len = sizeof(buf);
    str.ic_dp = (char *)&buf;

    if (ioctl(inetfd, I_STR, &str) < 0) 
    {
         perror(" ioctl");
         return(MAN_C_PROCESSING_FAILURE);
    }

    /* Get a timestamp of when the agent started (need this for later) */
    (void)time(&sysStartTime);

    sDlth = (strlen(buf.hostname) + strlen(buf.cputype) + 
             strlen(buf.kernel_version) + strlen("TCP/IP") + 67);
    system_instance->sysDescr = (char *)malloc(sDlth);
    sprintf(system_instance->sysDescr, "\n\tHostname:\n\t %s\n\tCPU:\n\t %s\n\tOperating System:\n\t %s\tNetwork Software:\n\t %s\n",buf.hostname, buf.cputype, buf.kernel_version, "TCP/IP");
    system_instance->sysDescr_len = sDlth;

    system_instance->sysName = (char *)malloc(strlen(buf.hostname));
    strncpy(system_instance->sysName,buf.hostname,strlen(buf.hostname));
    system_instance->sysName_len = strlen(buf.hostname);

    /* Read system information from the config file */
    fileid = fopen(CONFIG_FILE, "r");

    if (fileid == NULL) 
    {
        system_instance->sysContact = "Unknown";
	system_instance->sysContact_len = 7;
        system_instance->sysLocation = "Unknown";
	system_instance->sysLocation_len = 7;
	return(MAN_C_SUCCESS);
    }

    line = 1;
    bzero(buff, sizeof(buff));

    while (fgets (buff, sizeof (buff), fileid) != NULL)
    {
        if ( (buff[0] == '\n') || (buff[0] == '#') )
            continue;

        switch (line)
        {
            case 1: /* It is the system location */
                len = strlen(buff) - 1;  /* skip the carriage-ret/linefeed */
                system_instance->sysLocation = (char *)malloc(len);
                strncpy(system_instance->sysLocation, buff, len);
                system_instance->sysLocation_len = len;
                bzero(buff, sizeof(buff));
                line++;
                break;

            case 2: /* Its the system contact */
                len = strlen(buff) - 1;  /* skip the carriage-ret/linefeed */
                system_instance->sysContact = (char *)malloc(len);
                strncpy(system_instance->sysContact, buff, len);
                system_instance->sysContact_len = len;
                bzero(buff, sizeof(buff));
                line++;
                break;
        }
    }

    fclose (fileid);

    /*
     * To see how we did check the line number and accordingly fill
     * in the default values.
     */

    switch (line) 
    {
        case 1:    /* Fill in all default values */
            system_instance->sysContact = "Unknown";
	    system_instance->sysContact_len = 7;
            system_instance->sysLocation = "Unknown";
            system_instance->sysLocation_len = 7;
            return(MAN_C_SUCCESS);

        case 2:    /* Fill in 1 default values */
            system_instance->sysLocation = "Unknown";
            system_instance->sysLocation_len = 7;
            return(MAN_C_SUCCESS);

        default:    /* All values have been read in */
            return(MAN_C_SUCCESS);
    }
}

man_status refresh_system_list( system_DEF *header)
{
	time_t time_now;
	
        (void)time(&time_now);
        header->next->sysUpTime = (time_now - sysStartTime - 1) * 100;
	return(MAN_C_SUCCESS);
}
