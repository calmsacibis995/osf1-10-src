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
static char *rcsid = "@(#)$RCSfile: inet_mom_specific.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 19:20:29 $";
#endif
/*
**++
**  FACILITY:  ZKO3-3
**
**  Copyright (c) 1993  Digital Equipment Corporation
**
**  MODULE DESCRIPTION:
**
**      INET_MOM_SPECIFIC.C
**
**      This module is part of the Managed Object Module (MOM)
**      for rfc1213 (Internet MOM).
**      It performs required initialization and other functions that
**      are specific to the rfc1213.
**
**  AUTHORS:
**
**      Geetha M. Brown
**
**  CREATION DATE:  15-Feb-1993
**
**  MODIFICATION HISTORY: 
**
**      26-May-1993 M. Ashraf       Removed openlog() call, as that is now
**				    done in init.c
**
**--
*/

/* KLUDGE.
 * On OSF <sys/un.h> defines "u_short sun_family" however
 * on Ultrix it's defined as  "short sun_family". We
 * include <sys/types.h> here, which defines the "u_" typedefs.
 * <sys/types.h> only defines the "u_" typedefs if _OSF_SOURCE
 * is defined.  Warning, if <sys/types.h> is included before
 * man_data.h they will not be defined.
 */

#if defined(__osf__)
# if !defined(_OSF_SOURCE)
#  define _OSF_SOURCE
#  include <sys/types.h>
#  undef _OSF_SOURCE
# else
#  include <sys/types.h>
# endif
#endif

#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <syslog.h>

#ifndef NOIPC
# include <pthread.h>   /* For trap polling thread */
#endif /* NOIPC */

#include "inet_mom_specific.h"
#include "moss.h"
#include "iso_defs.h"   /* required for building TRAPS */
#include "evd_defs.h"   /* required for building TRAPS */
#include "snmp_mib.h"


/*
 *  Link Table Entry Definition for Interface (LINK-UP/LINK-DOWN) Trap Support
 */

typedef struct link_entry {
    struct link_entry   *next;    /* pointer to next entry in Link Table list */
    int                  index;   /* index of "this" link in Interfaces Table */
    int                  state;   /* state of this link (initial|up|down)     */
} link_entry_t ;


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  LINK-UP/DOWN Trap Definitions                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/

#define LINK_TRAP_POLL_INTERVAL   60  /* seconds */

#define LINK_STATE_DOWN           1   /* our value for state = "down"    */
#define LINK_STATE_UP             2   /* our value for state = "up"      */
#define LINK_STATE_INITIAL        0   /* our value for state = "initial" */

#define IMOM_LINKUP_ARG_SEQ          1.1  /* OIDs MUST be 2 digits (1.x) */
#define IMOM_LINKUP_ARG_LENGTH       2
#define IMOM_LINKDOWN_ARG_SEQ        1.2  /* OIDs MUST be 2 digits (1.x) */
#define IMOM_LINKDOWN_ARG_LENGTH     2

#define IMOM_EVENTTYPE_LENGTH        1    /* EventType's OID portion of the */
                                          /* 'prefix' is only 1 arc...      */
#define IMOM_LINKTRAP_SPEC_TRAP_NUM  0    /* ...and here is the value of it */


unsigned int
    eventtype_prefix_array[ PREFIX_EVENTTYPE_LENGTH ] = 
        { PREFIX_EVENTTYPE_SEQ };

unsigned int
    enterprise_prefix_array[ PREFIX_ENTERPRISE_LENGTH ] = 
        { PREFIX_ENTERPRISE_SEQ };

unsigned int
    agentaddr_prefix_array[ PREFIX_AGENTADDR_LENGTH ] = 
        { PREFIX_AGENTADDR_SEQ };

unsigned int
    generictrap_prefix_array[ PREFIX_GENERICTRAP_LENGTH ] = 
        { PREFIX_GENERICTRAP_SEQ };

unsigned int
    specifictrap_prefix_array[ PREFIX_SPECIFICTRAP_LENGTH ] = 
        { PREFIX_SPECIFICTRAP_SEQ };

unsigned int
    eventtime_prefix_array[ PREFIX_EVENTTIME_LENGTH ] = 
        { PREFIX_EVENTTIME_SEQ };


/*------------*/

unsigned int
    imom_linktrap_generictrap_array[ PREFIX_GENERICTRAP_LENGTH + 
                                     ULTRIX_LENGTH + 1 ] = 
    { PREFIX_GENERICTRAP_SEQ, ULTRIX_SEQ, IMOM_LINKTRAP_SPEC_TRAP_NUM };

object_id
    imom_linktrap_generictrap_oid = { PREFIX_GENERICTRAP_LENGTH + 
                                      ULTRIX_LENGTH + 1,
                                      imom_linktrap_generictrap_array };

/*------------*/

unsigned int
    imom_linktrap_specifictrap_array[ PREFIX_SPECIFICTRAP_LENGTH + 
                                      ULTRIX_LENGTH + 1 ] = 
    { PREFIX_SPECIFICTRAP_SEQ, ULTRIX_SEQ, IMOM_LINKTRAP_SPEC_TRAP_NUM };

object_id
    imom_linktrap_specifictrap_oid = { PREFIX_SPECIFICTRAP_LENGTH + 
                                       ULTRIX_LENGTH + 1,
                                       imom_linktrap_specifictrap_array };

/*------------*/

unsigned int
    imom_linktrap_eventtype_array[ PREFIX_EVENTTYPE_LENGTH + 
                                   ULTRIX_LENGTH + 1 ] = 
    { PREFIX_EVENTTYPE_SEQ, ULTRIX_SEQ, IMOM_LINKTRAP_SPEC_TRAP_NUM };

object_id
    imom_linktrap_eventtype_oid = { PREFIX_EVENTTYPE_LENGTH + 
                                    ULTRIX_LENGTH + 1,
                                    imom_linktrap_eventtype_array };

/*------------*/

unsigned int
    imom_linktrap_enterprise_array[ PREFIX_ENTERPRISE_LENGTH + 
                                    ULTRIX_LENGTH + 1 ] = 
    { PREFIX_ENTERPRISE_SEQ, ULTRIX_SEQ, IMOM_LINKTRAP_SPEC_TRAP_NUM };

object_id
    imom_linktrap_enterprise_oid = { PREFIX_ENTERPRISE_LENGTH + 
                                     ULTRIX_LENGTH + 1,
                                     imom_linktrap_enterprise_array };

/*------------*/

unsigned int
    imom_linktrap_agentaddr_array[ PREFIX_AGENTADDR_LENGTH + 
                                   ULTRIX_LENGTH + 1 ] = 
    { PREFIX_AGENTADDR_SEQ, ULTRIX_SEQ, IMOM_LINKTRAP_SPEC_TRAP_NUM };

object_id
    imom_linktrap_agentaddr_oid = { PREFIX_AGENTADDR_LENGTH + 
                                    ULTRIX_LENGTH + 1,
                                    imom_linktrap_agentaddr_array };

/*------------*/

unsigned int
    imom_linktrap_eventtime_array[ PREFIX_EVENTTIME_LENGTH + 
                                   ULTRIX_LENGTH + 1 ] = 
    { PREFIX_EVENTTIME_SEQ, ULTRIX_SEQ, IMOM_LINKTRAP_SPEC_TRAP_NUM };

object_id
    imom_linktrap_eventtime_oid = { PREFIX_EVENTTIME_LENGTH + 
                                    ULTRIX_LENGTH + 1,
                                    imom_linktrap_eventtime_array };

/*------------*/

unsigned int
    imom_linkup_varbindarg_array[ PREFIX_VARBINDARG_LENGTH + 
                                  ULTRIX_LENGTH + 1 +
                                  IMOM_LINKUP_ARG_LENGTH ] = 
    { PREFIX_VARBINDARG_SEQ, ULTRIX_SEQ, IMOM_LINKTRAP_SPEC_TRAP_NUM,
      IMOM_LINKUP_ARG_SEQ };

object_id
    imom_linkup_varbindarg_oid = { PREFIX_VARBINDARG_LENGTH +
                                   ULTRIX_LENGTH + 1 +
                                   IMOM_LINKUP_ARG_LENGTH, 
                                   imom_linkup_varbindarg_array };

unsigned int
    imom_linkdown_varbindarg_array[ PREFIX_VARBINDARG_LENGTH + 
                                    ULTRIX_LENGTH + 1 +
                                    IMOM_LINKDOWN_ARG_LENGTH ] = 
    { PREFIX_VARBINDARG_SEQ, ULTRIX_SEQ, IMOM_LINKTRAP_SPEC_TRAP_NUM,
      IMOM_LINKDOWN_ARG_SEQ };

object_id
    imom_linkdown_varbindarg_oid = { PREFIX_VARBINDARG_LENGTH +
                                     ULTRIX_LENGTH + 1 +
                                     IMOM_LINKDOWN_ARG_LENGTH, 
                                     imom_linkdown_varbindarg_array };


/* 
 *  The Link Table is used by the link 'polling' thread for checking the
 *  the state of each TCP/IP-related Interface (LINK-UP & LINK-DOWN traps).
 */
link_entry_t  *imom_link_table;          /* The table is built dynamically */
int            imom_link_table_entries;  /* Count of entries in Link Table */


/*
 *  Information for the attributes syscontact and syslocation.  These are read
 *  out of the Internet configuration file.  The default Link Polling Timer
 *  value is "read-only" for the Common Agent (to change this value, you
 *  must manually edit the file; you can't do it via the Common Agent).
 */

char *default_string         = "Unknown" ;
char *default_polling_string = "60";     /* 60 seconds (read-only attribute) */



inet_mom_specific_init()
{
    struct  servent *sp;
    struct  hostent *hp;
    short   routing_port;
    char    host[64];

#ifndef NOIPC
    void mom_init_link_trap_thread();
#endif /* NOIPC */
    
    if (gethostname(host,64) < 0)
        return(MAN_C_FAILURE);

    if ((inetfd = open("/dev/streams/kinfo", O_RDWR, 0)) < 0) 
        return(MAN_C_FAILURE);

    /*
     * Set up to communicate with routing daemon.
     * Get the port number from the /etc/services file.
     * It should be 520.
     */

    sp = getservbyname("snmp-rt", "udp");

    if (sp == NULL)
        routing_port = htons((short)MOM_C_ROUTER_PORT);
    else
        routing_port = sp->s_port;

    if ((hp = gethostbyname(host)) == NULL)
        return(MAN_C_FAILURE);

    bzero((char *)&local_addr, sizeof(struct sockaddr_in));
    bzero((char *)&to_router, sizeof(struct sockaddr_in));

    local_addr.sin_family = AF_INET;
    to_router.sin_family = AF_INET;

    local_addr.sin_port = (short)0;
    to_router.sin_port = routing_port;

    bcopy(hp->h_addr, (char *)&local_addr.sin_addr.s_addr, hp->h_length);
    bcopy(hp->h_addr, (char *)&to_router.sin_addr.s_addr, hp->h_length);

    rtsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (rtsock < 0) 
        return(MAN_C_PROCESSING_FAILURE);

    if (bind(rtsock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) 
    {
        close(rtsock);
        return(MAN_C_PROCESSING_FAILURE);
    }

    /*
     *  Initialize the Link Table used for storing information about each 
     *  Interface (link) that we report the state of (LINK UP or DOWN).
     */

    imom_link_table         = NULL;
    imom_link_table_entries = 0;
#ifndef NOIPC
    mom_init_link_trap_thread();
#endif

}


compare_oid (first_oid,last_oid)

object_id *first_oid;
object_id *last_oid;
{
    int longer_depth ;
    int i ;
    int ret_value ;
    unsigned  int *first ;
    unsigned  int *last  ;

    if ( first_oid == NULL || last_oid == NULL ||
         first_oid->value == NULL || last_oid->value == NULL )
        return( NULL ) ;

    first = first_oid->value ;
    last  = last_oid->value ;

    /*
     * Get the longer length between the two oid's.
     */

    if ( first_oid->count > last_oid->count )
        longer_depth = first_oid->count ;
    else
        longer_depth = last_oid->count ;

    /*
     * Now compare each element.
     */

    for ( i = 0 ; i < longer_depth ; i++ )
    {
        /*
         * If the two oid's are not the name length, then one will
         * be greater or less than the other based on who is shorter
         */

        if ( i >= first_oid->count )
            return( MOM_C_LESS_THAN ) ;
        if ( i >= last_oid->count )
            return( MOM_C_GREATER_THAN ) ;

        ret_value = COMPARE( *first, *last ) ;

        first++ ;
        last++ ;

        if ( ret_value != MOM_C_EQUAL_TO )
            return( ret_value ) ;
    }

    return( ret_value ) ;

}  /* end of compare_oid() */


#ifndef NOIPC
/*
 * Function Name:
 *
 *    mom_init_link_trap_thread()
 *
 * Function description:
 *
 *    This is the link polling thread creation function for the SNMP Internet 
 *    MOM for ULTRIX/Mips.
 *
 * Arguments:
 *
 *     None.
 *
 * Return value:
 *
 *     None.
 *
 * Side effects:
 *
 *     If thread creation fails, the anomaly is logged, and the Imom continues
 *     to function OK.  Link traps will not be checked in this case.
 */

void mom_init_link_trap_thread()
{
    pthread_t  link_trap_polling_thread;    /* handle for thread fcns  */
    int        link_trap_polling_arg;       /* actually 0 args         */
    int        status;
    char       msg[250];

    extern void poll_for_link_traps();
    
    
    status = pthread_create (&link_trap_polling_thread,
                             pthread_attr_default,
                             (pthread_startroutine_t) poll_for_link_traps,
                             (pthread_addr_t) &link_trap_polling_arg);

    if (status != 0)
    {
        sprintf (msg,  
                "I043 - Failed to create link trap polling thread, status=%d\n",
                 status);
        syslog (LOG_ERR, msg);
        pthread_exit(0);
    }

} /* end of mom_init_link_trap_thread() */

#endif


/*
 * Function Name:
 *
 *    poll_for_link_traps()
 *
 * Function description:
 *
 *    This is the link polling thread creation function for the SNMP Internet 
 *    MOM for ULTRIX/Mips.
 *
 * Arguments:
 *
 *     One token argument is passed in, but not used for anything.
 *
 * Return value:
 *
 *     None.
 *
 * Side effects:
 *
 *     No MUTEX protection is required on the global Link Table or its
 *     associated entry count variable, as the single 'polling' thread has
 *     exclusive ownership of these variables.
 *
 */

void 
poll_for_link_traps (arg)
 int  arg;
{
    int                  status, if_count, i;
    int                  new_state;
    man_status           if_status, c_status;
    link_entry_t        *link_entryP, *tempP;
    char                 if_name[300];  /* lots of room; you never know... */
    char                 msg[250];      /* used for creating log messages  */
    octet_string         octet;

    extern link_entry_t *imom_link_table;   /* No need for Mutex protection */
    extern int           imom_link_table_entries;
    extern void          imom_send_event();
    extern man_status    find_ifnet_by_index();
    extern man_status    find_number_of_net_if();
    extern man_status    read_inet_config_file();

#ifndef NOIPC
    struct timespec      poll_interval; /* polling interval structure       */
    struct timespec      pe_init_delay; /* PE init delay interval structure */


    /* initialize interval polling & PE-init-delay timer values */
    octet.string = (char *) NULL;
    c_status = read_inet_config_file ( (int) LINK_POLLING_INTERVAL, 
                                        &octet.string);

    if (c_status != MAN_C_SUCCESS)
    {
        poll_interval.tv_sec = LINK_TRAP_POLL_INTERVAL;
    }
    else
    {
        poll_interval.tv_sec = atoi (octet.string);
        if (poll_interval.tv_sec == 0)
            poll_interval.tv_sec = LINK_TRAP_POLL_INTERVAL;
    }

    poll_interval.tv_nsec = 0;

    pe_init_delay.tv_sec  = 15;   /* delay first "polling" for 15 seconds */
    pe_init_delay.tv_nsec = 0;    /* to give the SNMP_PE time to start up */
#endif


#ifndef NOIPC
    /* FOREVER */
    while (1)
    {
#endif
        /* create the imom_link_table if it does not yet exist */
        if (imom_link_table == NULL)
        {
            /* get number of TCP/IP-related interfaces */
            if_status = find_number_of_net_if (&if_count);
            if (if_status != MAN_C_SUCCESS)
                return;

            /* for each interface, construct a local link table entry */
            imom_link_table_entries = if_count;
            for (i = 1; i <= if_count; i++)
            {
                if_status = find_ifnet_by_index (i, &new_state);
                if (if_status != MAN_C_SUCCESS)
                    return;

                link_entryP = (link_entry_t *) malloc ( sizeof(link_entry_t) );
                if (link_entryP == NULL)
                {
                    sprintf (msg, "I045 - malloc of linktable entry FAILED\n");
                    syslog (LOG_ERR, msg);
#ifndef NOIPC
                    pthread_exit(0);
#else
                    exit(0);
#endif
                }
                else
                {
                    link_entryP->index = i;
                    link_entryP->state = LINK_STATE_INITIAL;
                    link_entryP->next  = NULL;

                    /* assign this entry to start of table if table is empty */
                    if (imom_link_table == NULL)
                        imom_link_table = link_entryP;
                    else 
                    {
                        /* add entry to the end of the list */
                        tempP = imom_link_table;
                        while (tempP->next != NULL)
                            tempP = tempP->next;
                        tempP->next = link_entryP;
                    }
                }
            }

#ifndef NOIPC
            /* sleep for a small interval to allow PE to finish initializing */
            status = pthread_delay_np (&pe_init_delay);
            if (status != 0)
            {
                sprintf (msg, 
                     "I052 - pthread_delay_np FAILED (status=%d); exiting...\n",
                         status);
                syslog (LOG_ERR, msg);
                pthread_exit(0);
            }
#endif
        }

        /* ELSE add new entries (if any) to imom_link_table as needed */
        else
        {
            /* get current number of interfaces */
            if_status = find_number_of_net_if (&if_count);
            if (if_status != MAN_C_SUCCESS)
                return;

            /* if any NEW entries, add them to the end of the list */
            if (if_count > imom_link_table_entries)
            {
                for (i = imom_link_table_entries+1; i <= if_count; i++)
                {
                    if_status = find_ifnet_by_index (i, &new_state);
                    if (if_status != MAN_C_SUCCESS)
                        return;

                    link_entryP = (link_entry_t *) 
                                   malloc ( sizeof(link_entry_t) );
                    if (link_entryP == NULL)
                    {
                        sprintf (msg, 
                                 "I046 - malloc of linktable entry FAILED\n");
                        syslog (LOG_ERR, msg);
#ifndef NOIPC
                        pthread_exit(0);
#else
                        exit(0);
#endif
                    }

                    link_entryP->index = i;
                    link_entryP->state = LINK_STATE_INITIAL;
                    link_entryP->next  = NULL;

                    /* add entry to the end of the list */
                    tempP = imom_link_table;
                    while (tempP->next != NULL)
                        tempP = tempP->next;
                    tempP->next = link_entryP;
                }

                imom_link_table_entries = if_count;
            }
        } 

        /* For each entry in our Link Table, check the link's state  */ 
        /* against the "old" state from the last time thru this loop */
        link_entryP = imom_link_table;

        while (link_entryP != NULL)
        {
            /* use the if_index from the link_entry to fetch the info */
            if_status = find_ifnet_by_index (link_entryP->index, &new_state);
            if (if_status != MAN_C_SUCCESS)
                return;

            if (new_state != link_entryP->state)
            {
                if (new_state == LINK_STATE_UP)
                {
                    /* set link_entry state to UP */
                    link_entryP->state = LINK_STATE_UP;

                    /* issue LINK UP trap */
                    imom_send_event (link_entryP);
                }
                else /* DOWN and LOOPBACK_TESTING are both considered "DOWN" */
                {
                    /* set linkEntry state to DOWN */
                    link_entryP->state = LINK_STATE_DOWN;

                    /* issue LINK DOWN trap */
                    imom_send_event (link_entryP);
                }
            }

            link_entryP = link_entryP->next;
        }

#ifndef NOIPC
        /* sleep for "polling interval" seconds */
        status = pthread_delay_np (&poll_interval);
        if (status != 0)
        {
            sprintf (msg, 
                     "I047 - pthread_delay_np FAILED (status=%d); exiting...\n",
                     status);
            syslog (LOG_ERR, msg);
            pthread_exit(0);
        }
#endif

#ifndef NOIPC
    }
#endif

#ifndef NOIPC
    pthread_exit(0);
#endif

} /* end of poll_for_link_traps() */


/*
 * Function Name:
 *
 *    imom_send_event()
 *
 * Function description:
 *
 *    This function creates the proper LINK-UP or LINK-DOWN event and
 *    sends the event to EVD for further processing.
 *
 * Arguments:
 *
 *     link_entryP - pointer to Link Table entry representing the 
 *                   link/interface for which to report the event.
 *
 * Return value:
 *
 *     None.
 *
 * Side effects:
 *
 *     None.
 */

void
imom_send_event (link_entryP)
 link_entry_t  *link_entryP;
{
    man_status             tstatus;
    evd_queue_handle      *q_handle;
    evd_queue_access_mode  q_mode;
    object_id             *event_type_oid=NULL;
    avl                   *event_parms_avl=NULL;
    uid                    event_uid;
    uid                    entity_uid;
    octet_string           os;
    char                   msg[250];

    int generic_trap_num;
    int linkup_trap_num   = 3;        /* SNMP generic trap #3 is LINK UP   */
    int linkdown_trap_num = 2;        /* SNMP generic trap #2 is LINK DOWN */
    int specific_trap_num = IMOM_LINKTRAP_SPEC_TRAP_NUM; /* LINK traps are */
                                                         /* generic        */

    extern object_id    sys_obj_id_oid;
    extern octet_string sysid_octet; /* init'd in system_load_structures() */


    /* get a queue handle into EVD (queue name is not currently used by EVD) */
    q_mode = EVD_POST;
    tstatus = evd_create_queue_handle (&q_handle, NULL, q_mode);

    if (tstatus == MAN_C_SUCCESS)
    {
        /* SET UP EVENT_PARAMETERS AVL: */
        tstatus = moss_avl_init (&event_parms_avl);

        /* set up the generic event_type oid required in evd_post_event() */
        /* call and in the event parameters avl                           */
        if (tstatus == MAN_C_SUCCESS)
        {
            if (link_entryP->state == LINK_STATE_UP)
                generic_trap_num = linkup_trap_num;
            else
                generic_trap_num = linkdown_trap_num;

            tstatus = moss_create_oid (1, (unsigned int *) &generic_trap_num,
                                       &event_type_oid);
        }

        /* set up and add Event Type (==>Specific trap) OID arg */
        if (tstatus == MAN_C_SUCCESS)
        {
            os.length = IMOM_EVENTTYPE_LENGTH * sizeof(unsigned int);
            os.data_type = ASN1_C_OCTET_STRING;
            os.string = (char *) &specific_trap_num;

            tstatus = moss_avl_add (event_parms_avl, 
                                    &imom_linktrap_eventtype_oid,
                                    MAN_C_SUCCESS, ASN1_C_OCTET_STRING, &os);
        }

        /* add already initialized Enterprise OID (system object id) arg */
        if (tstatus == MAN_C_SUCCESS)
        {
            tstatus = moss_avl_add (event_parms_avl, 
                                    &imom_linktrap_enterprise_oid,
                                    MAN_C_SUCCESS, ASN1_C_OBJECT_ID, 
                                    &sysid_octet);
        }

        /* set up and add Generic Trap Number arg */
        if (tstatus == MAN_C_SUCCESS)
        {
            os.length = sizeof(int);
            os.data_type = ASN1_C_INTEGER;
            os.string = (char *) &generic_trap_num;

            tstatus = moss_avl_add (event_parms_avl, 
                                    &imom_linktrap_generictrap_oid,
                                    MAN_C_SUCCESS, ASN1_C_INTEGER, &os);
        }

        /* set up and add Specific Trap Number arg */
        if (tstatus == MAN_C_SUCCESS)
        {
            os.length = sizeof(int);
            os.data_type = ASN1_C_INTEGER;
            os.string = (char *) &specific_trap_num;
            tstatus = moss_avl_add (event_parms_avl, 
                                    &imom_linktrap_specifictrap_oid,
                                    MAN_C_SUCCESS, ASN1_C_INTEGER, &os);
        }

        /* put the actual link number into the varbind arg */
        if (tstatus == MAN_C_SUCCESS)
        {
            os.length = sizeof(int);
            os.data_type = ASN1_C_INTEGER;
            os.string = (char *) &link_entryP->index;

            if (generic_trap_num == linkup_trap_num)
                tstatus = moss_avl_add (event_parms_avl, 
                                        &imom_linkup_varbindarg_oid,
                                        MAN_C_SUCCESS, ASN1_C_INTEGER, &os);
            else
                tstatus = moss_avl_add (event_parms_avl, 
                                        &imom_linkdown_varbindarg_oid,
                                        MAN_C_SUCCESS, ASN1_C_INTEGER, &os);
        }

        /* post the event to EVD */
        if (tstatus == MAN_C_SUCCESS)
        {
            tstatus = evd_post_event (q_handle, 
                                      &sys_obj_id_oid,
                                      NULL,       /* instance_name IGNORED  */ 
                                      NULL,       /* let event time default */
                                      event_type_oid,
                                      event_parms_avl,
                                      &event_uid, /* not used for anything  */
                                      &entity_uid);

            if (tstatus != MAN_C_SUCCESS)
            {
                sprintf (msg, 
                        "I048 - evd_post_event FAILED, tstatus=%d\n", tstatus);
                syslog (LOG_ERR, msg);
	    }
        }

        tstatus = evd_delete_queue_handle (&q_handle);
        if (tstatus != MAN_C_SUCCESS)
	{
            sprintf (msg, "I049 - evd_delete_queue_handle FAILED, tstatus=%d\n",
                     tstatus);
            syslog (LOG_ERR, msg);
	}

        if (event_type_oid != NULL)
            tstatus = moss_free_oid (event_type_oid);
        if (event_parms_avl != NULL)
            tstatus = moss_avl_free (&event_parms_avl, TRUE);
    }

    else /* couldn't get a queue handle; log this anomaly */
    {
        sprintf (msg, "I050 - evd_create_queue_handle FAILED, tstatus=%d\n",
                 tstatus);
        syslog (LOG_ERR, msg);
    }

}  /* end of imom_send_event() */

/*
 *    Function Name:
 *      find_number_of_net_if()
 *
 *
 *    Function Description:
 *    	This function seeks the if_blk structure out of the kernel and
 *	reads in the required data into a provided structure, and returns
 *      the ifNumber value in the provided return value.
 *    
 *    Arguments:
 *	if_countP      	Input/Output : A pointer to the if_count value
 *				       that needs to be filled in.
 *    Return Value:
 *   
 *         MAN_C_SUCCESS
 *	   MAN_C_PROCESSING_FAILURE
 *    
 *    Side Effects:
 *	None.
 */

man_status 
find_number_of_net_if (if_countP)
 int   *if_countP;      /* ptr to count field to update */
{
    if_blk          ifs;
    struct strioctl str;

    bzero (&ifs, sizeof(if_blk));
    str.ic_cmd    = KINFO_GET_INTERFACES;
    str.ic_timout = 15;
    str.ic_len    = sizeof(if_blk);
    str.ic_dp     = (char *)&ifs;

    if (ioctl (inetfd, I_STR, &str) < 0) 
    {
        perror ("ioctl");
        return (MAN_C_PROCESSING_FAILURE);
    }

    *if_countP = ifs.curr_cnt;  
    return (MAN_C_SUCCESS);
}


/*
 *    Function Name:
 *      find_ifnet_by_index()
 *    
 *    
 *    Function Description:
 *    	This function seeks the if_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *      if_index  - interface number to find
 *      stateP    - address of placeholder for interface's state
 *   
 *    Return Value:
 *         MAN_C_SUCCESS
 *	   MAN_C_PROCESSING_FAILURE
 *    
 *    Side Effects:
 *	None.
 */

man_status 
find_ifnet_by_index (if_index, stateP)
 int    if_index;
 int   *stateP;
{
    if_blk          ifs;
    struct strioctl str;
    int             j;

    bzero (&ifs, sizeof(if_blk));
    str.ic_cmd    = KINFO_GET_INTERFACES;
    str.ic_timout = 15;
    str.ic_len    = sizeof(if_blk);
    str.ic_dp     = (char *)&ifs;

    do 
    {
        if (ioctl (inetfd, I_STR, &str) < 0) 
        {
           perror (" ioctl");
           return (MAN_C_PROCESSING_FAILURE);
        }

        for (j=0; j < ifs.curr_cnt; j++) 
        {
            /* see if this instance matches the if_index passed in */
            if ( if_index == (j+1) )
            {           
                if (ifs.info[j].if_operstatus & IFF_UP) 
                    *stateP = LINK_STATE_UP;
                else
                    *stateP = LINK_STATE_DOWN; /* includes LOOP_TESTING state */
                return (MAN_C_SUCCESS);
            }
        }

    } while(ifs.more);

    return (MAN_C_PROCESSING_FAILURE);
}

/*
 * Function Name: 
 *    read_inet_config_file()
 *
 * Function Description:
 *
 *    This is the routine that reads the internet mom's .conf file for
 *    either the syscontact, syslocation, or default link polling interval
 *    timer attributes.
 *
 *    This routine is dependent on the format of the file
 *    being:
 *        The first line contains the location.
 *        The second line contains the contact.
 *        The third line contains the Internet MOM polling interval.
 *        All lines beginning with a # are ignored as comment lines.
 *
 * Arguments:
 *
 *    what_to_read                 integer indicating attribute to fetch
 *    info_string                  the address of the pointer to return the
 *                                 information
 *
 * Return values:
 *
 *    MAN_C_SUCCESS                 Successful
 *    MAN_C_PROCESSING_FAILURE      Failure accessing information
 *
 * Side effects:
 * 
 *    None.
 */

man_status
read_inet_config_file (what_to_read, info_string)
 int  what_to_read ;
 char **info_string ;
{
    FILE *fileid ;                 /* config file - inet_momd.conf */
    char buf[ MOM_C_MAX_BUFLEN ] ; /* buffer to read data into */
    char *ptr ;
    int len = 0 ;
    int line = 1 ;
    int found = FALSE ;

    fileid = fopen( CONFIG_FILE, "r" ) ;

    /*
     *  If there is no file found on the system we use the default
     *  of "Unknown".
     */

    if ( fileid == NULL )
    {
        syslog( LOG_INFO,
                "Warning: No Internet MOM configuration file.\n");
        *info_string = default_string ;
        return( MAN_C_SUCCESS ) ;
    }

    /*
     *  There is a file so extract the attributes.  Only read as much of the
     *  file as necessary to get the requested attributes.
     */

    memset( ( void * )buf, '\0', MOM_C_MAX_BUFLEN ) ;
    while( ( found == FALSE ) && ( line < 4 ) && ( fgets( buf, MOM_C_MAX_BUFLEN, fileid ) != NULL ) )
    {
        if ( ( buf[ 0 ] == '\n' ) || ( buf[ 0 ] == '#' ) )
        {
            memset( ( void * )buf, '\0', MOM_C_MAX_BUFLEN ) ;
            continue ;
        }

        switch ( line )
        {
            case 1 :
                if ( what_to_read == SYSTEM_LOCATION )
                {
                    ptr = (char *) index( buf, '\n' ) ;
                    *ptr = '\0' ;
                    len = strlen( buf ) + 1 ;
                    *info_string = (char *) malloc (len);

                    if ( *info_string != NULL )
                    {
                        memset( ( void * )*info_string, '\0', len ) ;
                        strncpy( *info_string, buf, len - 1 ) ;
                        found = TRUE ;
                        memset( ( void * )buf, '\0', MOM_C_MAX_BUFLEN ) ;
                    }
                    else
                    {
                        fclose( fileid ) ;
                        return( MAN_C_PROCESSING_FAILURE ) ;
                    }
                }
                line++ ;
                break ;

            case 2 :
                if ( what_to_read == SYSTEM_CONTACT )
                {
                    ptr = (char *) index( buf, '\n' ) ;
                    *ptr = '\0' ;
                    len = strlen( buf ) + 1 ;
                    *info_string = (char *) malloc (len);

                    if ( *info_string != NULL )
                    {
                        memset( ( void * )*info_string, '\0', len ) ;
                        strncpy( *info_string , buf, len - 1 ) ;
                        found = TRUE ;
                    }
                    else
                    {
                        fclose( fileid ) ;
                        return( MAN_C_PROCESSING_FAILURE ) ;
                    }
                 }
                 line++ ;
                 break ;

            case 3 :
                if ( what_to_read == LINK_POLLING_INTERVAL )
                {
                    ptr = (char *) index( buf, '\n' ) ;
                    *ptr = '\0' ;
                    len = strlen( buf ) + 1 ;
                    *info_string = (char *) malloc (len);

                    if ( *info_string != NULL )
                    {
                        memset( ( void * )*info_string, '\0', len ) ;
                        strncpy( *info_string , buf, len - 1 ) ;
                        found = TRUE ;
                    }
                    else
                    {
                        fclose( fileid ) ;
                        return( MAN_C_PROCESSING_FAILURE ) ;
                    }
                 }
                 line++ ;
                 break ;

            default :
                 break ;
        }
    }

    if ( found == FALSE )
        *info_string = default_string ;
    fclose( fileid ) ;
    return( MAN_C_SUCCESS );

}  /* end of read_inet_config_file() */


/*
 * Function description:
 *
 *    write_inet_config_file()
 *
 * Function description:
 *
 *    This is the routine that writes out either the new
 *    syscontact or syslocation to the inet_mom.conf file.
 *
 *    This routine is dependent on the format of the file
 *    being:
 *        The first line contains the location.
 *        The second line contains the contact.
 *        The third line contains the Internet MOM polling interval.
 *        All lines beginning with a # are ignored as comment lines.
 *
 * Arguments:
 *
 *    what_to_write      Integer indicating which attribute to set; can be
 *                       SYSTEM_LOCATION (1)  or SYSTEM_CONTACT (2) only
 *                       (see inet_mom_specific.h).  The POLLING_INTERVAL (3) 
 *                       is read-only.
 *    set_string         String to use for set.
 *
 * Return values:
 *
 *    MAN_C_SUCCESS                 Successful
 *    MAN_C_PROCESSING_FAILURE      Failure accessing information
 *
 * Side effects:
 * 
 */

man_status
write_inet_config_file (what_to_write, set_string)
 int          what_to_write ;
 octet_string *set_string ;
{
    FILE *fileid ;                 /* config file - inet_momd.conf */
    FILE *tmpid ;                  /* temporary configuration file */
    char buf[ MOM_C_MAX_BUFLEN ] ; /* buffer to read data into */
    char *tmp_name ;
    int in_len = 0 ;
    int line = 1 ;
    int accessible = 0 ;
    int system_status = 0 ;


    accessible = access( CONFIG_FILE, R_OK | W_OK ) ;
    if ( accessible == 0 )
    {
        fileid = fopen( CONFIG_FILE, "r+" ) ;

        if ( fileid == NULL )
        {
            syslog (LOG_INFO, 
                   "Warning: Error reading Internet MOM configuration file.\n");
            return( MAN_C_PROCESSING_FAILURE ) ;
        }
        else
        {
            tmp_name = tempnam( "/tmp", "INET_" ) ;
            if ( tmp_name == ( char * )NULL )
            {
                fclose( fileid ) ;
                return ( MAN_C_PROCESSING_FAILURE ) ;
            }

            tmpid = fopen( tmp_name, "w" ) ;
            if ( tmpid == NULL )
            {
                fclose( fileid ) ;
                return( MAN_C_PROCESSING_FAILURE ) ;
            }

            /*
             *  There is a file so find the desired attribute and replace it.
             */

            while( fgets( buf, MOM_C_MAX_BUFLEN, fileid ) != NULL )
            {
                if ( ( buf[ 0 ] == '\n' ) || ( buf[ 0 ] == '#' ) )
                {
                    in_len = strlen( buf ) ;
                    ( void )fwrite( buf, sizeof( char ), in_len, tmpid ) ;
                    memset( ( void * )buf, '\0', in_len ) ;
                    continue ;
                }

                switch ( line )
                {
                    case 1 :
                        if ( what_to_write == SYSTEM_LOCATION )
                        {
                            ( void )fwrite( set_string->string, sizeof( char ), set_string->length, tmpid ) ;
                            ( void )fwrite( "\n", sizeof( char ), 1, tmpid ) ;
                        }
                        else
                        {
                            in_len = strlen( buf ) ;
                            ( void )fwrite( buf, sizeof( char ), in_len, tmpid ) ;
                        }
                        fflush( tmpid ) ;
                        line ++ ;
                        break ;

                    case 2 :
                        if ( what_to_write == SYSTEM_CONTACT )
                        {
                            ( void )fwrite( set_string->string, sizeof( char ), set_string->length, tmpid ) ;
                            ( void )fwrite( "\n", sizeof( char ), 1, tmpid ) ;
                        }
                        else
                        {
                            in_len = strlen( buf ) ;
                            ( void )fwrite( buf, sizeof( char ), in_len, tmpid ) ;
                        }
                        fflush( tmpid ) ;
                        line ++ ;
                        break ;

                    default :
                        in_len = strlen( buf ) ;
                        ( void )fwrite( buf, sizeof( char ), in_len, tmpid ) ;
                        break ;
                }
                in_len = strlen( buf ) ;
                memset( ( void * )buf, '\0', in_len ) ;
                continue ;
            }
            fclose( fileid ) ;
            fclose( tmpid ) ;

            /*
             *  Copy the tmp file to the real file.
             */

            fileid = fopen( CONFIG_FILE, "w" ) ;
            tmpid = fopen( tmp_name, "r" ) ;
            while( fgets( buf, MOM_C_MAX_BUFLEN, tmpid ) != NULL )
            {
                in_len = strlen( buf ) ;
                ( void )fwrite( buf, sizeof( char ), in_len, fileid ) ;
                memset( ( void * )buf, '\0', in_len ) ;
            }
            fclose( fileid ) ;
            fclose( tmpid ) ;
            free( tmp_name ) ;
            remove( tmp_name ) ;
        }
    }
    else
    {
        /*
         *  The file does not exist.  So create it.
         */

        fileid = fopen( CONFIG_FILE, "w" ) ;
        if ( fileid == NULL )
            return( MAN_C_PROCESSING_FAILURE ) ;

        ( void )fwrite( "#syslocation\n", sizeof( char ), 13, fileid ) ;
        if ( what_to_write == SYSTEM_LOCATION )
            ( void )fwrite( set_string->string, sizeof( char ), 
                            set_string->length, fileid )  ;
        else
            ( void )fwrite( default_string, sizeof( char ), 
                            strlen( default_string ), fileid ) ;
        ( void )fwrite( "\n\n", sizeof( char ), 2, fileid ) ;

        ( void )fwrite( "#syscontact\n", sizeof( char ), 12, fileid ) ;
        if ( what_to_write == SYSTEM_CONTACT )
            ( void )fwrite( set_string->string, sizeof( char ), 
                            set_string->length, fileid )  ;
        else                                                 
            ( void )fwrite( default_string, sizeof( char ), 
                            strlen( default_string ), fileid ) ;
        ( void )fwrite( "\n\n", sizeof( char ), 2, fileid ) ;

        ( void )fwrite( "#Link Polling Interval\n", sizeof( char ), 
                        23, fileid ) ;
        ( void )fwrite( default_polling_string, sizeof( char ), 
                        strlen( default_polling_string ), fileid ) ;
        ( void )fwrite( "\n", sizeof( char ), 1, fileid ) ;

        fflush( fileid ) ;
        fclose( fileid ) ;
    }
    return( MAN_C_SUCCESS );

}  /* end of write_inet_config_file() */


/*
 * Function description:
 *
 *    This is the entry point for processing a set for the attribute
 *    snmpenableauthentraps.
 *
 * Arguments:
 *
 *    value                        new value for the set
 *
 * Return values:
 *
 *    MAN_C_SUCCESS                Operation successfully executed
 *    MAN_C_SET_LIST_ERROR         Insufficient resources
 *
 * Side effects:
 * 
 *    None.
 */

man_status
snmpenableauthentraps_set(
                          value
                         )
int value ;
{
    extern struct snmp_stats    *s_stat;

    if ( ( value == 1 ) || ( value == 2 ) )
       s_stat->enableauthtrap = value ;

    else
        return( MAN_C_SET_LIST_ERROR ) ;

}  /* end of snmpenableauthentraps_set() */

/* end of inet_mom_specific.c */
