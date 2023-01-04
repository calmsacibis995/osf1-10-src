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
static char *rcsid = "@(#)$RCSfile: snmp_lib.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:23:40 $";
#endif
/*
**  Copyright (c) Digital Equipment Corporation, 1989, 1990, 
**  1991, 1992.
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
**++
**  FACILITY:
**
**  ABSTRACT:
**
**  ASSUMPTIONS:
**
**  AUTHORS:
**
**    Arundhati Sankar, Networks & Communications Software Engineering
**
**
**  CREATION DATE:      09/01/89
**
**  MODIFICATION HISTORY:
**
** 01	   Nov-1989	MWD	Jacket calls to dump() by #ifdef and 
**				a check of the logical, bit 2 set.
**
** 02	   12-Feb-1990	MWD	Change reference to AM logical name from
**				"SNMP_AM" to "TCPIP_AM".
**
** 03	   29-Mar-1990	MWD	Cleanup debugging.
**
** 04	   20-Jun-1990	MWD	Portability Name Changes
**
** 05	   20-Sep-1990	MWD	Remove the calls to snmp_dump().  It should be
**				called when tranceiving, not when encoding.
**
** 06	   13-Nov-1990	TED	Port to ULTRIX platform.
**
** 07	   04-Feb-1991	MWD	Added the SNMP_DECODE_TRAP routine.
**
**-----------Conversion for use by Common Agent SNMP Protocol Engine--------
**
**         01-Sep-1991  DDB     Conversion to Common Agent. Changes marked
**                              by the string "<CAMOD>" (sans "").
**
**         30-Nov-1992  DEM     Fix ENCODE_TRAP to put EndOfConstructor (EOC)
**                              marker *after* the varbindlist sequence.
**/

/* <CAMOD>

   The main thrust of the changes to this library consist of changing the
   "orientation" of the two high level functions SNMP_ENCODE_REQUEST and
   SNMP_DECODE_REQUEST so that they are not "director"-specific.  Instead
   they become protocol-specific, thereby embracing the fact that the director
   and agent use the SNMP in an asymmetric way, but it's the same format-PDU
   that flies back and forth between director and agent.  Consequently:

            SNMP_ENCODE_REQUEST has become SNMP_ENCODE_PDU . . . and
            SNMP_DECODE_REQUEST has become SNMP_DECODE_PDU.

   The argument list for both is enlarged to include ALL the information
   that needs to be encoded/decoded regardless of whether the user 'cares'
   about any particular PDU 'field'. (For instance, a director doesn't
   care about the value of the error-index field, but nonetheless would have to
   provide a value in the argument list to encode there when this function
   is called, unlike the earlier version).

   Additionally, both of these functions are modified so as to alleviate
   the need to build or parse the varbind list separately.  Instead the
   varbind list is handled directly as an AVL by these functions, the separate
   varbind parsing/building calls being handled automatically.

   Other changes:

      - the "version" argument used by MCC to many of these functions is
        dropped (we don't do runtime binding)

      - the "_mcc" prefix to many functions here is globally pasted to just
        "mcc" to match the actual support routine names.  These means use
        of "asn_entry_points.h" is dropped.

      - the SNMP_DECODE_TRAP function is converted into SNMP_ENCODE_TRAP (by
        a lot of editing).
*/
typedef int BOOLEAN;
typedef int BOOL;

#include "snmppe_descrip.h"

#include "snmppe_interface_def.h"
#include "snmppe_msg.h"
#include "asn_def.h"
#include "snmp_def.h"
#include "snmp_service_def.h"

#include "moss.h"               /* For Support of AVL's */

#include "snmppe_snmplib.h"    /* Function prototypes */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/*************************************************************************/
/* Declare functions called outside of this module.                      */
/*************************************************************************/

extern int           mcc_asn_put_init();
extern int           mcc_asn_put_ref();
extern unsigned int  mcc_asn_tag();
extern int           mcc_asn_length();
extern int           mcc_asn_get_init();
extern int           mcc_asn_get_tag();
extern int           mcc_asn_get_ref();
extern unsigned int  mcc_asn_tag_id();
extern int           mcc_asn_copy_param();
extern unsigned int  mcc_asn_tag_class();




/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  SIDE EFFECTS:
**--
**  The function returns a Common-Agent/Protocol-Engine style return code.
**  If an error that is indicated by an MCC error code occurred, then
**  MAN_C_FAILURE is returned and the caller's cell pointed to by
**  p_mcc_error_code is loaded with the MCC error return.
**/
man_status    SNMP_ENCODE_PDU (	p_request_buff_info,    /* Overhead. . .               */
                                p_mcc_error_code,       /* Return code                 */

				p_snmp_version,         /* Info to be encoded . . .    */
				p_comm_name,
				pdu_type,
				p_request_id,
                                p_error_status,
                                p_error_index,
				p_varbind_info )
/* =================                                                                   */
/* Inbound Arguments                                                                   */
/* =================                                                                   */

MCC_T_Descriptor       	    *p_request_buff_info;       /* descriptor of output buffer */
unsigned int                *p_mcc_error_code;          /* Return code                 */

unsigned int    	    *p_snmp_version;            /* Protocal Version number     */
MCC_T_Descriptor       	    *p_comm_name;               /* descriptor for community    */
unsigned int    	    *pdu_type;                  /* GET, SET, GETNEXT           */
unsigned int    	    *p_request_id;              /* SNMP request id             */
unsigned int    	    *p_error_status;            /* SNMP error status           */
unsigned int    	    *p_error_index;             /* SNMP error index            */
MCC_T_Descriptor            *p_varbind_info;            /* VarBind list in ASN.1 form  */
/* ==================                                                                  */

{
    struct ASNContext 	ctx;                    /* Context blk used by ASN func*/
    int   		        status, switch_index;
    unsigned int 	        tag;
    int			        Type, size, length;

    status = MCC_S_NORMAL;
    switch_index = 1;
    do
	switch (switch_index++)
	/* -----------------------------------------------------------------------------------------------  */
	/*			start switch							            */
	/* -----------------------------------------------------------------------------------------------  */
	{ 
	case  1 :
	/* -----------------------------------------------------------------------------------------------  */
	/*		initialize the output context for request PDU  					    */
	/* -----------------------------------------------------------------------------------------------  */
	    status = mcc_asn_put_init (&ctx, p_request_buff_info);
	    break;
	case  2 :
	/* -------------------------------------------------------------------------------------------  */
	/*		start outer sequence 								*/
	/* ------------------------------------------------------------------------------------------  */
	    Type = tag = MCC_K_ASN_DT_SEQUENCE;
	    status = mcc_asn_put_ref (&ctx, tag, Type, 0, 0);
	    break;
	case 3 :
	/* -----------------------------------------------------------------------------------------------  */
	/*		encode version number 								    */
	/* -----------------------------------------------------------------------------------------------  */
	    Type = tag = MCC_K_ASN_DT_INTEGER;
	    size = 4;
	    status = mcc_asn_put_ref (&ctx, tag, Type, p_snmp_version, size);
	    break;
	case 4 :
	/* -------------------------------------------------------------------------------------------  */
	/*		encode community name 								*/
	/* -------------------------------------------------------------------------------------------  */
	    Type = tag = MCC_K_ASN_DT_OCTETSTRING;
	    size = p_comm_name->mcc_w_curlen;
	    status = mcc_asn_put_ref (&ctx, tag, Type, p_comm_name->mcc_a_pointer, size);
	    break;
	case 5 :
	/* -----------------------------------------------------------------------------------------------  */
	/*		start context [0] for GetRequest PDU 						    */
	/*		or context [1] for GetNextrequest PDU	  					    */
	/*		or context [2] for GetResponse PDU	  					    */
	/*		or context [3] for Setrequest PDU	  					    */
	/* -----------------------------------------------------------------------------------------------  */
	    status = MCC_S_NORMAL;
	    switch ( *pdu_type )
	    {
		case SNMP_K_GETREQ_CONT :
		        tag = mcc_asn_tag ( SNMP_K_GETREQ_CONT, MCC_K_ASN_CL_CONT );
			break;

		case SNMP_K_GETNEXT_CONT :
			tag = mcc_asn_tag ( SNMP_K_GETNEXT_CONT, MCC_K_ASN_CL_CONT );
			break;

		case SNMP_K_GETRES_CONT :
			tag = mcc_asn_tag ( SNMP_K_GETRES_CONT, MCC_K_ASN_CL_CONT );
			break;

		case SNMP_K_SETREQ_CONT :
			tag = mcc_asn_tag ( SNMP_K_SETREQ_CONT, MCC_K_ASN_CL_CONT );
			break;
		default:
			tag = mcc_asn_tag ( SNMP_K_GETREQ_CONT, MCC_K_ASN_CL_CONT );
			break;
		    };
	    Type = - MCC_K_ASN_DT_SEQUENCE;
	    status = mcc_asn_put_ref (&ctx, tag, Type, 0, 0);
	    break;
	case 6 :
	/* -----------------------------------------------------------------------------------------------  */
	/*		encode request id 								    */
	/* -----------------------------------------------------------------------------------------------  */
	    Type = tag = MCC_K_ASN_DT_INTEGER;
	    size = 4;
	    status = mcc_asn_put_ref (&ctx, tag, Type, p_request_id, size);
	    break;
	case 7 :
	/* -------------------------------------------------------------------------------------------  */
	/*		encode error for this pdu                					*/
	/* -------------------------------------------------------------------------------------------  */
	    Type = tag = MCC_K_ASN_DT_INTEGER;
	    size = 4;
	    status = mcc_asn_put_ref (&ctx, tag, Type, p_error_status, size);
	    break;
	case 8 :
	/* -------------------------------------------------------------------------------------------  */
	/*		encode error index for this pdu                 				*/
	/* -------------------------------------------------------------------------------------------  */
	    Type = tag = MCC_K_ASN_DT_INTEGER;
	    size = 4;
	    status = mcc_asn_put_ref (&ctx, tag, Type, p_error_index, size);
	    break;
	case 9 :
	    /* ---------------------------------------------------------------------------------------  */
	    /*		encode VarBindList 								*/
	    /* ---------------------------------------------------------------------------------------  */
	    status = encode_var_bindlist (p_varbind_info,  &ctx );
	    break;
	case 10 :
	/* ---------------------------------------------------------------------------------------------  */
	/*		end the Request PDU								  */
	/* ---------------------------------------------------------------------------------------------  */
	    status = mcc_asn_put_ref ( &ctx, tag, MCC_K_ASN_DT_EOC, 0, 0);
	    break;
	case 11 :
	/* ---------------------------------------------------------------------------------------------  */
	/*		end the outer sequence 								  */
	/* ---------------------------------------------------------------------------------------------  */
	    status = mcc_asn_put_ref ( &ctx, tag, MCC_K_ASN_DT_EOC, 0, 0);
	    break;

	case 12 :
	    length = mcc_asn_length (p_request_buff_info);
	    p_request_buff_info->mcc_w_curlen = length;
	    break;

	} 
    while (  ( switch_index  <= 12 ) && (status == MCC_S_NORMAL));
    /* ------------------------------------------------------------------------------------------  */
    /*			end  switch								   */
    /* ------------------------------------------------------------------------------------------  */

    if (status != MCC_S_NORMAL) {
        *p_mcc_error_code = status;     /* Return the MCC value so we */
        return (MAN_C_FAILURE);         /* can detect buffer overflow */
        }
    else {
        return (MAN_C_SUCCESS);
        }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  SIDE EFFECTS:
**--
**  The function returns a Common-Agent/Protocol-Engine style return code.
**  If an error that is indicated by an MCC error code occurred, then
**  MAN_C_FAILURE is returned by the function.
**/
man_status    SNMP_DECODE_PDU (	p_inbound_buff_info, /* Overhead, , ,      */
                                p_mcc_error_code,    /* MCC return err code*/

				p_snmp_version,      /* Info to be decoded */
				p_comm_name,
				pdu_type,
				p_request_id,
                                p_error_status,
                                p_error_index,
				p_var_bind_info )
/* =================                                                                   */
/* Inbound Arguments                                                                   */
/* =================                                                                   */

MCC_T_Descriptor       	    *p_inbound_buff_info;       /* descriptor of input buffer  */
unsigned int                *p_mcc_error_code;          /* MCC Return code             */

unsigned int    	    *p_snmp_version;            /* Protocal Version number     */
MCC_T_Descriptor       	    *p_comm_name;               /* descriptor for community    */
unsigned int    	    *pdu_type;                  /* GET, SET, GETNEXT           */
unsigned int    	    *p_request_id;              /* SNMP request id             */
unsigned int    	    *p_error_status;            /* SNMP error status           */
unsigned int    	    *p_error_index;             /* SNMP error index            */
MCC_T_Descriptor            *p_var_bind_info;           /* VarBind list buffer descptor*/
/* ==================                                                                  */

{
    struct ASNContext    ctx;
    int 		        status, tag_id, switch_index;
    unsigned int	        tag;
    int		                Type, size, expected_type;
    unsigned int                got_size;


    status = MCC_S_NORMAL;
    switch_index = 1;
    do
	switch (switch_index++) 
	{ /*   start switch */

	case  1 :
	/* -----------------------------------------------------------------------------------------------  */
	/*		initialize the context for response PDU  					    */
	/* -----------------------------------------------------------------------------------------------  */
	    status = mcc_asn_get_init (&ctx, p_inbound_buff_info);
	    break;

	case  2 :
	/* -------------------------------------------------------------------------------------------  */
	/*		get the outer most tag 								*/
	/* -------------------------------------------------------------------------------------------  */
	    status = mcc_asn_get_tag ( &ctx, &tag );
	    break;

	case  3 :
	/* -----------------------------------------------------------------------------------------------  */
	/*		this must be a univ sequence     						    */
	/*		if so open the sequence for further processing					    */
	/* -----------------------------------------------------------------------------------------------  */
	    if ( tag  != MCC_K_ASN_DT_SEQUENCE ) status = MCC_S_INVALID_PDU; 
	    else	{
		    	expected_type =  MCC_K_ASN_DT_SEQUENCE ;
		    	size = 4;
		    	status = mcc_asn_get_ref (	&ctx, 
							expected_type,
							&Type, 
							&tag, 
							size,
							&got_size );
			};
	    break;

	case  4 :
	/* -------------------------------------------------------------------------------------  */
	/*		get the next tag, expect it to be univ [2] for version			  */
	/* -------------------------------------------------------------------------------------  */
	    status  = mcc_asn_get_tag ( &ctx, &tag );
	    break;

	case  5 :
	/* -------------------------------------------------------------------------------------  */
	/*		Acquire the protocol version number            				  */
	/* -------------------------------------------------------------------------------------  */
	    if ( tag  != MCC_K_ASN_DT_INTEGER ) status = MCC_S_INVALID_PDU; 
	    else	{
		    	expected_type =  MCC_K_ASN_DT_INTEGER ;
		    	size = 4;
                        *p_snmp_version = 0;
		    	status = mcc_asn_get_ref (	&ctx, 
							expected_type,
							&Type, 
							p_snmp_version, 
							size,
							&got_size );
			};
	    break;

	case  6 :
	/* -------------------------------------------------------------------------------------  */
	/*		If all OK so far get the next tag for community name           	      	  */
	/* -------------------------------------------------------------------------------------  */
	    status  = mcc_asn_get_tag ( &ctx, &tag );
	    break;

	case  7 :
	/* -------------------------------------------------------------------------------------  */
	/*		This tag must be Univ 4 for community name. If OK get the name		  */
	/* -------------------------------------------------------------------------------------  */
	    if ( tag  != MCC_K_ASN_DT_OCTETSTRING ) status = MCC_S_INVALID_PDU; 
	    else	{
		    	expected_type =  MCC_K_ASN_DT_OCTETSTRING;
		    	size = p_comm_name->mcc_w_maxstrlen;
		    	status = mcc_asn_get_ref (	&ctx, 
							expected_type,
							&Type, 
							p_comm_name->mcc_a_pointer, 
							size,
							&got_size );
                        p_comm_name->mcc_w_curlen = got_size;    /* Return actual size */
			};
	    break;

	case  8 :
	/* -------------------------------------------------------------------------------------  */
	/*		get the next tag for response pdu                          	      	  */
	/* -------------------------------------------------------------------------------------  */
            status  = mcc_asn_get_tag ( &ctx, &tag );
            break;

	case  9 :
	/* -----------------------------------------------------------------------------------  */
	/*		tag should be PDU Type                   				*/
	/* -----------------------------------------------------------------------------------  */
	    tag_id = mcc_asn_tag_id (tag);

            if (   tag_id != SNMP_K_GETREQ_CONT
                && tag_id != SNMP_K_GETNEXT_CONT
                && tag_id != SNMP_K_GETRES_CONT
                && tag_id != SNMP_K_SETREQ_CONT
                                               ) status = MCC_S_INVALID_PDU; 
	    else  {		 
                    *pdu_type = tag_id;
		    expected_type = - MCC_K_ASN_DT_SEQUENCE;
		    size = 4;
		    status = mcc_asn_get_ref (	&ctx, 
						expected_type,
						&Type, 
						&tag, 
						size,
						&got_size );
		};
	    break;

	case 10 :
	/* -------------------------------------------------------------------------------------  */
	/*		decode tag for request id 						  */
	/* -------------------------------------------------------------------------------------  */
	    status  = mcc_asn_get_tag ( &ctx, &tag );
	    break;

	/* -------------------------------------------------------------------------------------  */
	/*		decode request id  							  */
	/* -------------------------------------------------------------------------------------  */
	case 11 :
	    tag_id = mcc_asn_tag_id (tag);
	    if ( tag_id != MCC_K_ASN_DT_INTEGER) status = MCC_S_INVALID_PDU; 
            else        {   expected_type = MCC_K_ASN_DT_INTEGER;        
                            size = 4;                                    
                            status = mcc_asn_get_ref ( &ctx,            
                                                        expected_type,   
                                                        &Type,           
                                                        p_request_id,    
                                                        size,            
                                                        &got_size );                    
                        };                                               
	    break;

	case  12 :
	/* -----------------------------------------------------------------------------------  */
	/*		decode tag for error 							*/
	/* -----------------------------------------------------------------------------------  */
	    status = mcc_asn_get_tag ( &ctx, &tag );
	    break;

	/* -----------------------------------------------------------------------------------  */
	/*		decode error status 							*/
	/* -----------------------------------------------------------------------------------  */
	case  13 :
	    tag_id = mcc_asn_tag_id (tag);
	    if ( tag_id != MCC_K_ASN_DT_INTEGER) status = MCC_S_INVALID_PDU; 
            else        {   expected_type = MCC_K_ASN_DT_INTEGER;       
                            size = 4;                                    
                            status = mcc_asn_get_ref ( &ctx,                    
                                                        expected_type,   
                                                        &Type,           
                                                        p_error_status,
                                                        size,            
                                                        &got_size );     
                        };                                               
                                                                 
	    break;

        case  14 :	
	/* -----------------------------------------------------------------------------------  */
	/*              decode tag for error index						*/
	/* -----------------------------------------------------------------------------------  */
	    status = mcc_asn_get_tag ( &ctx, &tag );
	    break;

	/* -----------------------------------------------------------------------------------  */
	/*		decode error index							*/
	/* -----------------------------------------------------------------------------------  */
	case  15 :
	    tag_id = mcc_asn_tag_id (tag);
	    if ( tag_id != MCC_K_ASN_DT_INTEGER) status = MCC_S_INVALID_PDU; 
            else        {   expected_type = MCC_K_ASN_DT_INTEGER;        
                            size = 4;                                    
                            status = mcc_asn_get_ref ( &ctx,            
                                                        expected_type,   
                                                        &Type,           
                                                        p_error_index,   
                                                        size,            
                                                        &got_size );     
                        };                                               
	    break;

	case  16 :
	/* -----------------------------------------------------------------------------------  */
	/*		decode VarBindList 							*/
	/* -----------------------------------------------------------------------------------  */
	    status = mcc_asn_get_tag ( &ctx, &tag );
	    break;

	case  17 :
	    if ( tag != MCC_K_ASN_DT_SEQUENCE) status = MCC_S_INVALID_PDU; 
	    else {      /* Copy the varbind list into supplied buffer */
                status = decode_var_bindlist (p_var_bind_info, &ctx);
                /* Status may be MCC_S_ILVEOC(52875186) here, if so we must catch it below */
                }
	    break;

	} /*   end switch */
    while (  ( switch_index <= 17 ) && (status == MCC_S_NORMAL));

    if ((status == MCC_S_ILVEOC) || (status == MCC_S_NORMAL)) {
        return (MAN_C_SUCCESS);
        }
    else {
        *p_mcc_error_code = status;
        return (MAN_C_FAILURE);
        }

}


/*************************************************************************
 *
 * Name:	SNMP_ENCODE_TRAP
 *
 * Description:	Encode the data into a standard SNMP Trap PDU. 
 *
 * Input
 * Parameters:  PDU_Desc	Ptr to mcc descriptor of the empty PDU buffer.
 *
 *              p_mcc_error     Ptr to integer to receive MCC error code
 *                              in the event of an encoding error.
 *
 *              trap_type       Kind of trap being generated
 *
 *              snmp_version    Protocol Version number
 *
 *              community       Ptr to descriptor for buffer  containing
 *                              the Community Name.
 *
 *		system_OID      OID for System Identifier
 *
 *		agent_address   Internet Address of SNMP PE's host machine
 *                              (passed via descriptor)
 *
 *              up_timeticks    Time since agent started in "ticks", an
 *                              unsigned number to be encoded with tag
 *                              INET_C_SMI_TIME_TICKS
 *
 *              specific_trap   The id code for the enterprise-specific trap
 *
 *              p_varbind_info  Ptr to mcc descriptor for the varbind list
 *
 * Output
 * Parameters:	None		Updates the PDU_Desc buffer to contain
 *				the ASN.1-encoded Trap PDU image.
 *
 *
 * Returns	"man_status" of MAN_C_SUCCESS or
 *                              MAN_C_FAILURE and any MCC ( ASN ) error status.
 *                              via "p_mcc_error".
 *
 * MCC procedures
 * used:
 *				asn_*
 *
 *************************************************************************
 */

man_status    SNMP_ENCODE_TRAP (PDU_Desc,
                                p_mcc_error,
                                trap_type,
                                snmp_version,
                                community,
                                system_OID,
                                agent_address,
                                up_timeticks,
                                specific_trap,
                                p_varbind_info
                                )

   MCC_T_Descriptor	*PDU_Desc;
   int                  *p_mcc_error;
   int                  trap_type;
   int                  snmp_version;
   MCC_T_Descriptor	*community;
   object_id            *system_OID;
   MCC_T_Descriptor	*agent_address;
   unsigned int         up_timeticks;
   int                  specific_trap;
   MCC_T_Descriptor     *p_varbind_info;

{
	struct	ASNContext ctx;
	int		status, switch_index;
	unsigned int 		tag;
	int			Type, size, length;
        char            pad_unsign[9];  /* Buf to rcv unsigned # for leading */
                                        /* zero pad                          */
        char            *move_from;     /* Ptrs for loading the pad_unsign[] */
        char            *move_to;       /*  buffer above.                    */

	/*
	 ***************************************************************
	 *
	 *  Loop thru the various stages of encoding the
	 *  SNMP Trap.  The format of a Trap PDU SNMP Message is
	 *
	 *  SEQUENCE{
	 *
	 *	version INTEGER{
	 *			version-1 ( 0 )
	 *		       },
	 * 
	 *	community OCTET STRING,
	 *
	 *	data ANY ( PDUs if no authentication )
	 *
	 *	    }
	 *
	 *  So this much is identical to decoding a response PDU, and the rest
	 *  is just expecting different tags in different orders
	 *  ( see SNMP_DECODE_RESPONSE ).
	 *  The Trap PDU looks like
	 *
	 *  IMPLICIT SEQUENCE{
	 *
	 *	enterprise OBJECT IDENTIFIER,
	 *
	 *	agent-address IPAddress,
	 *
	 *	generic-trap INTEGER{
	 *			      coldStart 	( 0 ),
	 *			      warmStart 	( 1 )
	 *			      linkDown 		( 2 ),
	 *			      linkUp 		( 3 ),
	 *			      authFailure 	( 4 ),
	 *			      egpNeighborLoss	( 5 ),
	 *			      enterpriseSpecific( 6 )
	 *			     },
	 *
	 *	specific-trap INTEGER,
	 *
	 *	time-stamp TimeTicks,
	 *
	 *	variable-bindings VarBindList
	 *
	 *		     }
	 *
	 *
	 ***************************************************************
	 */

	status = MCC_S_NORMAL;
	switch_index = 1;
	do
	{
	   switch (switch_index++) 
	   {

	      case 1 :
                /* initialize the output context for PDU buffer */
                status = mcc_asn_put_init (&ctx, PDU_Desc);
                break;


	      case 2 :
                /* start outer sequence for entire PDU */
                Type = tag = MCC_K_ASN_DT_SEQUENCE;
                status = mcc_asn_put_ref (&ctx, tag, Type, 0, 0);
                break;


	      case  3 :
                /* encode version number */
                Type = tag = MCC_K_ASN_DT_INTEGER;
                size = sizeof(int);
                status =
                  mcc_asn_put_ref (&ctx, tag, Type, &snmp_version, size);
                break;


	      case 4 :
                /* encode community name */
                Type = tag = MCC_K_ASN_DT_OCTETSTRING;
                size = community->mcc_w_curlen;
                status =
                  mcc_asn_put_ref(&ctx,tag,Type,community->mcc_a_pointer,size);
                break;


	      /***********************************************************
	       *
	       *  From here on the Trap PDU differs from other SNMP PDUs.
	       *
	       ***********************************************************/
	      case  5 :
                tag = mcc_asn_tag ( SNMP_K_TRAP_CONT, MCC_K_ASN_CL_CONT );
                Type = - MCC_K_ASN_DT_SEQUENCE;
                status = mcc_asn_put_ref (&ctx, tag, Type, 0, 0);
                break;


	      case  6 :
                /* encode the enterprise field, an object identifier */
                tag = Type = MCC_K_ASN_DT_OBJECTID;
                size = sizeof(unsigned int) * system_OID->count;
                status = mcc_asn_put_ref(&ctx,
                                         tag,
                                         Type,
                                         &(system_OID->value[0]),
                                         size
                                         );
                break;


	      case 7 :
                /* encode the agent address as a NetworkAddress */
                tag =  mcc_asn_tag ( APPL_K_IPADDRESS, MCC_K_ASN_CL_APPL );
                Type = - MCC_K_ASN_DT_OCTETSTRING;                
                size = agent_address->mcc_w_curlen;
                status = mcc_asn_put_ref(&ctx,
                                         tag,
                                         Type,
                                         agent_address->mcc_a_pointer,
                                         size);
                break;


	      case 8 :
                /* encode the generic-trap indicator */
                Type = tag = MCC_K_ASN_DT_INTEGER;
                size = sizeof(int);
                status =
                  mcc_asn_put_ref (&ctx, tag, Type, &trap_type, size);
                break;


	      case 9 :
                /* encode the specific-trap code */
                Type = tag = MCC_K_ASN_DT_INTEGER;
                size = sizeof(int);
                status =
                  mcc_asn_put_ref (&ctx, tag, Type, &specific_trap, size);
                break;


	      case 10 :
                /* encode the time-stamp as TimeTicks */
                tag = mcc_asn_tag ( APPL_K_TIMETICKS, MCC_K_ASN_CL_APPL );
                Type = - MCC_K_ASN_DT_INTEGER;  

                pad_unsign[0] = pad_unsign[1] = pad_unsign[2] = pad_unsign[3] =
                pad_unsign[4] = pad_unsign[5] = pad_unsign[6] = pad_unsign[7] =
                pad_unsign[8] = '\0';   /* Fill padding buffer w/zeroes */

                /* move unsigned integer to local storage w/leading zero   */
                move_to = pad_unsign;       /* To padding buffer. . .      */
                move_from = (char *) &up_timeticks; /* From timetick arg . */
                size = sizeof(unsigned int);/* This many bytes             */

                while ( size-- > 0 ) /* Copy the unsigned number over . . .*/
                    *move_to++ = *move_from++;

                size = sizeof(unsigned int ) + 1;       /* Get zero pad */
                status = mcc_asn_put_ref(&ctx, tag, Type, pad_unsign, size);
                break;

	      case 11:

                /* If there is no Varbindlist passed in, start sequence    */
                /* for an empty Varbindlist. Otherwise encode the          */
                /* Varbindlist.                                            */

                if (p_varbind_info == NULL) {
                    Type = tag = MCC_K_ASN_DT_SEQUENCE;
                    status = mcc_asn_put_ref (&ctx, tag, Type, 0, 0);
                    }
                else {
                    status = encode_var_bindlist(p_varbind_info, &ctx);
                    }
                break;

	      case 12:

                /* If there is no Varbindlist passed in, end the Sequence  */
                /* which is the Varbindlist. Otherwise, do nothing.        */

                if (p_varbind_info == NULL) {
                    tag = Type = MCC_K_ASN_DT_EOC;
                    status = mcc_asn_put_ref(&ctx, tag, Type, 0, 0);
                    }
                break;

	      case 13:
                /* End the Sequence which is the Trap-PDU */
                tag = Type = MCC_K_ASN_DT_EOC;
                status = mcc_asn_put_ref(&ctx, tag, Type, 0, 0);
                break;

	      case 14 :
                /* End the Sequence which is the entire PDU */
                tag = Type = MCC_K_ASN_DT_EOC;
                status = mcc_asn_put_ref(&ctx, tag, Type, 0, 0);
                break;

              case 15 :
                /* Fix up the buffer descriptor to reflect what was put in */
                length = mcc_asn_length (PDU_Desc);
                PDU_Desc->mcc_w_curlen = length;
                break;
	   }
	}
	while( ( switch_index <= 15 ) && ( status == MCC_S_NORMAL ) );

    /*
    |  If we blew out of the loop above with anything other than MCC_S_NORMAL,
    |  we need to return that code thru p_mcc_error.
    */
    if (status != MCC_S_NORMAL) {
        *p_mcc_error = status;
        return (MAN_C_FAILURE);
        }

    return( MAN_C_SUCCESS );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  ASSUMPTIONS:
**
**--
**/
var_bind_list_build_init ( p_buff_info )

MCC_T_Descriptor 		    *p_buff_info;

{ /* begin var_bind_list_build_init */
 
struct ASNContext *ctx;
MCC_T_Descriptor buff_desc;
int status, switch_index, tag;
int Type;

    status = MCC_S_NORMAL;
    switch_index = 1;

    do
	switch (switch_index++) 
	{ /*   start switch */
	case  1 :
	    status = get_ctx ( p_buff_info, &ctx, &buff_desc);
	    break;

	case  2 :
	    status = mcc_asn_put_init ( ctx, &buff_desc);
	    break;

	case  3 :
	    tag = Type = MCC_K_ASN_DT_SEQUENCE;  
	    status = mcc_asn_put_ref ( ctx, tag, Type, 0, 0);
	    break;

	} /*   end switch */
    while (  ( switch_index <= 3 ) && (status == MCC_S_NORMAL));
    return (status);


} /* end var_bind_list_build_init    */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  ASSUMPTIONS:
**
**--
**/
var_bind_list_build_end ( p_buff_info )

MCC_T_Descriptor         *p_buff_info;

{ /* begin var_bind_list_build_end */
 
struct ASNContext *ctx;
MCC_T_Descriptor buff_desc;
int status;
int Type;
int tag;
int length;

    status = get_ctx ( p_buff_info, &ctx, &buff_desc);
    if (status == MCC_S_NORMAL) 
	{    tag = Type = MCC_K_ASN_DT_EOC;  
	    status = mcc_asn_put_ref ( ctx, tag, Type, 0, 0);
	
	    length = mcc_asn_length (&buff_desc);
	    p_buff_info->mcc_w_curlen = length + sizeof(*ctx);
	};

    return (status);


} /* end var_bind_list_build_end  */



/* Not Used by SNMPPE */
#if 0
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  ASSUMPTIONS:
**
**--
**/
var_bind_list_put ( p_version, p_buff_info, p_identifier, p_attr_value, snmp_datatype)

MCC_T_Version 		    *p_version;
MCC_T_Descriptor 		    *p_attr_value,
			    *p_buff_info;

unsigned int 	    *p_identifier[], 
			    snmp_datatype;

{
struct 		ASNContext 		*ctx;

MCC_T_Descriptor 		    		buff_desc;

int 				status,
	 				tag_value;

int 					size, 
					Type;

unsigned int  				tag,
					switch_index;

unsigned int 			   	obj_id [400],
					obj_id_index,
					obj_max_index,
					obj_move_index;

char					*move_from, *move_to,
					pad_unsign [9],
                                        *value_buf_ptr;
 
    status = MCC_S_NORMAL;
    switch_index = 1;
    pad_unsign[0] = pad_unsign[1] = pad_unsign[2] = pad_unsign[3] = pad_unsign[4] = 0;
    pad_unsign[5] = pad_unsign[6] = pad_unsign[7] = pad_unsign[8] = 0;
    do
	switch (switch_index++) 
	{ /*   start switch */
	case  1 :
	    status = get_ctx ( p_buff_info, &ctx, &buff_desc );
	    break;

	case  2 :
	/* -----------------------------------------------------------------------------------------------  */
	/* 		start a sequence to encode the attribute id and its value			    */
	/* -----------------------------------------------------------------------------------------------  */
	    tag = Type = MCC_K_ASN_DT_SEQUENCE;  
	    status = mcc_asn_put_ref ( ctx, tag, Type, 0, 0);
	    break;

	case  3 :
	/* -----------------------------------------------------------------------------------------------  */
	/* 		get all the components to build the object Id             			    */
	/* 		start with identifier, then encode the value as an object id                       	    */
	/* -----------------------------------------------------------------------------------------------  */
	    obj_id_index = 0;

	    obj_max_index = (unsigned int )p_identifier[0];
            obj_move_index = 1;
	    while (obj_move_index <= obj_max_index)
		obj_id [ obj_id_index++] = (unsigned int )p_identifier[obj_move_index++];
 		
	    tag = Type = MCC_K_ASN_DT_OBJECTID;  
	    size = obj_id_index * 4;	/* multiply the last used index by 4 to get size of the array in bytes */
	    value_buf_ptr = (char *)(&obj_id[0]);
    	    status = mcc_asn_put_ref ( ctx, tag, Type, value_buf_ptr, size);
	    break;

	case  4 :
 	/* -----------------------------------------------------------------------------------------------  */
	/* 		next encode the value.                                    			    */
	/* 		snmp_datatype decides what gets encoded.                    			    */
	/* -----------------------------------------------------------------------------------------------  */
 	    switch ( snmp_datatype)	/* snmp_datatype detrmines the tag and Type to be used */
		{
                case SNMP_K_DT_INTEGER:                          
		    tag = Type = MCC_K_ASN_DT_INTEGER;  
	    	    size = p_attr_value->mcc_w_curlen;		
	    	    value_buf_ptr = (char *)p_attr_value->mcc_a_pointer;
		    break;			

                case SNMP_K_DT_OCTETSTR:                         /* 4   SNMP data type octet string  */
		    tag = Type = MCC_K_ASN_DT_OCTETSTRING;  
	    	    size = p_attr_value->mcc_w_curlen;		
	    	    value_buf_ptr = (char *)p_attr_value->mcc_a_pointer;
		    break;			

		case SNMP_K_DT_NULL:
		    tag = Type = MCC_K_ASN_DT_NULL;  		/* 5   SNMP data type NULL	     */
	    	    size = p_attr_value->mcc_w_curlen;		
	    	    value_buf_ptr = (char *)p_attr_value->mcc_a_pointer;
		    break;			

                case SNMP_K_DT_OBJECTID:                         /* 6   SNMP data type object Id     */
		    tag = Type = MCC_K_ASN_DT_OBJECTID;  
	    	    size = p_attr_value->mcc_w_curlen;		
	    	    value_buf_ptr = (char *)p_attr_value->mcc_a_pointer;
		    break;			

                case SNMP_K_DT_SEQUENCE:                         /* 16  SNMP data type sequence      */
		    tag = Type = MCC_K_ASN_DT_SEQUENCE;  
	    	    size = p_attr_value->mcc_w_curlen;		
	    	    value_buf_ptr = (char *)p_attr_value->mcc_a_pointer;
		    break;			

                case SNMP_K_DT_IPADDRESS:                        /* 20  SNMP data type IP address    */
		    tag =  mcc_asn_tag ( APPL_K_IPADDRESS, MCC_K_ASN_CL_APPL );
		    Type = - MCC_K_ASN_DT_OCTETSTRING;  
	    	    size = p_attr_value->mcc_w_curlen;		
	    	    value_buf_ptr = (char *)p_attr_value->mcc_a_pointer;
		    break;			

                case SNMP_K_DT_COUNTER:                          /* 21  SNMP data type Counter       */
		    tag =  mcc_asn_tag ( APPL_K_COUNTER, MCC_K_ASN_CL_APPL );
		    Type = - MCC_K_ASN_DT_INTEGER;  
	    	    size = p_attr_value->mcc_w_curlen; 
		    move_from  = (char *)p_attr_value->mcc_a_pointer;
		    move_to = &pad_unsign[0];
		    while ( size-- > 0 )                        /* add leading zero for padding  */
			*move_to++ = *move_from++;
	    	    size = p_attr_value->mcc_w_curlen + 1;	/* this is unsigned number, pad with a leading zero */
	    	    value_buf_ptr = &pad_unsign[0]; 
		    break;			

                case SNMP_K_DT_GAUGE:                            /* 22  SNMP data type Gauge         */
		    tag =  mcc_asn_tag ( APPL_K_GAUGE, MCC_K_ASN_CL_APPL );
		    Type = - MCC_K_ASN_DT_INTEGER;  
	    	    size = p_attr_value->mcc_w_curlen; 
		    move_from  = (char *)p_attr_value->mcc_a_pointer;
		    move_to = &pad_unsign[0];
		    while ( size-- > 0 )                        /* add leading zero for padding  */
			*move_to++ = *move_from++;
	    	    size = p_attr_value->mcc_w_curlen + 1;	/* this is unsigned number, pad with a leading zero */
	    	    value_buf_ptr = &pad_unsign[0]; 
		    break;			

                case SNMP_K_DT_TIMETICKS:                        /* 23  SNMP data type Time Ticks    */
		    tag =  mcc_asn_tag ( APPL_K_TIMETICKS, MCC_K_ASN_CL_APPL );
		    Type = - MCC_K_ASN_DT_INTEGER;  
	    	    size = p_attr_value->mcc_w_curlen; 
		    move_from  = (char *)p_attr_value->mcc_a_pointer;
		    move_to = &pad_unsign[0];
		    while ( size-- > 0 )                        /* add leading zero for padding  */
			*move_to++ = *move_from++;
	    	    size = p_attr_value->mcc_w_curlen + 1;	/* this is unsigned number, pad with a leading zero */
	    	    value_buf_ptr = &pad_unsign[0]; 
		    break;			

                case SNMP_K_DT_OPAQUE:                           /* 24  SNMP data type Opaque        */
		    tag =  mcc_asn_tag ( APPL_K_OPAQUE, MCC_K_ASN_CL_APPL );
		    Type = - MCC_K_ASN_DT_OCTETSTRING;  
	    	    size = p_attr_value->mcc_w_curlen;		
	    	    value_buf_ptr = (char *)p_attr_value->mcc_a_pointer;
		    break;			
		}

    	    status = mcc_asn_put_ref ( ctx, tag, Type, value_buf_ptr, size);
	    break;
	
	case  5 :
	/* -----------------------------------------------------------------------------------------------  */
	/* 		end the sequence                                          			    */
	/* -----------------------------------------------------------------------------------------------  */
	    tag = Type = MCC_K_ASN_DT_EOC;  
	    status = mcc_asn_put_ref ( ctx, tag, Type, 0, 0);
	    break;
	
	} /*   end switch */
    while (  ( switch_index <= 5 ) && (status == MCC_S_NORMAL));

    return (status);
}    
#endif
/* Not Used by SNMPPE */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  ASSUMPTIONS:
**
**--
**/
var_bind_list_parse_init (p_buff_info )

MCC_T_Descriptor       	    *p_buff_info;

{ /* begin var_bind_list_parse_init */
 
struct ASNContext    *ctx;
MCC_T_Descriptor 	    buff_desc;
int 		expected_type, status, switch_index, tag, Type;
unsigned int            got_size;			

    status = MCC_S_NORMAL;
    switch_index = 1;

    do
	switch (switch_index++) 
	{ /*   start switch */
	case  1 :
	    status = get_ctx ( p_buff_info, &ctx, &buff_desc);
	    break;

	case  2 :
	    status = mcc_asn_get_init ( ctx, &buff_desc);
	    break;

	case  3 :
	    status = mcc_asn_get_tag ( ctx, &tag );
	    break;

	case  4 :
	/* -----------------------------------------------------------------------------------------------  */
	/* 		open the outer sequence to decode the attribute id and its value		    */
	/* -----------------------------------------------------------------------------------------------  */
	    expected_type = MCC_K_ASN_DT_SEQUENCE;
	    status = mcc_asn_get_ref ( ctx, expected_type, &Type, 0, 0, &got_size);
	    break;

	} /*   end switch */
    while (  ( switch_index <= 4 ) && (status == MCC_S_NORMAL));

    return (status);

} /* end var_bind_list_parse_init    */


/* Not Used by SNMPPE */
#if 0
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  ASSUMPTIONS:
**
**--
**/
var_bind_list_get ( p_version, p_buff_info, p_identifier, p_attr_value, p_snmp_datatype)

MCC_T_Version 		    *p_version;
MCC_T_Descriptor 		    *p_attr_value,
			    *p_buff_info;

unsigned int 	    *p_identifier[], 
			    *p_snmp_datatype;

{ /* begin var_bind_list_get */

struct ASNContext 	*ctx;

MCC_T_Descriptor 		buff_desc;

int 		expected_type,	
			size,	
			
			status, switch_index, 
			tag_value, tag_class, tag_form, tag_id, Type;

unsigned int  		got_size,
			limit_size,
			tag;

unsigned int 		obj_id [400],
			obj_id_index,
			obj_max_index,
			obj_move_index;

char			*move_from, *move_to,
			pad_unsign [9],
                        *value_buf_ptr;

    status = MCC_S_NORMAL;
    switch_index = 1;

    do
	switch (switch_index++) 
	{ /*   start switch */
	case  1 :
	    status = get_ctx ( p_buff_info, &ctx, &buff_desc);
	    break;

	case  2 :
	/* -----------------------------------------------------------------------------------------------  */
	/* 		open the inner sequence to get the attribute id and its value		    	    */
	/* -----------------------------------------------------------------------------------------------  */
	    status = mcc_asn_get_tag ( ctx, &tag );
	    break;

	case  4 :
	    expected_type = MCC_K_ASN_DT_SEQUENCE;  
	    status = mcc_asn_get_ref ( ctx, 
				    	expected_type, 
					&Type, 
				    	&tag_value,
				    	size,
				    	&got_size);
	    break;

	case  5 :
	    /* -------------------------------------------------------------------------------------------  */
	    /*	    First thing in this sequence is object Id of the arrtibute.  get it.           	    */
	    /* -------------------------------------------------------------------------------------------  */
       	    status = mcc_asn_get_tag ( ctx, &tag );
	    break;

	case  6 :
	    if ( tag == MCC_K_ASN_DT_OBJECTID )
	    	{
	    	expected_type = MCC_K_ASN_DT_OBJECTID;
                /* multiply by sizeof int to get the size of int array of 400 in bytes */
/*		size = 400 << 2; */
                size = 400 * sizeof(int);
    		status = mcc_asn_get_ref (ctx, 
					  expected_type, 
					  &Type, 
					  &obj_id[0],  
					  size, 
					  &got_size);

		}
 	    else status = MCC_S_INVALID_PDU;
	    break;

	case  7 :
	    /* -------------------------------------------------------------------------------------------  */
	    /*	    Now move the required required number of entries in different arrays           	    */
	    /* -------------------------------------------------------------------------------------------  */
	    obj_id_index = 0;
            /* divide the byte size by sizeof(int) to get integer array size  */
/*	    got_size = got_size >> 2;	*/
            got_size = got_size / sizeof(int);

	    obj_max_index = (unsigned int )p_identifier[0];
            obj_move_index = 1;
	    while (obj_move_index <= obj_max_index && obj_id_index < got_size )
		p_identifier [obj_move_index++] = (unsigned int *)obj_id [ obj_id_index++];
	    /* -----------------------------------------------------------------------  */
	    /* If we have less number of objec id components than requested,            */
	    /* return # integers moved which is one less than obj_move_index            */
	    /* which points to the next index to be moved 				*/ 
	    /* -----------------------------------------------------------------------  */
	    if ( obj_id_index == got_size ) 
		p_identifier[0] = (unsigned int  *)(obj_move_index - 1); 

  	    break;

	case  8 :
	/* -----------------------------------------------------------------------------------------------  */
	/* 		next decode the value.                                    			    */
	/* 		the tag tells us the snmp_datatype                                                  */
	/*              helps decide how to decode it encoded.                    			    */
	/* -----------------------------------------------------------------------------------------------  */
	    status = mcc_asn_get_tag ( ctx, &tag );
 	    break;

	case  9 :
    	    tag_class = mcc_asn_tag_class (tag);
	    if ( tag_class == MCC_K_ASN_CL_APPL )
		{	/* begin to handle application data types  */
		tag_id = mcc_asn_tag_id (tag);
		switch ( tag_id )
		{
                case APPL_K_IPADDRESS:		/* 20  SNMP data type IP address    */
			*p_snmp_datatype = SNMP_K_DT_IPADDRESS;
		    	expected_type = - MCC_K_ASN_DT_OCTETSTRING;  
		    	break;			

                case APPL_K_COUNTER: 		/* 21  SNMP data type Counter       */
			*p_snmp_datatype = SNMP_K_DT_COUNTER;
		    	expected_type = - MCC_K_ASN_DT_INTEGER;  

			/* max length allowed, needed to trim padded leading zero */
	    	    	if (p_attr_value->mcc_w_maxstrlen >= 4) 
                            limit_size = 4;	/* counter = 32 bit unsig int*/ 
			else
                            limit_size = p_attr_value->mcc_w_maxstrlen;

			/* initialize  the buffer		*/
			size = limit_size;		
		    	move_to  = (char *)p_attr_value->mcc_a_pointer;
		    	while ( size-- > 0 )  *move_to++ = 0;
		    	break;			

                case APPL_K_GAUGE: 		/* 22  SNMP data type Gauge         */
		    	*p_snmp_datatype = SNMP_K_DT_GAUGE;
		        expected_type = - MCC_K_ASN_DT_INTEGER;  
			/* max length allowed, needed to trim padded leading zero */
	    	    	if (p_attr_value->mcc_w_maxstrlen >= 4) 
                            limit_size = 4;	/* gauge = 32 bit unsign int */ 
			else
                            limit_size = p_attr_value->mcc_w_maxstrlen;

			/* initialize  the buffer		*/
			size = limit_size;		
		    	move_to  = (char *)p_attr_value->mcc_a_pointer;
		    	while ( size-- > 0 )  *move_to++ = 0;
		    	break;			

                case APPL_K_TIMETICKS: 			/* 23  SNMP data type Time Ticks    */
		    	*p_snmp_datatype = SNMP_K_DT_TIMETICKS;
		        expected_type = - MCC_K_ASN_DT_INTEGER;  
	    	    	if (p_attr_value->mcc_w_maxstrlen >= 8)
                            limit_size = 8;	/* time = 64 bit unsign int */ 
			else
                            limit_size = p_attr_value->mcc_w_curlen;

			/* initialize  the buffer		*/
			size = limit_size;		
		    	move_to  = (char *)p_attr_value->mcc_a_pointer;
		    	while ( size-- > 0 )  *move_to++ = 0;
		    	break;			

                default:		/* 24  SNMP data type Opaque        */
		    	*p_snmp_datatype =  SNMP_K_DT_OPAQUE;                            
		    	expected_type = - MCC_K_ASN_DT_OCTETSTRING;  
		    	break;			
		};	/* end of switch */

		}	/* end  of application data types  */
	    else
		{	/* begin to handle Universal data types  */
		switch ( tag )
		{
		case MCC_K_ASN_DT_INTEGER:		/* 2   SNMP data type integer       */
			*p_snmp_datatype = SNMP_K_DT_INTEGER;
	    		expected_type = MCC_K_ASN_DT_INTEGER; 
			break;

                case MCC_K_ASN_DT_OCTETSTRING:		/* 4   SNMP data type octet string  */
			*p_snmp_datatype = SNMP_K_DT_OCTETSTR;
	    		expected_type = MCC_K_ASN_DT_OCTETSTRING;  
			break;

		case MCC_K_ASN_DT_NULL:			/* 5   SNMP data type Null  */
			*p_snmp_datatype = SNMP_K_DT_NULL;
	    		expected_type = MCC_K_ASN_DT_NULL;
			break;

                case MCC_K_ASN_DT_OBJECTID:		/* 6   SNMP data type object Id     */
			*p_snmp_datatype = SNMP_K_DT_OBJECTID;                       
		    	expected_type = MCC_K_ASN_DT_OBJECTID;  
			break;

                default: 				/* 16  SNMP data type sequence      */
			*p_snmp_datatype = SNMP_K_DT_SEQUENCE;                         
		    	expected_type = MCC_K_ASN_DT_SEQUENCE;  
			break;

		};	/* end switch for univ data types  */
		};	/* end  to handle Universal data types  */
	    size = p_attr_value->mcc_w_maxstrlen;		
	    value_buf_ptr = (char *)p_attr_value->mcc_a_pointer;
	    status = mcc_asn_get_ref (	ctx, 
					expected_type, 
					&Type, 
					value_buf_ptr,  
					size, 
					&got_size);

	    break;
	case  10 :

	/* -----------------------------------------------------------------------------------------------  */
	/* 		make sure this is the end of this sequence                     			    */
	/* -----------------------------------------------------------------------------------------------  */
	    if ( expected_type == - MCC_K_ASN_DT_INTEGER )
		   p_attr_value->mcc_w_curlen = limit_size;
	    else   p_attr_value->mcc_w_curlen = got_size;

	    status = mcc_asn_get_tag ( ctx, &tag );
	    if (status == MCC_S_ILVEOC ) status = MCC_S_NORMAL;
	    break;

	} /*   end switch */
    while ( ( switch_index <=  10) && (status == MCC_S_NORMAL));

    return (status);

} /* end var_bind_list_get */
#endif
/* Not Used by SNMPPE */

int	encode_var_bindlist ( p_var_bindlist_info, p_ctx)

MCC_T_Descriptor 		    *p_var_bindlist_info;
struct	ASNContext	    *p_ctx; 

{
    struct ASNContext	    *src_ctx;
    MCC_T_Descriptor 		    src_buff_desc;
    int			    status,  switch_index;
    unsigned int 		    tag;


    status = MCC_S_NORMAL;
    switch_index = 1;
    do
	switch (switch_index++)
	/* --------------------------------------------------------------------------------  */
	/*			start switch						     */
	/* --------------------------------------------------------------------------------  */
	{				
	case 1 :
	    status = get_ctx ( p_var_bindlist_info, &src_ctx, &src_buff_desc);
	    break;
	case 2 :
	    status = mcc_asn_get_init (src_ctx, &src_buff_desc);
	    break;
	case 3 :
	    status = mcc_asn_get_tag (src_ctx, &tag);
	    break;
	case 4 :
	    status = mcc_asn_copy_param ( src_ctx, p_ctx );
	    break;

	} 
    while (  ( switch_index <= 4) && (status == MCC_S_NORMAL));
    /* ------------------------------------------------------------------------------------------  */
    /*			end  switch								   */
    /* ------------------------------------------------------------------------------------------  */

    return (status);
}

int	decode_var_bindlist ( p_var_bindlist_info, p_ctx)

MCC_T_Descriptor 		    *p_var_bindlist_info;
struct	ASNContext	    *p_ctx; 

{
    struct ASNContext	    *dst_ctx;
    MCC_T_Descriptor 		    dst_buff_desc;
    int			    status, switch_index;
    int				    length;


    status = MCC_S_NORMAL;
    switch_index = 1;
    do
	switch (switch_index++)
	/* --------------------------------------------------------------------------------  */
	/*			start switch						     */
	/* --------------------------------------------------------------------------------  */
	{				
	case 1 :
	    status = get_ctx ( p_var_bindlist_info, &dst_ctx, &dst_buff_desc);
	    break;
	case 2 :
	    status = mcc_asn_put_init (dst_ctx, &dst_buff_desc);
	    break;
	case 3 :
	    status = mcc_asn_copy_param ( p_ctx, dst_ctx );
	    break;
	case 4 :
	    length = mcc_asn_length (&dst_buff_desc);
	    p_var_bindlist_info->mcc_w_curlen = length + sizeof(*dst_ctx);
	    break;

	} 
    while (  ( switch_index <= 5) && (status == MCC_S_NORMAL));
    /* ----------------------------------------------------------------------------------  */
    /*			end  switch							   */
    /* ----------------------------------------------------------------------------------  */

    return (status);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  ASSUMPTIONS:
**
**--
**/
int get_ctx ( p_buff_info, pp_ctx, p_buff_desc )
MCC_T_Descriptor 		        *p_buff_info;
struct 	ASNContext 		**pp_ctx;
MCC_T_Descriptor 		        *p_buff_desc;
{
int min_length;

    min_length = sizeof ( **pp_ctx )  + 20; /* 2 * MCC_K_ASN_SAFETY_MARGIN */
    if (p_buff_info->mcc_w_maxstrlen < min_length ) return (MCC_S_ILVBUFTOOBIG);
/*###1175 [cc] warning: illegal pointer combination%%%*/
    *pp_ctx = (struct ASNContext *)p_buff_info->mcc_a_pointer;
    p_buff_desc->mcc_w_maxstrlen = p_buff_info->mcc_w_maxstrlen - sizeof(**pp_ctx);
    p_buff_desc->mcc_a_pointer = p_buff_info->mcc_a_pointer + sizeof(**pp_ctx) ;
    p_buff_desc->mcc_b_dtype = DSC_K_DTYPE_T;
    p_buff_desc->mcc_b_class = DSC_K_CLASS_S;
    return (MCC_S_NORMAL);
}

#if 0
int snmp_dump ( p_buff_info )
MCC_T_Descriptor 		    *p_buff_info;

{ /* begin snmp_dump */
 

int status;
/* MCC_T_Descriptor mcc_desc;                   */
/*        status = Dump_SNMP (type);		*/
/*	  if (status == 1)			*/
/*		mcc_desc.mcc_w_maxstrlen = p_buff_info->mcc_w_maxstrlen;
 *		mcc_desc.mcc_b_dtype = DSC_K_DTYPE_T;
 *		mcc_desc.mcc_b_class = DSC_K_CLASS_S;
 *		mcc_desc.mcc_w_curlen =  p_buff_info->mcc_w_curlen;
 *		mcc_desc.mcc_a_pointer = p_buff_info->mcc_a_pointer;
 *		status = mcc_asn_dump (&mcc_desc);
 */
 		status = mcc_asn_dump (p_buff_info);
	  return (status);
} /* end snmp_dump    */
#endif

/* avl_to_asn1 - Encode AVL from Varbind List into ASN.1 */
/* avl_to_asn1 - Encode AVL from Varbind List into ASN.1 */
/* avl_to_asn1 - Encode AVL from Varbind List into ASN.1 */

int avl_to_asn1(p_var_bind_avl, p_varbind_info, p_moss_error, build_start,
                build_end )

avl              *p_var_bind_avl;  /* ->AVL containing the varbind list info */
MCC_T_Descriptor *p_varbind_info;  /* ->MCC descptr for buffer to rcv ASN.1  */
man_status       *p_moss_error;    /* ->man_status to rcv MOSS error code    */
BOOL             build_start;      /* TRUE: Starting Build on this call      */
BOOL             build_end;        /* TRUE: Ending Build on this call        */

/*
INPUTS:

    "p_var_bind_avl" is a pointer to the AVL containing the varbind list
    information

    "p_varbind_info" is a pointer to an MCC descriptor describing the buffer
    to receive the ASN.1 encoding of the contents of the inbound AVL.

    "p_moss_error" is a pointer to a "man_status" cell to receive the MOSS
    error code should this function return the MCC code "MCC_S_FAILED".

    "build_start" and "build_end" serve to indicate to this function whether
    or not to start the outer-most sequence (surrounding the varbind list)
    and/or to end the outer-most sequence.  If both are TRUE, then the varbind
    list has only one entry  (this function is being called only once).

OUTPUTS:

    On success, the function returns MCC_S_NORMAL if the ASN.1 encoding
    succeeded, otherwise:

       * If the error occurred in the supporting MCC ASN.1 functions, that
         function's MCC error return code is returned by this function

       * If the error occurred in an AVL (MOSS) routine, then this function
         returns the MCC code "MCC_S_FAILED" (which is not used by the
         supporting MCC ASN.1 functions) and also returns the MOSS error code
         to the caller of this function.


BIRD'S EYE VIEW:
    Context:
        The caller is the caller SNMP_ENCODE_PDU (found in this module), and
        it has an AVL containing varbind list information that it needs
        encoded into ASN.1.

    Purpose:
        This function uses the SNMP_LIB functions to build an ASN.1
        representation of the contents of the inbound AVL (using MOSS
        functions to read it) in the passed buffer.


ACTION SYNOPSIS OR PSEUDOCODE:

    <reset the AVL to the beginning>
    if ( build_start is TRUE)
        if (initialization of the buffer to receive the encoding failed)
            <return MCC error code>                (var_bind_list_build_init)

    if (get of context block failed)
        <return MCC error code>

    if ("point to next AVL element" failed)
        <return MOSS error>
        <return MCC_S_FAILED>

    <extract tag and class from AVL tag value>
    if (AVL tag class shows CONTEXT_SPECIFIC or PRIVATE or CONSTRUCTED)
        <return MOSS "MAN_C_NOT_SUPPORTED" as MOSS error>
        <return MCC_S_FAILED>

    if (attempt to start a sequence to encode OID and value failed)
        <return MCC error code>

    <construct calling sequence for encoding object id>
    if (attempt to put_ref the object id value failed)
        <return MCC error code>

    if (AVL tag class shows "Universal")
        <construct put_ref calling sequence values for tag, Type, value size
         and value addr using AVL tag and octet_string datatype value>
    else
        (* Assume if not Universal, then Application *)
        <construct put_ref "tag" value from extracted AVL tag + APPLICATION>
        <construct IMPLICIT Type value from octet_string low-order value>
        switch (AVL extracted tag value)
            case APPL_K_COUNTER:
            case APPL_K_GAUGE:
            case APPL_K_TIMETICKS:
                <move data from octet_string to local storage w/leading zero>
                <set pointer to data for put_ref to local storage>
                <set size from octet string length + 1 character pad>
                break;

            case APPL_K_IPADDRESS :
            case APPL_K_OPAQUE :
            default:
                <set pointer to data for put_ref to octet string value>
                <set size from octet string length>
                break;

    if (attempt to put_ref given value failed)
        <return MCC error code>

    if (attempt to end the sequence to encode OID and value failed)
        <return MCC error code>

    if (building end is TRUE)
        <close off this build w/call to var_bind_list_build_end>
        if (close failed)
            <return MCC error code>

    <return MCC_S_NORMAL>


OTHER THINGS TO KNOW:

    This function replaces (for our purposes) the SNMP_LIB function named
    "var_bind_list_put()".

    This function endeavors to hide the MOSS routines that manipulate the AVL
    from the rest of the code in this module.  Any errors that occur are
    simply passed upward via the "p_moss_error" argument as described under
    OUTPUTS (above).  They'll get logged eventually.

    SNMPPE doesn't expect to have to handle AVL's containing constructed
    values, nor elements whose tags are for CONTEXT-SPECIFIC or PRIVATE,
    since these don't occur in SNMP land.

    If there is an APPLICATION tag for which we don't have code to recognize
    the tag value, we do the best we can ("default" in the Application
    "switch" statement).

*/


{
struct ASNContext *ctx;  /* ->Context Block used by MCC_ASN functionn */
MCC_T_Descriptor buff_info;     /* Buff descptr rtned by get_ctx: not used   */
int             size;           /* size of data to be submitted to _put_ref  */
int             Type;           /* MCC "type" value, neg-signed = IMPLICIT,  */
                                /*  for _put_ref                             */
unsigned int    tag;            /* MCC encoded tag value, for _put_ref       */
char            *value_buf_ptr; /* MCC pointer to data to be ASN.1ed         */

object_id       *oid;           /* --> Object ID of AVL element              */
unsigned int    modifier;       /* AVL Modifier cell for AVL element         */
unsigned int    avl_tag;        /* AVL element's full tag value              */
unsigned int    avl_t_class;    /* AVL element's tag's ASN.1 class           */
                                /*               (low-order 29 bits = 0)     */
unsigned int    avl_t_value;    /* AVL element's tag's ASN.1 value           */
                                /*               (hi-order 3 bits = 0)       */
octet_string    *octet;         /* -->AVL element's octet string             */

man_status      moss_status;    /* Rtn code from AVL routines->rtn to caller */
int             mcc_status;     /* Rtn codes from MCC funcs->rtn to caller   */
int             last_one;       /* AVL flag: "end of the line reached"       */

char            pad_unsign[9];  /* Buf to rcv unsigned # for leading zero pad*/
char            *move_from;     /* Ptrs for loading the pad_unsign[] buffer  */
char            *move_to;       /*  above.                                   */


/* reset the AVL to the beginning */
if ((moss_status = moss_avl_reset(p_var_bind_avl)) != MAN_C_SUCCESS) {
    *p_moss_error = moss_status;        /* Return this code   */
    return (MCC_S_FAILED);             /* Signal that we did */
    }

/* if (building start is TRUE) */
if (build_start == TRUE) {
    /* if (initialization of the buffer to receive the encoding failed) */

    /* (This also puts in a "Start Sequence" to demark the start of the
    |   varbind list sequence)
    */
    if ((mcc_status = var_bind_list_build_init( p_varbind_info )) != MCC_S_NORMAL) {
        return (mcc_status);        /* return MCC error code */
        }
    }

/* if (get of context block failed) */
if ((mcc_status = get_ctx (p_varbind_info, &ctx, &buff_info)) != MCC_S_NORMAL) {
    return (mcc_status);        /* return MCC error code */
    }

/*
|  Copy the AVL element into ASN.1 form via the appropriate MCC function.
|  The AVL elements may not be constructed (not supported by SNMP) and must
|  have tags of class UNIVERSAL or APPLICATION.
*/

/* if ("point to next AVL element" failed) */
if ((moss_status = moss_avl_point(p_var_bind_avl,   /* AVL inbound          */
                                  &oid,             /* OID of next element  */
                                  &modifier,        /* Modifier of element  */
                                  &avl_tag,         /* Tag of element       */
                                  &octet,           /* Value of element     */
                                  &last_one )) != MAN_C_SUCCESS) {
    *p_moss_error = moss_status;        /* Return this code   */
    return (MCC_S_FAILED);              /* Signal that we did */
    }

/* extract tag and class from AVL tag value */
avl_t_class = avl_tag & 0xc0000000; /* Only the hi-order 3 bits   */
avl_t_value = avl_tag & 0x1fffffff; /* Only the low-order 28 bits */

/* if (AVL tag class shows CONTEXT_SPECIFIC or PRIVATE or CONSTRUCTED)    */
/* (We're rejecting the stuff we can't handle here, we shouldn't have to) */
if (   (CONTEXT_SPECIFIC(0) == avl_t_class)
    || (PRIVATE(0) == avl_t_class)
    || IS_CONSTRUCTED(avl_tag) ) {

    /* return MOSS "MAN_C_NOT_SUPPORTED" as MOSS error */
    *p_moss_error = MAN_C_NOT_SUPPORTED;
    return (MCC_S_FAILED);         /* return MCC_S_FAILED */
    }

/* if (attempt to start a sequence to encode OID and value failed) */
tag = Type = MCC_K_ASN_DT_SEQUENCE;
if ((mcc_status = mcc_asn_put_ref( ctx, tag, Type, 0, 0)) != MCC_S_NORMAL) {
    return (mcc_status);    /* return MCC error code */
    }

/* construct calling sequence for encoding object id */
tag = Type = MCC_K_ASN_DT_OBJECTID;
size = sizeof(unsigned int) * oid->count;

/* if (attempt to put_ref the object id value failed) */
if ((mcc_status = mcc_asn_put_ref( ctx, tag, Type, &oid->value[0], size))
    != MCC_S_NORMAL) {
    return (mcc_status);    /* return MCC error code */
    }

/* if (AVL tag class shows "Universal") */
if (avl_t_class == UNIVERSAL(0)) {
    /* construct put_ref calling sequence values for tag, Type, value size */
    /* and value addr using AVL tag and octet_string datatype value        */
    /*
    | Here's whats going on here for tag & Type:
    |
    |       For a UNIVERSAL (primitive value), the mcc_asn_put_ref() function
    |       expects both of these arguments to be the same, and to be the value
    |       of a predefined constant of the form MCC_K_ASN_DT_<universal-type>,
    |       as in
    |
    |               MCC_K_ASN_DT_INTEGER or MCC_K_ASN_DT_OCTETSTRING
    |
    |       The values of these constants are found in header file
    |       mcc_interface_def.h and they are simply the ASN.1 constants
    |       assigned to specify these primitive types:
    |               #define MCC_K_ASN_DT_INTEGER  2
    |               #define MCC_K_ASN_DT_OCTETSTRING 4
    |
    |       Consequently, all we do is take the AVL tag (low-order) value and
    |       assign it to both "tag" and "Type"
    */
    tag = Type = avl_t_value;

    /*
    |  The value and it's length are simply extracted directly from the
    |  octet_string portion of the AVL in this instance, no transformation
    |  required.
    */
    value_buf_ptr = octet->string;
    size = octet->length;
    }
else {  /* At this point, we can assume if not UNIVERSAL, then APPLICATION */

    /* construct put_ref "tag" value from extracted AVL tag + APPLICATION
    |
    |  The mcc_asn_tag() call here actually computes the ASN.1 "identifier
    |  octet(s)" as they should be laid down in the outgoing buffer (up to
    |  four bytes long, obviously).
    |  In the UNIVERSAL C-code above, the identifier octet just looks like the
    |  tag number of the primitive type, since the hi-bits are clear.  Here
    |  we have to "compute" it.
    */
    tag = mcc_asn_tag (avl_t_value, MCC_K_ASN_CL_APPL);

    /* construct IMPLICIT Type value from octet_string data_type value
    |
    |  The mcc_asn_put_ref() code, when called with Type != tag, examines the
    |  sign of the Type value to see whether it should encode as IMPLICIT.
    |  For APPLICATION it should be, so we take the primitive value of the
    |  octet_string datatype (which should be an ASN.1 primitive datatype
    |  value) and negate it, signalling "IMPLICIT".
    */
    Type = - octet->data_type;

    /* Now we dispatch according to the APPLICATION tag value to handle the
    |  special encoding requirements of APPLICATION types using unsigned
    |  numbers: these must be encoded with a leading zero byte to be sure
    |  they are not treated as though they are twos-complement negative
    |  numbers by the encoding logic in mcc_asn_put_ref() (which would strip
    |  the leading one-bits).
    */
    switch (avl_t_value) {

        case APPL_K_COUNTER:
        case APPL_K_GAUGE:
        case APPL_K_TIMETICKS:
            pad_unsign[0] = pad_unsign[1] = pad_unsign[2] = pad_unsign[3] =
            pad_unsign[4] = pad_unsign[5] = pad_unsign[6] = pad_unsign[7] =
            pad_unsign[8] = '\0';   /* Fill padding buffer w/zeroes */

            /* move data from octet_string to local storage w/leading zero */
            move_to = pad_unsign;           /* To padding buffer. . .      */
            move_from = octet->string;      /* From octet string . . .     */
            size = octet->length;           /* This many bytes             */
            while ( size-- > 0 )    /* Copy the unsigned number over . . . */
                *move_to++ = *move_from++;

            /* set pointer to data for put_ref to local storage */
            value_buf_ptr = pad_unsign;

            /* set size from octet string length + 1 character pad */
            size = octet->length + 1;
            break;


        case APPL_K_IPADDRESS :
        case APPL_K_OPAQUE :
        default: /* Dft: We just assume it doesn't require special handling */
            /* set pointer to data for put_ref to octet string value */
            value_buf_ptr = octet->string;

            /* set size from octet string length */
            size = octet->length;
            break;
        }
    }

/* if (attempt to put_ref given value failed) */
if ( (mcc_status = mcc_asn_put_ref(ctx, tag, Type, value_buf_ptr, size)) != MCC_S_NORMAL) {
    return (mcc_status);    /* return MCC error code */
    }

/* if (attempt to end the sequence to encode OID and value failed) */
tag = Type = MCC_K_ASN_DT_EOC;
if ((mcc_status = mcc_asn_put_ref( ctx, tag, Type, 0, 0)) != MCC_S_NORMAL) {
    return (mcc_status);    /* return MCC error code */
    }

/* if (building end is TRUE) */
if (build_end == TRUE) {
    /* close off this build w/call to var_bind_list_build_end
    | (This also puts in a "End of Sequence" to demark the end of the
    |   varbind list sequence)
    */
    if ((mcc_status = var_bind_list_build_end( p_varbind_info )) != MCC_S_NORMAL) {
        return (mcc_status);        /* return MCC error code */
        }
   }

return (MCC_S_NORMAL);
}

/* asn1_to_avl - Decode ASN.1 for Varbind List into AVL */
/* asn1_to_avl - Decode ASN.1 for Varbind List into AVL */
/* asn1_to_avl - Decode ASN.1 for Varbind List into AVL */

int asn1_to_avl(p_varbind_info, p_var_bind_avl, p_moss_error, parse_start)

MCC_T_Descriptor *p_varbind_info;  /* -> MCC dsc for buf to containing ASN.1 */
avl              **p_var_bind_avl; /* ->> AVL to rcv varbind list info       */
man_status       *p_moss_error;    /* -> man_status for MOSS error code      */
BOOL             parse_start;      /* TRUE: Starting new parse on this call  */

/*
INPUTS:

    "p_varbind_info" is a pointer to an MCC descriptor describing the buffer
    to containing the ASN.1 encoding of the contents of the inbound varbind
    list.

    "p_var_bind_avl" is the address of a pointer to the AVL to receive the
    varbind list information.  This pointer should be NULL (no AVL) upon entry.

    "p_moss_error" is a pointer to a "man_status" cell to receive the MOSS
    error code should this function return the MCC code "MCC_S_FAILED".

    parse_start indicates whether we are starting a new ASN.1 parse on this
    call. (This is basically a fancy one-time-only flag used to control
    parsing of the outer sequence around the entire varbind list).

OUTPUTS:

    On success, the function returns MCC_S_NORMAL if the ASN.1 decoding
    succeeded, otherwise:

       * If the error occurred in the supporting MCC ASN.1 functions, that
         function's MCC error return code is returned by this function

       * If the error occurred in an AVL (MOSS) routine, then this function
         returns the MCC code "MCC_S_FAILED" (which is not used by the
         supporting MCC ASN.1 functions) and also returns the MOSS error
         code to the caller of this function.


BIRD'S EYE VIEW:
    Context:
        The caller is SNMP_DECODE_PDU in this module, and it has a buffer
        containing varbind list information in ASN.1 format that it needs
        transferred into an AVL.

    Purpose:
        This function uses the SNMP_LIB functions to parse the ASN.1
        representation of the contents of the inbound buffer and build
        (using MOSS functions) an AVL containing the corresponding
        information.


ACTION SYNOPSIS OR PSEUDOCODE:

    <initialize the AVL>
    if (parsing start is TRUE)
        if (initialization of the buffer to containing the encoding failed)
            <return MCC error code>               (var_bind_list_parse_init)

    if (get of context block failed)
        <return MCC error code>

    if (attempt to parse a start-sequence encoding OID and value failed)
        <return MCC error code>

    if (attempt to get_ref the object id value failed)
        <return MCC error code>

    <set up an Common Agent Object ID to contain this value>

    if (attempt to obtain the tag of the value failed)
        <return MCC error code>

    <extract tag and class from ASN.1 tag value>
    if (ASN.1 tag class shows CONTEXT_SPECIFIC or PRIVATE or CONSTRUCTED)
        <return MOSS "MAN_C_NOT_SUPPORTED" as MOSS error>
        <return MCC_S_FAILED>

    if (ASN.1 tag class shows "Universal")
        <construct get_ref calling sequence values for tag, Type, value size
         and value addr using ASN.1 tag and class value>
        if (tag is MCC_K_ASN_DT_INTEGER)
            <zero (integer-size) bytes of receiving data buffer>
    else
        (* Assume if not Universal, then Application *)
        switch (ASN.1 extracted tag value)
            case APPL_K_IPADDRESS :
                <set expected_type to "Implicit Octetstring">
                <set octet datatype to Octetstring>
                break;

            case APPL_K_COUNTER:
            case APPL_K_GAUGE:
                <set expected_type to "Implicit Integer">
                <set octet datatype to Integer>
                <zero 4 bytes of receiving data buffer>
                break;

            case APPL_K_TIMETICKS:
                <set expected_type to "Implicit Integer">
                <set octet datatype to Integer>
                <zero 8 bytes of receiving data buffer>
                break;

            case APPL_K_OPAQUE :
            default:
                <set expected_type to "Implicit Octetstring">
                <set octet datatype to Octetstring>
                (* Call MIR using OID for "default" *)
                break;

    if (attempt to get_ref given value failed)
        <return MCC error code>

    if (we processed unsigned integers)
        <set octet string length to limit size>
    else if (we processed a signed integer
             and returned length was < integer_size)
        <set octet string length to integer size>

    if (attempt to parse the end of the sequence encoding OID and value failed)
        <return MCC error code>

    if ("add to AVL " failed)
        <return MOSS error>
        <return MCC_S_FAILED>

    <return MCC_S_NORMAL>


OTHER THINGS TO KNOW:

    This function replaces (for our purposes) the SNMP_LIB function named
    "var_bind_list_get()".

    This function endeavors to hide the MOSS routines that manipulate the AVL
    from the rest of the code in this module.  Any errors that occur are
    simply passed upward via the "p_moss_error" argument as described
    under OUTPUTS (above). They'll get logged eventually.

    SNMPPE doesn't expect to have to handle ASN.1 input containing
    constructed values, nor input whose tags are for CONTEXT-SPECIFIC or
    PRIVATE, since these don't occur in SNMP land.

    If there is an APPLICATION tag for which we don't have code to recognize
    the tag value, we should call the MIR.

*/

/* Maximum number of arcs in an Object Identifier */
#define MAX_ARCS 400

/* Maximum amount of data that can be parsed from ASN.1 from a a single */
/*  varbind list entry.                                                 */
#define MAX_DATA 484		/* Max PDU size */
{
struct ASNContext *ctx;  /* ->Context Block used by MCC_ASN functions */
MCC_T_Descriptor buff_info;     /* Buf dscptr rtned by get_ctx:not used here */
int             size;           /* size of data to be submitted to _put_ref  */
int             limit_size;     /* truncation size for unsigned numbers      */
int             Type;           /* MCC "type" value, neg-signed=IMPLICIT, for*/
                                /* mcc_asn1_put_ref                          */
unsigned int    tag;            /* MCC encoded tag value, for _put_ref       */
int             expected_type;  /* MCC function "expected-type" value        */
unsigned int    got_size;       /* MCC function returned "size" value        */
int             tag_id;         /* idcode from ASN.1 tag                     */
int             tag_class;      /* class from ASN.1 tag                      */
unsigned int    tag_form;       /* constructed bit                           */
int             tag_value;      /* returned value                            */
object_id       oid;            /* Object ID for insertion into AVL element  */
unsigned int    value[MAX_ARCS];/* Array to hold parsed Object Id arcs       */
unsigned int    modifier;       /* AVL Modifier cell for AVL element         */
unsigned int    avl_tag;        /* AVL element's full tag value              */
octet_string    octet;          /* AVL element's octet string                */
char            data[MAX_DATA]; /* AVL octet_string data buffer              */
int             i;              /* Handy-Dandy loop index                    */
man_status      moss_status;    /* Return value from AVL routines-->return to*/
                                /*  caller */
int             mcc_status;     /* Return from MCC functions>return to caller*/


/* initialize the AVL */
octet.string = data;
if ((moss_status = moss_avl_init(p_var_bind_avl)) != MAN_C_SUCCESS) {
    *p_moss_error = moss_status;        /* Return this code   */
    return (MCC_S_FAILED);             /* Signal that we did */
    }

/* if (parsing start is TRUE) */
if (parse_start == TRUE) {
    /* if (initialization of the buffer containing the encoding failed)      */
    /* (This also parses a "Start Sequence" marking the start of the varbind */
    /*  list sequence)                                                       */
    if ((mcc_status = var_bind_list_parse_init( p_varbind_info ))
        != MCC_S_NORMAL) {
        return (mcc_status);        /* return MCC error code */
        }
    }

/* if (get of context block failed) */
/* (This get_ctx() call corresponds to the one in "var_bind_list_get()" ) */
if ((mcc_status=get_ctx (p_varbind_info, &ctx, &buff_info)) != MCC_S_NORMAL) {
    return (mcc_status);        /* return MCC error code */
    }

/* ========================================================================= */
/* Now we parse the ASN.1 sequence containing an OID and value from buffer.  */
/* We only support an ASN.1 encoding of the value which is NOT constructed,  */
/* and which may contain only APPLICATION or UNIVERSAL tags.                 */
/*                                                                           */
/*  Don't panic.  There are no loops, it's just straight "fall-thru" code!   */
/* ========================================================================= */

/* if (attempt to parse a start-sequence encoding OID and value failed) */
if ((mcc_status = mcc_asn_get_tag( ctx, &tag)) != MCC_S_NORMAL) {   /*  TAG  */
    return (mcc_status);    /* return MCC error code */
    }
if (tag == MCC_K_ASN_DT_SEQUENCE) {                                 /* VALUE */
    size = 4;   /* Makes Saber happy */
    expected_type = MCC_K_ASN_DT_SEQUENCE;
    mcc_status = mcc_asn_get_ref (ctx,
                                  expected_type,
                                  &Type,
                                  &tag_value,
                                  size,
                                  &got_size);

    if (mcc_status != MCC_S_NORMAL) {
        return (mcc_status);    /* return MCC error code */
        }
    }
else {
    return (MCC_S_INVALID_PDU);
    }

/* if (attempt to get_ref the object id value failed) */
if ((mcc_status = mcc_asn_get_tag( ctx, &tag)) != MCC_S_NORMAL) {
    return (mcc_status);    /* return MCC error code */
    }
if (tag == MCC_K_ASN_DT_OBJECTID) {
    expected_type = MCC_K_ASN_DT_OBJECTID;
    size = MAX_ARCS * sizeof(value[0]);     /* Obtain size in bytes */
    got_size = 0; /* init so we know if we get a 0 length OID */
    mcc_status = mcc_asn_get_ref(ctx,expected_type,&Type,value,size,&got_size);
    if (mcc_status != MCC_S_NORMAL)
        return (mcc_status);    /* return MCC error code */
    else if (got_size == 0)
        return (MCC_S_INVALID_PDU); /* zero length OID not acceptable */
    }
else {
    return (MCC_S_INVALID_PDU);
    }

/* set up an Common Agent Object ID to contain this value */
oid.count = got_size / sizeof(value[0]);
oid.value = &value[0];

/* if (attempt to obtain the tag of the value failed) */
if ((mcc_status = mcc_asn_get_tag( ctx, &tag)) != MCC_S_NORMAL) {
    return (mcc_status);    /* return MCC error code */
    }

/* extract tag and class from ASN.1 tag value */
tag_class = mcc_asn_tag_class (tag);
tag_id = mcc_asn_tag_id (tag);
tag_form = tag & MCC_K_ASN_FORM_CONS;

/* if (ASN.1 tag class shows CONTEXT_SPECIFIC or PRIVATE or CONSTRUCTED)    */
/* (We're rejecting the stuff we can't handle here, we shouldn't have to) */
if (   (tag_class == MCC_K_ASN_CL_CONT)
    || (tag_class == MCC_K_ASN_CL_PRIV)
    || (tag_form == MCC_K_ASN_FORM_CONS) ) {

    /* return MOSS "MAN_C_NOT_SUPPORTED" as MOSS error */
    *p_moss_error = MAN_C_NOT_SUPPORTED;
    return (MCC_S_FAILED);         /* return MCC_S_FAILED */
    }

/* if (ASN.1tag class shows "Universal") */
if (tag_class == MCC_K_ASN_CL_UNIV) {
    /* construct get_ref calling sequence values for tag, Type, value size */
    /* and value addr using ASN.1 tag and class value                      */
    /*
    | Here's whats going on here for expected_type argument to _get_ref():
    |
    |       For a UNIVERSAL (primitive value), the mcc_asn_get_ref() function
    |       expects the "expected_type" argument to be the same as what it
    |       parses, and to be the value of a predefined constant of the form
    |       MCC_K_ASN_DT_<universal-type>, as in
    |
    |               MCC_K_ASN_DT_INTEGER or MCC_K_ASN_DT_OCTETSTRING
    |
    |       The values of these constants are found in header file
    |       mcc_interface_def.h and they are simply the ASN.1 constants
    |       assigned to specify these primitive types:
    |
    |               #define MCC_K_ASN_DT_INTEGER  2
    |               #define MCC_K_ASN_DT_OCTETSTRING 4
    |
    |       Consequently, all we do to generate the "expected_value" value is
    |       to take the ASN.1 tag ID value and use it.
    */
    expected_type = tag_id;

    /*
    |  The value and it's length are simply extracted directly to the
    |  octet_string portion of the AVL in this instance, no transformation
    |  required.  So we transfer it to a local buffer to become part of the
    |  octet_string when the moss_add_avl() call is executed.
    */
    octet.data_type = tag_id;

    /*
    |  The value for the AVL's tag will simply be the UNIVERSAL of whatever
    |  tag id we got from the ASN.1 tag.
    */
    avl_tag = UNIVERSAL(tag_id);

    /*
    |  If the signed 'integer' comes in under the size of our integer,
    |  we'll pad with zeroes.
    |
    |if (tag is MCC_K_ASN_DT_INTEGER)
    */
    if (tag_id == MCC_K_ASN_DT_INTEGER) {
        /* zero (integer-size) bytes of receiving data buffer */
        for (i=0; i < (sizeof(int)); i++)
            data[i] = '\0';
        }
    }

else {   /* At this point, we can assume if not UNIVERSAL, then APPLICATION */

    avl_tag = APPLICATION(tag_id);

    switch (tag_id) {
        case APPL_K_IPADDRESS :
            /* set expected_type to "Implicit Octetstring" */
            expected_type = - MCC_K_ASN_DT_OCTETSTRING;

            /* set octet datatype to Octetstring */
            octet.data_type = MCC_K_ASN_DT_OCTETSTRING;
            break;


        case APPL_K_COUNTER:
        case APPL_K_GAUGE:
             /* set expected_type to "Implicit Integer" */
             expected_type = - MCC_K_ASN_DT_INTEGER;

             /* set octet datatype to Integer */
             octet.data_type = MCC_K_ASN_DT_INTEGER;

             /* zero 4 bytes of receiving data buffer */
             data[0] = data[1] = data[2] = data[3] = '\0';
             limit_size = 4;
             break;


        case APPL_K_TIMETICKS:
             /* set expected_type to "Implicit Integer" */
             expected_type = - MCC_K_ASN_DT_INTEGER;

             /* set octet datatype to Integer */
             octet.data_type = MCC_K_ASN_DT_INTEGER;

             /* zero 8 bytes of receiving data buffer */
             data[0] = data[1] = data[2] = data[3] =
             data[4] = data[5] = data[6] = data[7] = '\0';
             limit_size = 8;
             break;


        case APPL_K_OPAQUE :
        default:
            /* ON DEFAULT (but not opaque): Should call the MIR! */
            /* set expected_type to "Implicit Octetstring" */
            expected_type = - MCC_K_ASN_DT_OCTETSTRING;

            /* set octet datatype to Octetstring */
            octet.data_type = MCC_K_ASN_DT_OCTETSTRING;

            break;
        }
    }

/* if (attempt to get_ref given value failed) */
if ( (mcc_status = mcc_asn_get_ref(ctx,
                                   expected_type,
                                   &Type,
                                   data,
                                   MAX_DATA,
                                   &octet.length)) != MCC_S_NORMAL) {
    return (mcc_status);    /* return MCC error code */
    }

/* if (we processed unsigned numbers) */
if ( expected_type == - MCC_K_ASN_DT_INTEGER ) {
    octet.length = limit_size;      /* set octet string length to limit size */
    }
else /* if (we did a signed integer and returned length was < integer_size) */
    if (expected_type == MCC_K_ASN_DT_INTEGER && octet.length < sizeof(int)) {
    /* set octet string length to integer size */
    octet.length = sizeof(int);
    }

#if defined(sun) || defined(sparc)

if ( ( expected_type == MCC_K_ASN_DT_INTEGER ) ||
     ( expected_type == - MCC_K_ASN_DT_INTEGER ) )
    octet.string = &data[0] + MAX_DATA - octet.length;

#endif

/* if (attempt to parse the end of the sequence encoding OID and value failed) */
if ((mcc_status = mcc_asn_get_tag( ctx, &tag)) != MCC_S_ILVEOC) {
    return (mcc_status);    /* return MCC error code */
    }

/* if ("add to AVL" failed) */
modifier = 0;
if ((moss_status = moss_avl_add  (*p_var_bind_avl,  /* AVL outbound         */
                                  &oid,             /* OID of next element  */
                                  modifier,         /* Modifier of element  */
                                  avl_tag,          /* Tag of element       */
                                  &octet            /* Value of element     */
                                  )) != MAN_C_SUCCESS) {
    *p_moss_error = moss_status;        /* Return this code   */
    return (MCC_S_FAILED);              /* Signal that we did */
    }


return (MCC_S_NORMAL);
}
