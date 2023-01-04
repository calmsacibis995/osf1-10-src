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
static char *rcsid = "@(#)$RCSfile: snmppe_netio.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:25:33 $";
#endif
/*
 **  Copyright (c) Digital Equipment Corporation, 1991, 1992
 **  All Rights Reserved.  Unpublished rights reserved
 **  under the copyright laws of the United States.
 **  
 **  The software contained on this media is proprietary
 **  to and embodies the confidential technology of 
 **  Digital Equipment Corporation.  Possession, use,
 **  duplication or dissemination of the software and
 **  media is authorized only pursuant to a valid written
 **  license from Digital Equipment Corporation.
 **
 **  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 **  disclosure by the U.S. Government is subject to
 **  restrictions as set forth in Subparagraph (c)(1)(ii)
 **  of DFARS 252.227-7013, or in FAR 52.227-19, as
 **  applicable.
 **
 *
 * MODULE DESCRIPTION:
 *
 * Common Agent SNMP Protocol Engine
 *
 * Module SNMPPE_NETIO.C
 *      Contains "Common Agent - Network I/O Functions" for the
 *      SNMP Protocol Engine for the Comman Agent.
 *
 *      These are functions providing network I/O for both the sending
 *      and receiving threads of the Protocol Engine.  (For V1.0 only
 *      the sending thread makes calls to these functions).
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   Sept 1991
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines accepts requests over a  network,
 *       converts incoming information into standard Common Agent format
 *       and calls the Common Agent for processing of that request.
 *
 *    Purpose:
 *       This module contains the network I/O functions required by
 *       the SNMP Protocol Engine.  These functions not only send and
 *       receive PDUs, but also "serialize" and "deserialize" PDUs
 *       (ie convert using the ASN.1 encoding rules)
 *
 * History
 *      V1.0    Sept 1991               D. D. Burns
 *      V1.1    May 1992                D. D. Burns, add code to release
 *                                        storage associated with "snmp_oid"
 *                                        in vbe.

Module Overview:
---------------

This module contains the SNMP PE function(s) that perform Network I/O
and conversion of the PDU information to/from internal data structures.


Thread Overview:
---------------

Both the sending and receiving threads may call functions contained in this
module, however only the sending thread does in the current implementation.

MODULE CONTENTS:

USER-LEVEL INTERFACE    (ie Functions defined in this module for reference
                         elsewhere):

Function Name           Synopsis
-------------           --------
strerror		Print errno message (to support SunOS only)

netio_get_pdu           Waits for an inbound PDU to arrive, logs errors
                        incurred until a PDU can be successfully returned

netio_deserialize_pdu   Applies ASN.1 encoding rules to a supplied PDU
                        to parse the PDU into a SNMP PE internal data
                        structure.  (This function provides an interface
                        to Arun Sankar's SNMP-LIB function that uses MCC
                        ASN.1 general-purpose decode routines).

netio_put_pdu           Sends a PDU outbound to address from which it came.

netio_serialize_pdu     Applies ASN.1 encoding rules to information in
                        SNMP PE internal data structures to create a valid
                        PDU ready for sending.  (This function provides an
                        interface to Arun Sankar's SNMP-LIB function that 
                        uses MCC ASN.1 general-purpose encode routines).

netio_send_trap         Applies ASN.1 encoding rules to create a specific
                        Trap PDU and sends it to all known trap communities.


MODULE INTERNAL FUNCTIONS:

Function Name           Synopsis
-------------           --------

netio_asn1_dump         Serves to interface SNMP PE to MCC ASN.1 Dump
                        function.
*/

/* Module-wide Stuff */
#include <syslog.h>
#include <string.h>

#if defined(__osf__) && !defined(_OSF_SOURCE)
/* KLUDGE. <sys/socket.h> needs <sys/types.h>, which doesn't define "u_"
   typedefs unless you've got _OSF_SOURCE turned on. */
# define _OSF_SOURCE
# include <sys/types.h>
# undef _OSF_SOURCE

#else
#include <sys/types.h>
#endif

#include <sys/socket.h>
#include <errno.h>
#include <malloc.h>
#include <sys/time.h>


/* includes required for "snmppe.h" */
#include "port_cmip.h"
#include <stdio.h>
#include "moss.h"
#include <sys/types.h>
#include <netinet/in.h>
#define OID_DEF
#include "snmppe.h"

#include "snmppe_msg.h"
#include "snmppe_snmplib.h"

#include <arpa/inet.h>

/* external */

#if defined(sun) || defined(sparc)

extern char *sys_errlist[];                /* vector of message strings */
extern int sys_nerr;                       /* number of messages */

#endif

/*
|
|   Define Prototypes for Module-Local Functions
|
*/

#if defined(sun) || defined(sparc)

/* strerror - Print errno message (to support SunOS only) */
char *
strerror PROTOTYPE((
int                     /* errno */
));

#endif

/* netio_asn1_dump - Interface to MCC ASN.1 Dump function */
static void
netio_asn1_dump PROTOTYPE((
big_picture_t    *,         /*-> Big Picture Structure for SNMP PE           */
MCC_T_Descriptor *,         /*-> Descriptor for buf to rcv encoded PDU image */
int                         /* Bit Mask corresponding to Message Class       */
));


#if defined(sun) || defined(sparc)

/* SunOS on Sparc does not have strerror(), so we write our own. */

/* strerror - Print errno message */
/* strerror - Print errno message */
/* strerror - Print errno message */

char *
strerror(errno)
int errno;      /* errno number */

/*
INPUTS:

    errno is the errno number

OUTPUTS:

    This function returns a pointer to an error message string.

BIRD'S EYE VIEW:
    Purpose:
	The caller supplies the system error number errno. A pointer to
	the corresponding error message string is returned from sys_errlist[]
	if the errno is within the range (sys_nerr). If not, a pointer to
	an internal message is returned.

*/

{
static char buf1[40];
char buf2[20];

if ((errno > 0) && (errno <= sys_nerr))
    return(sys_errlist[errno]);
else {
    strcpy(buf1, "Unknown errno: ");
    sprintf(buf2, "%d", errno);
    strcat(buf1, buf2);
    return(buf1);
}

}

#endif


/* netio_get_pdu - Get Inbound PDU */
/* netio_get_pdu - Get Inbound PDU */
/* netio_get_pdu - Get Inbound PDU */

void
netio_get_pdu( bp, svc, pdu_info )

big_picture_t    *bp;           /*-> Big Picture Structure for SNMP PE                */
service_t        *svc;          /*-> Service Blk (on Service List) to be receive pdu  */
MCC_T_Descriptor *pdu_info;     /*-> Descriptor for buffer to receive the PDU image   */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is a pointer (to a Service Block) that is to receive the contents of the
    received PDU.  The network address from which the PDU is received is stored in here.

    "pdu_info" is the address of a descriptor for a buffer that is to receive the inbound
    PDU image.


OUTPUTS:

    The function returns only when a PDU has been successfully received.

    The length of the PDU received is stored into the pdu descritpor while the network
    address is stored in the service block for that PDU.
    

BIRD'S EYE VIEW:
    Context:
        The caller is the sending thread in casend_get_msg().  It wants
        to obtain the next PDU inbound.

    Purpose:
        This function performs network I/O to receive the SNMP PDU.
        Errors are reported until a PDU is correctly received.

ACTION SYNOPSIS OR PSEUDOCODE:

    <initialize error count to zero>
    <initialize timeout structure to "poll">

    for (ever)

        if (error occurred on recvfrom)
            <SYSLOG "Error during recvfrom: %s">
            <count another error>
            if (error count > 20)
                <CRASH "Error limit exceeded">
            <continue>

        <store length of PDU>
        <count another inbound packet, presumed to be snmp>
        <return>


OTHER THINGS TO KNOW:

    MTHREADS:
    This function does not currently acquire a mutex for the port it uses
    to do the receive for the incoming the PDU.  If code is added in main()
    to spawn multiple threads to act as a sending threads, then this function
    here probably must be changed to acquire mutex "in_m" in Big Picture
    before trying to do the recvfrom().  You should review the DCE docs
    regarding what functions get jacketed to prevent image blocking, in which
    case this may not be needed.
*/

{
int             status;         /* Status returned from system calls    */
int             errcount;       /* Count of continuous errors           */
char            msg[LINEBUFSIZE];       /* Error message build buffer   */
int             buffer_length;  /* Length extracted from descriptor     */
int             rcvd_length;    /* Length of PDU actually received      */
extern int      recvfrom();


errcount = 0;   /* initialize error count to zero */

for (;/* ever */;) {

    /* attempt recvfrom */
    buffer_length = pdu_info->mcc_w_maxstrlen;      /* Copy buf length   */
    rcvd_length = sizeof(svc->inbound);             /*    to integer     */
    status = recvfrom( bp->in_socket,               /* Socket            */
                       pdu_info->mcc_a_pointer,     /* Buffer            */
                       buffer_length,               /* Buffer Size       */
                       0,                           /* Flags             */
                       &svc->inbound,               /* Sending Address   */
                       &rcvd_length);               /* Address buf size  */

    /* if (error occurred on recvfrom) */
    if (status < 0 ) {
        /* SYSLOG "Error during recvfrom: %s" */
        sprintf(msg,
                MSG(msg045, "M045 - Error during recvfrom: '%s'\n"), strerror(errno));
        SYSLOG( LOG_ERR, msg );

        errcount += 1;  /* count another error */

        if (errcount > 20) {
            CRASH(MSG(msg046, "M046 - Internal RECVFROM Error limit exceeded"));
            }
        continue;
        }

    pdu_info->mcc_w_curlen = status;        /* Store the length of the PDU */

    /* count another inbound packet */
    bp->statistics->inpkts += 1;

    return;                                 /* SUCCESS! */

    }   /* forever */
}

/* netio_deserialize_pdu - Deserialize ASN.1-encoded PDU */
/* netio_deserialize_pdu - Deserialize ASN.1-encoded PDU */
/* netio_deserialize_pdu - Deserialize ASN.1-encoded PDU */

BOOL
netio_deserialize_pdu( bp, svc, pdu_info )

big_picture_t    *bp;       /*-> Big Picture Structure for SNMP PE           */
service_t        *svc;      /*-> Svc Blk (on Service List) to rcv PDU info   */
MCC_T_Descriptor *pdu_info; /*-> Buffer Descriptor for buf holding PDU image */


/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is the address of a Service Block that is to receive the information
    encoded in the PDU.

    "pdu_info" is the address of a descriptor for a buffer that contains the
    inbound PDU image.


OUTPUTS:

    The function returns TRUE when a PDU has been successfully deserialized
    from it's ASN.1 encoding into the supplied service block.

    The function returns FALSE if there was an error in the decoding.
    Such errors are logged.  The service block is "reset" so that it may
    be re-used.
    

BIRD'S EYE VIEW:
    Context:
        The caller is the sending thread in casend_get_msg().  It wants
        to decode the encoded PDU it has acquired into internal data
        structures before submitting those for processing.

    Purpose:
        This function performs the ASN.1 decoding of the PDU into the
        supplied Service Block, handling any errors that may occur.


ACTION SYNOPSIS OR PSEUDOCODE:


    <construct descriptor for varbind list ASN.1 buffer to be parsed from PDU>
    <construct a descriptor for the inbound community name buffer>

    if (attempt to decode the PDU failed)
        <increment count of bad ASN1-parsed PDUs received>
        <log PDU-decode failure>
        if (logging L_ASN1IN)
            <dump the PDU>
        <return FALSE>

    if (version number is invalid)
        <increment count of bad versions received>
        <log badversion>
        if (logging L_ASN1IN)
            <dump the PDU>
        <return FALSE>

    <store community name in service block>

    <initialize pointer to varbind list header in service block>
    <initialize parse_start TRUE>
    <initialize varbind entry count to 1>

    forever
        if (parse of next varbind entry failed)
            if (an avl had been allocated)
		<free the avl>
            if (failed with "End of Sequence")
                if (current entry count is 1)
                    <increment count of bad ASN1-parsed PDUs received>
                    <log PDU decode failure>
                    if (logging L_ASN1IN)
                        <dump the PDU>
                    <return FALSE>
                if (there is another varbind block after the current one)
                    <mark it invalid>
                if (logging L_ASN1IN)
                    if (PDU type matches any requested dump types)
                        <dump the PDU>
                <return TRUE>
            <increment count of bad ASN1-parsed PDUs received>
            <log PDU-decode failure>
            if (logging L_ASN1IN)
                <dump the PDU>
            <return FALSE>

        if (another varbind entry block does not exist on service block list)
            if (storage alloc. attempt for another varbind entry blk failed)
                <CRASH "Mxxx - Memory exhausted">
            <step pointer to current varbind entry block>
            <initialize the varbind entry block storage>
        else
            <step pointer to current varbind entry block>
            if ("in" AVL pointer is non-null)
                if (attempt to free "in" AVL failed)
                    <CRASH "Mxxx - moss error %d on moss_avl_free() call">
            if ("out" AVL pointer is non-null)
                if (attempt to free "out" AVL failed)
                    <CRASH "Mxxx - moss error %d on moss_avl_free() call">
            if (class object identifier has storage associated with it)
                <free old OID storage>
                <show storage NULL>
            if (snmp object identifier has storage associated with it)
                <free old OID storage>
                <show storage NULL>
            if (instance information list is present)
                <release the entire list>

        <load varbind entry id with current correct value>
        <increment varbind entry counter>
        <show parse_start FALSE>
        <step varbind pointer>

OTHER THINGS TO KNOW:

<TC> In function "netio_deserialize_pdu()" . . .
<TC> The code is "threadable" in that no data-structures that need to be
<TC> thread-locked are referenced.

    ASN.1 Dumps of the inbound PDU occur if the user has selected "ASN1IN"
    as an option AND the user has also selected either PDUGETIN, PDUSETIN or
    PDUNXTIN.  If ASN1IN is selected and an error occurs in this function
    while breaking out the varbind list, then a dump is produced regardless
    of any PDUxxxIN options selected.

    The long and the short of the dump scheme is that you only get ASN.1
    dumps for the types of PDUs you are interested in OR if there is an
    error parsing the varbind list.
*/

{
MCC_T_Descriptor  varbind_info;           /* Descriptor for . . .            */
char              varbind[VAR_BIND_SIZE]; /* ASN1 Buffer for varbind data    */

MCC_T_Descriptor  comm_name_info;         /* Descriptor for . . .            */
char              cname_buf[MAX_COMM_NAME_SIZE]; /* Buffer Community Name    */

int               pdu_length;             /* Full integer length of PDU      */
avl               *vb_avl;                /* Temporary storage for avl ptr   */
char              msg[LINEBUFSIZE];       /* Error messages get built here   */
man_status        moss_error_code,        /* to hold AVL return error code   */
                  status_code;            /* to hold moss_avl_free() code    */
unsigned int      mcc_error_code=0;       /* Returned from decode            */
varbind_t         **vb_next;              /* ->pointer to next varbind entry */
varbind_t         *vb_current;            /* ->current varbind entry block   */
int               vb_count;               /* Counter of varbind entries      */
BOOL              parse_start;            /* TRUE: Starting ASN.1 parse      */


pdu_length = pdu_info->mcc_w_curlen; /* (Used in error reporting to sprintf) */

/* construct a descriptor for the varbind list ASN.1 buffer to be parsed */
/* from PDU */
varbind_info.mcc_b_class = DSC_K_CLASS_S;
varbind_info.mcc_b_dtype = DSC_K_DTYPE_T;
varbind_info.mcc_b_flags = 0;
varbind_info.mcc_b_ver = 1;
varbind_info.mcc_w_maxstrlen = VAR_BIND_SIZE;
varbind_info.mcc_w_curlen = 0;
varbind_info.mcc_a_pointer = (unsigned char *) varbind;
varbind_info.mcc_l_id = 0;
varbind_info.mcc_l_dt = 0;

/* construct a descriptor for the inbound community name buffer */
comm_name_info.mcc_w_curlen = 0;
comm_name_info.mcc_w_maxstrlen = MAX_COMM_NAME_SIZE;
comm_name_info.mcc_a_pointer = (unsigned char *) cname_buf;

/*
|  We attempt to bust up the ASN.1-encoded PDU into "C" readable things plus a
|  buffer that contains the raw ASN.1-encoded varbind list, which we deal with
|  separately below.
|
|  The idea is we should be able to "reconstruct" (in ASN.1) the original
|  PDU from the information in the service block in case we need to report an
|  error.  Consequently we save EVERYTHING, even the original error status and
|  index, (even though they'll probably get changed in the event of an error
|  before rebuilding and resending).
*/

moss_error_code = SNMP_DECODE_PDU(
     pdu_info,          /* Descriptor for Buffer containing the PDU to parse */
     &mcc_error_code,   /* Returned MCC error code                           */
                        /* PDU Info:                                         */
     &svc->version,     /* SNMP Protocol Version number                      */
     &comm_name_info,   /* Descriptor for buffer to recv community name      */
  (unsigned int *)  &svc->pdu,  /* PDU type, GET/SET/GETNEXT                 */
     &svc->request_id,  /* Request ID in PDU                                 */
     &svc->error_status,/* Error status value received in PDU                */
     &svc->error_index, /* Error index value received in PDU                 */
     &varbind_info      /* Raw ASN.1 varbind list-->into this buffer         */
     );

/* if (attempt to decode the PDU failed) */
if (moss_error_code != MAN_C_SUCCESS)  {

    /* increment count of bad ASN1-parsed PDUs received */
    bp->statistics->badASN1 += 1;

    /* log PDU-decode failure */
    sprintf(msg,
     MSG(msg047, "M047 - ASN.1 PDU parse failed from %s, size = %d, MCC = %d, PDU ignored"),
            inet_ntoa(svc->inbound.sin_addr),
            pdu_length,
            mcc_error_code);
    SYSLOG( LOG_ERR, msg);

    /* if (logging L_ASN1IN) */
    IFLOGGING(L_ASN1IN) {
        netio_asn1_dump(bp, pdu_info, L_ASN1IN);
        }

    return (FALSE);
    }

/* if (version number is invalid) */
if (svc->version != 0) {
    /* increment count of bad versions received */
    bp->statistics->badvers += 1;

    /* log badversion */
    sprintf(msg,
     MSG(msg198, "M198 - Bad Version %d in PDU from %s, PDU ignored"),
            svc->version,
            inet_ntoa(svc->inbound.sin_addr)
            );
    SYSLOG( LOG_ERR, msg);

    /* if (logging L_ASN1IN) */
    IFLOGGING(L_ASN1IN) {
        netio_asn1_dump(bp, pdu_info, L_ASN1IN);
        }
    return (FALSE);
    }

/* store community name in service block */
svc->comm_namelen = comm_name_info.mcc_w_curlen;
if ((svc->comm_namelen > 0) &&
    (svc->comm_name = (char *) malloc(svc->comm_namelen))
    != NULL) {
    bcopy(cname_buf, svc->comm_name, comm_name_info.mcc_w_curlen);
    }  /* If storage allocation fails, we'll fail soon enough below!  */
       /* No need to log here.                                        */

/*
|  Handle the ASN.1 buffer containing the varbind list here separately.  Each
|  varbind entry is decoded into an AVL residing in a data block of type
|  "varbind_t".  All these blocks are strung on a singly-linked list off of
|  the service block for the pdu.
|
|  Since we re-use service blocks, we also re-use the varbind blocks associated
|  with the service block.  The loop below dumps the old storage associated
|  with data structures in the 'next' varbind block it is about to re-use.  If
|  there are no more varbind blocks to re-use, it allocates a new one from
|  heap storage.
|
|  A varbind block is "valid" only if the "vb_entry_id" cell is non-zero.  The
|  end of the list of valid varbind blocks for this PDU when this function
|  returns is the last "valid" varbind block in the list.  This may be the
|  last in the list, or if the list contains old surplus un-reused blocks,
|  then the first of that series of surplus blocks will have a zero
|  "vb_entry_id" indicating "end of list" encountered.
|
*/

/* initialize pointer to varbind list header in service block */
vb_next = &svc->varbind_list;
vb_current = *vb_next;

/*initialize parse_start to "TRUE" */
parse_start = TRUE;

/*initialize varbind entry count to 1*/
vb_count = 1;

for (;/* ever */;) {

    /* ===========================================
    |  PARSE OFF NEXT VARBIND ENTRY INTO LOCAL AVL
    |============================================= */

    vb_avl = NULL;

    /* if (parse of next varbind entry failed) */
    if ((mcc_error_code = asn1_to_avl(&varbind_info,
                                      &vb_avl,
                                      &moss_error_code,
                                      parse_start
                                      )
        ) != MCC_S_NORMAL) {

        /* if (failed and avl had been allocated then free it) */
        if (vb_avl != NULL) {
           status_code = moss_avl_free(&vb_avl, TRUE);
           }

        /* if (failed with "End of Sequence") */
        /* (We parsed "off the end" of the varbind list) */
        if (mcc_error_code == MCC_S_ILVEOC) {

            /* if (current entry count is 1) */
            /* (This implies a NULL varbind list!) */
            if (vb_count == 1) {

                /* increment count of bad ASN1-parsed PDUs received */
                bp->statistics->badASN1 += 1;

                /* log PDU decode failure */
                sprintf(msg,
     MSG(msg051, "M051 - No varbindlist in PDU from %s, size = %d (decimal), PDU ignored"),
                  inet_ntoa(svc->inbound.sin_addr),
                  pdu_length);
                SYSLOG( LOG_ERR, msg);

                /* if (logging L_ASN1IN) */
                IFLOGGING(L_ASN1IN) {
                    netio_asn1_dump(bp, pdu_info, L_ASN1IN);
                    }

                return (FALSE);
                }

       /* ==========================================
       |  Exit the Outer FOR Loop here, successfully
       |============================================ */
            /* if (there is another varbind block after the current one) */
            if (vb_current != NULL && vb_current->next != NULL ) {
                vb_current->next->vb_entry_id = 0;      /* mark it invalid */
                }

            /* if (logging L_ASN1IN) */
            IFLOGGING(L_ASN1IN) {

                /* if (we're logging any inbound PDUs) */
                if ( (bp->log_state.log_classes & L_PDUSUMIN) != 0) {

                    /* Dump it in ASN.1 also */
                    netio_asn1_dump(bp, pdu_info, L_ASN1IN);
                    }
                }

            return (TRUE);
            }

        /* increment count of bad ASN1-parsed PDUs received */
        bp->statistics->badASN1 += 1;

        /* log PDU-decode failure */
        sprintf(msg, 
MSG(msg052, "M052 - ASN.1 varbindlist parse failed on PDU from %s, size = %d (dec), mcc(%d), moss(%d), PDU ignored"),
                inet_ntoa(svc->inbound.sin_addr),
                pdu_length,
                mcc_error_code,
                (int) moss_error_code
                );
        SYSLOG( LOG_ERR, msg);

        /* if (logging L_ASN1IN) */
        IFLOGGING(L_ASN1IN) {
            netio_asn1_dump(bp, pdu_info, L_ASN1IN);
            }

        return (FALSE);
        }

    /* ====================================================================
    |  GET ANOTHER VARBIND ENTRY BLOCK TO RECEIVE JUST-PARSED VARBIND ENTRY
    |==================================================================== */

    /* if (another varbind entry block does not exist on service block list) */
    if (*vb_next == NULL) {     /* ====== CREATE A NEW ONE ====== */

        /* if (storage alloc. attempt for another varbind entry blk failed) */
        if ((*vb_next = (varbind_t *) malloc(sizeof(varbind_t))) == NULL) {
            CRASH( MSG(msg048, "M048 - Memory exhausted during varbind list parse"));
            }

        /* step pointer to current varbind entry block */
        vb_current = *vb_next;

        /* initialize the varbind entry block storage */
        bzero(vb_current, sizeof(varbind_t));
        }
    else {                      /* ====== RE-USE AN OLD ONE ======*/
        /*step pointer to current varbind entry block*/
        vb_current = *vb_next;

        /* if ("in" AVL pointer is non-null) */
        if (vb_current->in_entry != NULL) {
            /* if (attempt to free "in" AVL failed) */
            if ((moss_error_code = moss_avl_free(&vb_current->in_entry, TRUE))
                != MAN_C_SUCCESS) {
                sprintf(msg, MSG(msg049, "M049 - moss error %d on moss_avl_free() call"),
                        (int) moss_error_code);
                CRASH( msg );
                }
            }

        /* if ("out" AVL pointer is non-null) */
        if (vb_current->out_entry != NULL) {
            /* if (attempt to free "out" AVL failed) */
            if ((moss_error_code = moss_avl_free(&vb_current->out_entry, TRUE))
                != MAN_C_SUCCESS) {
                sprintf(msg, MSG(msg050, "M050 - moss error %d on moss_avl_free() call"),
                        (int) moss_error_code);
                CRASH( msg );
                }
            }

        /* if (class object identifier has storage associated with it) */
        if (vb_current->class_oid.value != NULL) {
            /* free old OID storage */
            free(vb_current->class_oid.value);
            vb_current->class_oid.count = 0;            /* show no arcs      */
            vb_current->class_oid.value = NULL;         /* show storage NULL */
            }

        /* if (snmp object identifier has storage associated with it) */
        if (vb_current->snmp_oid.value != NULL) {
            /* free old OID storage */
            free(vb_current->snmp_oid.value);
            vb_current->snmp_oid.count = 0;             /* show no arcs      */
            vb_current->snmp_oid.value = NULL;          /* show storage NULL */
            }

        /* if (instance information list is present) */
        dump_instance_list(&vb_current->inst_list);
        }

    /* ========================================================
    |  COPY THE VARBIND ENTRY DATA INTO THE VARBIND ENTRY BLOCK
    |========================================================== */

    /*
    |  load varbind entry id with current correct value
    |   (non-zero value makes block "valid")
    |
    |  "vb_count" is the "object class group count" discussed in "snmppe.h" in
    |  the documentation for cell "vb_entry_id".  In V1.0 of this PE, varbind
    |  entries are not grouped by object class, each becomes a separate
    |  request.  Consequently we can just assign it here this way.  In any
    |  event we need a non-zero value in this field for each valid entry block
    |  upon return from this function (so a zero in this field in a surplus
    |  block can mark the end of the list of valid entries).  The value in
    |  vb_entry_id can be overridden later in the processing function
    |  "casend_process_msg()" (by adding code there at a later date) to
    |  group by object class without changing this code here.
    */

    /*                          (bottom 3 bytes)      (bottom 1 byte)        */
    vb_current->vb_entry_id = (svc->svc_count << 8) | (vb_count & 0xFF);

    vb_count += 1;                      /* increment varbind entry counter   */

    vb_current->in_entry = vb_avl;      /* copy avl pointer into entry blk   */

    parse_start = FALSE;                /* 2nd & subsequent: we're not       */
                                        /*                   starting parse  */

    vb_next = &vb_current->next;        /* step next varbind pointer         */
    }
}

/* netio_put_pdu - Put Outbound PDU */
/* netio_put_pdu - Put Outbound PDU */
/* netio_put_pdu - Put Outbound PDU */

void
netio_put_pdu( bp, svc, pdu_info )

big_picture_t    *bp;       /*-> Big Picture Structure for SNMP PE           */
service_t        *svc;      /*-> Service Blk (on Service List) to receive pdu*/
MCC_T_Descriptor *pdu_info; /*-> Descriptor for buffer to recv the PDU image */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is a pointer (to a Service Block) that corresponds to the PDU in the
    inbound (to this function) PDU buffer described by "pdu_info".

    "pdu_info" is the address of a descriptor for a buffer that is to contains
    the outbound PDU image.


OUTPUTS:

    The function returns when the PDU has been successfully sent or any error
    incurred during an attempted send has been handled.
    

BIRD'S EYE VIEW:
    Context:
        The caller is the sending thread. Having received a response from
        the CA, it desires to push out a fully serialized response PDU.

    Purpose:
        This function performs network I/O to send the SNMP GET-RESPONSE PDU.
        Errors are logged.

ACTION SYNOPSIS OR PSEUDOCODE:

    if (attempt to send the pdu failed)
        <SYSLOG
         "Mxxx - GETRESPONSE PDU 'sendto' address(%s) failed, errno = %d "xxx">
    else
        <count another packet sent>

OTHER THINGS TO KNOW:

    This function does not currently acquire a mutex for the port it uses
    to send the PDU.  If code is added in main() to spawn multiple sending
    threads , then this function here may have to be changed to
    acquire mutex "in_m" in Big Picture before trying to do the "sendto".
    See the DCE documntation regarding what I/O functions get jacketed,
    the use of a mutex may not be needed.
*/

{
int             msg_length;          /* Integer to contain PDU length        */
int             status;              /* Status returned from system calls    */
char            msg[LINEBUFSIZE];    /* Error message build buffer           */
extern int      sendto();


msg_length = pdu_info->mcc_w_curlen;
/* if (attempt to send the pdu failed) */
if ((status = sendto(bp->in_socket,             /* Socket               */
                     pdu_info->mcc_a_pointer,   /* Buffer               */
                     msg_length,                /* PDU length in Buffer */
                     0,                         /* flags                */
                     &svc->inbound,             /* Send-to INET address */
                     sizeof(svc->inbound)       /* Size of address      */
                     )) < 0) {

    sprintf(msg,
  MSG(msg053, "M053 - GET-RESPONSE PDU 'sendto %d' address(%s) failed, errno = %d, '%s'"),
            status,
            inet_ntoa(svc->inbound.sin_addr),
            errno,
            strerror(errno));
    SYSLOG(LOG_ERR, msg);
    }
else {
    /* count another packet sent */
    bp->statistics->outpkts += 1;
    }
}

/* netio_serialize_pdu - Serialize Service Block info into ASN.1-encoded PDU */
/* netio_serialize_pdu - Serialize Service Block info into ASN.1-encoded PDU */
/* netio_serialize_pdu - Serialize Service Block info into ASN.1-encoded PDU */

BOOL
netio_serialize_pdu( bp, svc, pdu_info, too_big )

big_picture_t    *bp;       /*-> Big Picture Structure for SNMP PE           */
service_t        *svc;      /*-> Service Blk containing PDU info             */
MCC_T_Descriptor *pdu_info; /*-> Descriptor for buf to rcv encoded PDU image */
BOOL             *too_big;  /*-> FALSE on return if PDU was too big          */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is the address of a Service Block that contains the information
    to be encoded into the PDU.

    "pdu_info" is the address of a descriptor for a buffer that receives the
    PDU image.


OUTPUTS:

    The function returns TRUE when a PDU has been successfully serialized
    (into ASN.1 encoding) using the supplied service block information.

    The function returns FALSE if there was an error in the encoding.
    Such errors are logged.  If the error was due to the PDU being too big to
    encode into the given buffer, then when the function returns FALSE,
    boolean "too_big" is set TRUE.  For all other errors that result in
    a FALSE function return, this boolean is set FALSE.
    

BIRD'S EYE VIEW:
    Context:
        The caller is the sending thread reporting an error, or
        endeavoring to return a response.

    Purpose:
        This function performs the ASN.1 encoding of the PDU using the
        information in the supplied Service Block, handling any errors
        that may occur.


ACTION SYNOPSIS OR PSEUDOCODE:

    <construct a descriptor for the varbind list ASN.1 buffer to be
      encoded into the PDU>
    <initialize pointer to varbind list header in service block>
    <initialize build_start to "TRUE">
    <initialize build_end to "FALSE">

    while (varbind entry blocks remain)

        if (this is the last varbind entry block)
            <build_end = TRUE>

        if (PDU error status is noError)
            <select out_entry AVL for encoding>
        else
            <select in_entry AVL for encoding>

        if (encode of next varbind entry failed)
            if (encode failed due to lack of space)
                <set too_big boolean TRUE>
                <return FALSE>
            <log PDU-encode failure>
            <set too_big boolean to FALSE>
            <return FALSE>

        <show build-starting FALSE>
        <step varbind pointer>

    <construct a descriptor for the community name using data from
      service block>

    if (attempt to encode the PDU failed)
        if (encode failed due to lack of space)
            <set too_big boolean TRUE>
            <return FALSE>
        <log PDU-encode failure>
        <set too_big boolean to FALSE>
        <return FALSE>

    if (we're dumping ASN.1 for these PDUs)
        <construct the line buffer>
        <perform a dump>

    <return TRUE>


OTHER THINGS TO KNOW:

<TC> In function "netio_serialize_pdu()" . . .
<TC> The code is "threadable" in that no data-structures that need to be
<TC> thread-locked are referenced (unless doing an ASN.1 Dump of the PDU
<TC> in which case the log file is locked).

*/

{
MCC_T_Descriptor  varbind_info;           /* Descriptor for . . .            */
char              varbind[VAR_BIND_SIZE]; /* ASN1 Buffer for varbind data    */

MCC_T_Descriptor  comm_name_info;         /* Descriptor for community name   */
avl               *vb_avl;                /* Temporary storage for avl ptr   */
char              msg[LINEBUFSIZE];       /* Error messages get built here   */
man_status        moss_error_code;        /* to hold AVL return error code   */
unsigned int      mcc_error_code;         /* Returned from decode            */
varbind_t         **vb_next;              /* ->pointer to next varbind entry */
varbind_t         *vb_current;            /* ->current varbind entry block   */
BOOL              build_start;            /* TRUE: Varbind build starting    */
BOOL              build_end;              /* TRUE: Varbind build ending      */


/* construct a descriptor for the varbind list ASN.1 buffer to be encoded */
/* into this PDU */
varbind_info.mcc_b_class = DSC_K_CLASS_S;
varbind_info.mcc_b_dtype = DSC_K_DTYPE_T;
varbind_info.mcc_b_flags = 0;
varbind_info.mcc_b_ver = 1;
varbind_info.mcc_w_maxstrlen = VAR_BIND_SIZE;
varbind_info.mcc_w_curlen = 0;
varbind_info.mcc_a_pointer = (unsigned char *) varbind;
varbind_info.mcc_l_id = 0;
varbind_info.mcc_l_dt = 0;

/* initialize pointer to varbind list header in service block */
vb_next = &svc->varbind_list;

build_start = TRUE;     /* initialize build_start to "TRUE" */
build_end = FALSE;      /* initialize build_end to "FALSE" */

/* =======================================================
| FOR EVERY VARBIND ENTRY BLOCK PRESENT FOR THIS PDU . . .
|  ==================================================== */

/* while (varbind entry blocks remain) */
while (*vb_next != NULL && build_end == FALSE) {

    vb_current = *vb_next;      /* Step forward to next varbind entry block */

    /* if (this is the last varbind entry block) */
    if (vb_current->next == NULL || vb_current->next->vb_entry_id == 0) {
        build_end = TRUE;
        }

    /* What is happening here is we are either building the original PDU for return with  */
    /* an error code specified, OR we are building a new PDU for return with (presumably) */
    /* new data in it (or at least an indication that the SET succeeded).                 */

    /* if (PDU error status is noError) */
    if (svc->error_status == (int) noError) {
        vb_avl = vb_current->out_entry;         /* select out_entry AVL for encoding */
        }
    else {
        vb_avl = vb_current->in_entry;          /* select in_entry AVL for encoding */
        }


    /* if (encode of next varbind entry failed) */
    if ((mcc_error_code = avl_to_asn1(vb_avl,           /* AVL we're "reading" from */
                                      &varbind_info,    /* ASN.1 buffer encoding-to */
                                      &moss_error_code, /* Common Agent error code  */
                                      build_start,      /* TRUE: Starting build     */
                                      build_end         /* TRUE: Ending build       */
                                      )
        ) != MCC_S_NORMAL) {

        /* if (encode failed due to lack of space) */
        if (    (mcc_error_code == MCC_S_ILVTOOBIG)
             || (mcc_error_code == MCC_S_INSUF_BUF)
             || (mcc_error_code == MCC_S_ILVBUFTOOBIG)
           ) {
            *too_big = TRUE;    /* set too_big boolean TRUE */
            return (FALSE);
            }

        /* log PDU-encode failure */
        sprintf(msg,MSG(msg054, "M054 - ASN.1 varbindlist encode failed on PDU to %s, mcc(%d), moss(%d), response not generated"),
                inet_ntoa(svc->inbound.sin_addr),
                mcc_error_code,
                (int) moss_error_code
                );
        SYSLOG( LOG_ERR, msg);
        *too_big = FALSE;
        return (FALSE);
        }

    build_start = FALSE;                /* 2nd and subsequent, we aren't starting build */
    vb_next = &vb_current->next;        /* step next varbind pointer                    */
    }


/* construct a descriptor for the inbound community name buffer */
comm_name_info.mcc_w_maxstrlen = comm_name_info.mcc_w_curlen =
  strlen(svc->ok_community->comm_name);
comm_name_info.mcc_a_pointer = (unsigned char *) svc->ok_community->comm_name;

/*
|  Now build the final encoded PDU image using the ASN.1 for the varbind list
|  that was just built above.
*/

/* if (attempt to encode the PDU failed) */
if (SNMP_ENCODE_PDU(pdu_info,          /* Descriptor for Buffer containing the PDU to parse */
                                       /* PDU Info:                                         */
                    &mcc_error_code,   /* if not MAN_C_SUCCESS: error code                  */
                    &svc->version,     /* SNMP Protocol Version number                      */
                    &comm_name_info,   /* Descriptor for buffer to recv community name      */
  (unsigned int *)  &svc->pdu,         /* PDU type, GET/SET/GETNEXT                         */
                    &svc->request_id,  /* Request ID in PDU                                 */
                    &svc->error_status,/* Error status value received in PDU                */
                    &svc->error_index, /* Error index value received in PDU                 */
                    &varbind_info      /* Raw ASN.1 varbind list just built into this buffer*/
                    ) != MAN_C_SUCCESS) {

    /* if (encode failed due to lack of space) */
    if (    (mcc_error_code == MCC_S_ILVTOOBIG)
         || (mcc_error_code == MCC_S_INSUF_BUF)
         || (mcc_error_code == MCC_S_ILVBUFTOOBIG) /* This is a coding error!*/
       ) {
        *too_big = TRUE;    /* set too_big boolean TRUE */
        return (FALSE);
        }

    /* log PDU-encode failure */
    sprintf(msg,
            MSG(msg055, "M055 - ASN.1 encode failed on PDU from %s, response not generated"),
            inet_ntoa(svc->inbound.sin_addr)
            );
    SYSLOG( LOG_ERR, msg);
    *too_big = FALSE;
    return (FALSE);
    }

/* if (we're dumping ASN.1 for these PDUs) */
IFLOGGING(L_ASN1OUT) {
    /* perform a dump */
    netio_asn1_dump(bp, pdu_info, L_ASN1OUT);
    }

return(TRUE);

}

/* netio_send_trap - Issue a Trap to all proper trap communities */
/* netio_send_trap - Issue a Trap to all proper trap communities */
/* netio_send_trap - Issue a Trap to all proper trap communities */

void
netio_send_trap( bp, enterprise_oid, agent_addr_info, trap_type,
                 specific_trap, timestamp, p_varbind_info )
big_picture_t    *bp;                 /*-> Big Picture Structure for SNMP PE */
object_id        *enterprise_oid;     /*-> enterprise oid of trap poster     */
MCC_T_Descriptor *agent_addr_info;    /*-> mcc descriptor containing address */
                                      /*   of trap poster                    */
trap_pdu_type    trap_type;           /* Type of trap to send                */
int              specific_trap;       /* Id code of enterprise-specific trap */
unsigned int     *timestamp;          /*-> time-stamp of trap                */
MCC_T_Descriptor *p_varbind_info;     /*-> mcc descriptor containing         */
                                      /*   varbind list for trap             */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "enterprise_oid" points to an oid for the enterprise sending the trap.
    If this is NULL, the default oid is used.

    "agent_addr_info" points to a mcc descriptor containing the address of
    the trap poster. If this is NULL, the default address on which this PE
    is located is used.

    "trap_type" indicates what kind of trap should be issued.

    "specific_trap" is the id code for the enterprise-specific trap.

    "timestamp" points to a time-stamp for the trap. If this is NULL, the
    time-stamp will be calculated.

    "p_varbind_info" points to a mcc descriptor containing the varbind list.
    This can be NULL.


OUTPUTS:

    This function causes a Trap PDU (of the type specified by the second
    argument) to be created and sent to each community on the trap list.


BIRD'S EYE VIEW:
    Context:
        The caller is a function that has detected a situation requiring
        the issuance of a Trap PDU.

    Purpose:
        This function performs the chore of building the proper Trap PDU
        and then sending it to each Trap List community.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (trap list is empty)
        <return>

    if (trap is AuthenticationTrap AND Authentication traps are NOT ENABLED)
        <return>

    if (caller did not pass enterprise oid)
        <obtain the system's OID (from snmp_stat struct in big picture)>
    if (caller did not pass agent-address)
        <construct descriptor for agent-address>
    if (caller did not pass timestamp)
        <acquire uptime in "ticks" as an unsigned int
                                      word (from snmp_stat struct)>

if MTHREADS
    if (acquire mutex on output port FAILED)
        <write message to syslog>
        <return>
endif

    for (each trap list entry on the trap list)

        <construct descriptor for PDU encode-buffer>
        <construct descriptor for community name>

        if (attempt to build a Trap PDU failed)
            <SYSLOG  "Mxxx - Trap PDU ASN.1 Encode type %d failed code=%d,
                  trap not sent>
            <break>

        <construct the target address socket structure>

        if (PDU send-to attempt failed for current trap list entry)
            <SYSLOG "Mxxx - Trap 'sendto %d' address(%s) failed, errno = %d>
        else
            <count another trap pdu sent>
            IFLOGGING(ASN1TRAP)
               <perform a dump on the buffer>

    <release any dynamically allocated storage>

if MTHREADS
    if (release mutex on output port FAILED)
        <write message to syslog>
        <return>
endif

    <return>

OTHER THINGS TO KNOW:

    The underlying ASN.1 encode routine now supports encoding an optional
    a varbind list into the trap.  This is done for V1.1 to support
    enterprise-specific trap and linkUp and linkDown traps.

    MTHREADS:
    Mutex is shown protecting the outbound port & sendto call.  Careful
    design review may reveal that it is not necessary for proper operation
    in a multiple-sending thread environment.
*/

{
MCC_T_Descriptor  pdu_info;     /*-> Desc for buf to receive serialized PDU  */
char              pdu_buf[MAXPDUSIZE];  /* Bld the SNMP Resp PDU in this buf */
MCC_T_Descriptor  comm_name_info;       /* Descriptor for               */
object_id         sys_oid;              /* System Object Identifier          */
unsigned int      sys_oid_value[20];    /* Arcs for sys_oid                  */
MCC_T_Descriptor  local_agent_addr_info;/* Descriptor for agent address      */
unsigned int      ticks;                /* Time Since bootup in "timeticks"  */
community_t       *tl_entry;            /* -> Next Trap-List Entry           */
char              msg[LINEBUFSIZE];     /* Error messages get built here     */
int               mcc_error;            /* MCC error code returned here      */
int               status;               /* Status returned from system calls */
int               msg_length;           /* Integer to contain PDU length     */
struct sockaddr_in where_to;            /* Build target address for trap here*/
time_t            time_now;             /* structure to hold current time    */
int               i;                    /* loop counter */
extern int        sendto();


/* if (trap list is empty) */
if (bp->trap_list == NULL ) {
    return;
    }

/* if (trap is AuthenticationTrap AND Authentication traps are NOT ENABLED) */
if (trap_type == authenticationFailure && bp->statistics->enableauthtrap != 1)
    return;

/* If the caller did not pass in the enterprise oid, obtain the system's    */
/* OID (stored in the big picture's snmp_stats structure)                   */
if (enterprise_oid == NULL) {
    sys_oid.count = bp->statistics->sysobjid[0];             
    for (i = 0; i < sys_oid.count; i++)
        sys_oid_value[i] = bp->statistics->sysobjid[i+1];

    sys_oid.value = &sys_oid_value[0];
    enterprise_oid = &sys_oid;
    }

/* If the caller did not pass in the agent-address, construct descriptor    */
/* for agent-address                                                        */
if (agent_addr_info == NULL) {
    local_agent_addr_info.mcc_w_maxstrlen = 
    local_agent_addr_info.mcc_w_curlen    = bp->our_addr_length;
    local_agent_addr_info.mcc_a_pointer   =
                               (unsigned char *) &(bp->our_addr.sin_addr);

    agent_addr_info = &local_agent_addr_info;
    }

/* If the caller did not pass in the timestamp, then do the following.     */
/* Acquire uptime in "ticks" as an unsigned int  word. This value is the   */
/* number of ticks (in 100th's of a second) since the net mgmt agent last  */
/* re-initialized.  For a COLD START trap, this value is zero.  For other  */
/* trap types, it is the difference between the current time and the value */
/* stored in the snmp_stats.sysuptime structure (set when the IMOM inits)  */

if (timestamp == NULL) {
    if (trap_type == coldStart)
        ticks = 0L;
    else
    {
        time (&time_now);
        ticks = (int ) ((time_now - bp->statistics->sysuptime) * 100);
    }
}
else
{
    ticks = *timestamp;
}


#ifdef MTHREADS
/* if (acquire mutex on output port FAILED) */
if (pthread_mutex_lock(&bp->out_m) != 0) {
    /* write message to syslog */
    SYSLOG(LOG_ERR, MSG(msg216, "M216-mutex lock failed in netio_send_trap(): %m"));
    return;
    }
#endif


/* for (each trap list entry on the trap list) */
for (tl_entry = bp->trap_list; tl_entry != NULL; tl_entry = tl_entry->next) {

    /* construct descriptor for PDU encode-buffer */
    pdu_info.mcc_b_class     = DSC_K_CLASS_S;
    pdu_info.mcc_b_dtype     = DSC_K_DTYPE_T;
    pdu_info.mcc_b_flags     = 0;
    pdu_info.mcc_b_ver       = 1;
    pdu_info.mcc_w_maxstrlen = MAXPDUSIZE;
    pdu_info.mcc_w_curlen    = 0;
    pdu_info.mcc_a_pointer   = (unsigned char *) pdu_buf;
    pdu_info.mcc_l_id        = 0;
    pdu_info.mcc_l_dt        = 0;

    /* construct descriptor for community name */
    comm_name_info.mcc_w_maxstrlen = 
    comm_name_info.mcc_w_curlen    = strlen(tl_entry->comm_name);
    comm_name_info.mcc_a_pointer   = (unsigned char *) tl_entry->comm_name;
    pdu_info.mcc_l_id = 0;
    pdu_info.mcc_l_dt = 0;

    /* if (attempt to build a Trap PDU failed) */
    if (SNMP_ENCODE_TRAP(&pdu_info,       /* Buffer to build in           */
                         &mcc_error,      /* MCC error code returned here */
                   (int) trap_type,       /* Kind of trap to build        */
                         0,               /* SNMP Version number          */
                         &comm_name_info, /* Community Name by descriptor */
                         enterprise_oid,  /* enterprise Object Identifier */
                         agent_addr_info, /* Agent Network Address        */
                         ticks,           /* UpTime in "ticks"            */
                         specific_trap,   /* enterprise-specific trap id  */
                         p_varbind_info   /* Optional varbind list        */
                         ) != MAN_C_SUCCESS) {
        sprintf(msg,
  MSG(msg218, "M218 - Trap (type %d) PDU ASN.1 encode failed, MCC-code =%d, no trap sent"),
            trap_type,
            mcc_error
            );
        SYSLOG(LOG_ERR, msg);
        break;          /* Break out of the loop, release mutex and exit */
        }

    /* construct the target address socket structure */
    bcopy(&(tl_entry->comm_addr), /* Copy Target addr from trap entry block */
          &where_to.sin_addr,     /* To our local address structure         */
          sizeof(tl_entry->comm_addr));
    where_to.sin_family = AF_INET;

    /* TRAP PDU's are sent out to out_port which is the internal */
    /* representation of port # 162 in /etc/services */
    where_to.sin_port = bp->out_port;

    /* if (PDU send-to attempt failed for current trap list entry) */
    msg_length = pdu_info.mcc_w_curlen;
    if ((status = sendto(bp->out_socket,            /* Socket               */
                         pdu_info.mcc_a_pointer,    /* Buffer               */
                         msg_length,                /* PDU length in Buffer */
                         0,                         /* flags                */
                         &where_to,                 /* Send-to INET address */
                         sizeof(where_to)           /* Size of address      */
                         )) < 0) {
        sprintf(msg,
         MSG(msg219, "M219 - TRAP PDU 'sendto = %d' address(%s) failed, errno = %d, '%s'"),
                status,
                inet_ntoa(where_to.sin_addr),
                errno,
                strerror(errno));
        SYSLOG(LOG_ERR, msg);
        }
    else {
        /* count another trap packet sent */
        bp->statistics->outtraps += 1;

        IFLOGGING( L_ASN1TRAP ) {
            /* perform a dump on the buffer */
            sprintf(msg,
                    "Trap PDU sent to address(%s)",
                    inet_ntoa(where_to.sin_addr)
                    );
            LOG(L_ASN1TRAP, msg);
            netio_asn1_dump(bp, &pdu_info, L_ASN1TRAP);
            }
        }

    } /* for */

/* release any dynamically allocated storage */

#ifdef MTHREADS
/* if (release mutex on output port FAILED) */
if (pthread_mutex_unlock(&bp->out_m) != 0) {
    /* write message to syslog */
    SYSLOG(LOG_ERR, MSG(msg217, "M217-mutex unlock failed in netio_send_trap(): %m"));
    return;
    }
#endif

/* return */

}

/* netio_asn1_dump - Interface to MCC ASN.1 Dump function */
/* netio_asn1_dump - Interface to MCC ASN.1 Dump function */
/* netio_asn1_dump - Interface to MCC ASN.1 Dump function */

static void
netio_asn1_dump(bp, pdu_info, msg_class)

big_picture_t    *bp;       /*-> Big Picture Structure for SNMP PE           */
MCC_T_Descriptor *pdu_info; /*-> Descriptor for buf to rcv encoded PDU image */
int              msg_class; /* Bit Mask corresponding to Message Class       */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "pdu_info" is the address of a descriptor for a buffer that contains the
    PDU image in ASN.1 format to be dumped.

    "msg_class" is the bit mask pattern corresponding to the kind of dump
    being done.  (For SNMP PE V1.0 it'll be either L_ASN1IN or L_ASN1OUT).


OUTPUTS:

    The function causes an interpreted ASN.1 dump to occur on the specified
    buffer into the currently open log file.

    It locks the log file via mutex in a multiply-threaded environment before
    doing I/O into the file.


BIRD'S EYE VIEW:
    Context:
        The caller is a function in this module that needs to dump an
        interpreted reperesentation of a PDU in ASN.1 format.

    Purpose:
        This function performs the overhead stuff of locking the log file
        and building the proper prefix for the dump (into an output line
        buffer) before invoking the MCC function to do the nitty-gritty.


ACTION SYNOPSIS OR PSEUDOCODE:

if MTHREADS
    if (acquire mutex on log file pointer FAILED)
        <write message to syslog>
        <return>
endif


<build ASCII name for message class into local dump line buffer>
<invoke MCC ASN1 dump function>


if MTHREADS
    if (release mutex on log file pointer FAILED)
        <write message to syslog>
        <return>
endif

    

OTHER THINGS TO KNOW:

<TC> In function "netio_asn1_dump()" . . .
<TC> This function acquires the mutex associated with the log file until
<TC> the entire dump has been performed, whereupon the mutex is released.

*/

{
char    dmpbuf[LINEBUFSIZE];    /* Diagnostic Line Buffer               */
int     ss;                     /* Start-String index into dmpbuf[]     */
extern  int mcc_asn1_dump();


#ifdef MTHREADS
/* if (acquire mutex on log file pointer FAILED) */
if (pthread_mutex_lock(&bp->log_state.log_file_m) != 0) {
    /* write message to syslog */
    syslog(LOG_ERR, MSG(msg212, "M212-mutex lock failed in netio_asn1_dump(): %m"));
    return;
    }
#endif


/* build ASCII name for message class into local dump line buffer */
build_line_prefix(&bp->log_state, msg_class, dmpbuf, &ss);

/* invoke MCC ASN1 dump function */
mcc_asn1_dump(pdu_info, bp->log_state.log_file, dmpbuf, ss);


#ifdef MTHREADS
/* if (release mutex on log file pointer FAILED) */
if (pthread_mutex_unlock(&bp->log_state.log_file_m) != 0) {
    /* write message to syslog */
    syslog(LOG_ERR, MSG(msg213, "M213-mutex unlock failed in netio_asn1_dump(): %m"));
    return;
    }
#endif

}
