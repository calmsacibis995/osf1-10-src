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
static char *rcsid = "@(#)$RCSfile: set.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/05/20 18:32:42 $";
#endif
/*
**++
**  FACILITY:  ZKO3-3
**
**  Copyright (c) 1993  Digital Equipment Corporation
**
**  MODULE DESCRIPTION:
**
**      This module is part of the Managed Object Module (MOM)
**	for internet.
**	It provides the entry point for the SET function.
**
**  AUTHORS:
**
**      Geetha M. Brown
**
**      This code was initially created with the 
**	Ultrix MOM Generator - version X1.1.0
**
**  CREATION DATE:  24-Apr-1993
**
**  MODIFICATION HISTORY:
**
**--
*/

#include "moss.h"
#include "common.h"

extern man_status
  system_perform_set(),
  interfaces_perform_set(),
  ifEntry_perform_set(),
  at_perform_set(),
  atEntry_perform_set(),
  ip_perform_set(),
  ipAddrEntry_perform_set(),
  ipRouteEntry_perform_set(),
  ipNetToMediaEntry_perform_set(),
  icmp_perform_set(),
  tcp_perform_set(),
  tcpConnEntry_perform_set(),
  udp_perform_set(),
  udpEntry_perform_set(),
  egp_perform_set(),
  egpNeighEntry_perform_set(),
  snmp_perform_set();


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      moi_set_attributes
**
**	This routine calls the moi_directive routine with the class specific
**	perform_set and moss_send_set_reply routines as arguments.
**
**  FORMAL PARAMETERS:
**
**	mom_handle	- Unused binding handle.
**	object_class	- OID of the class to manipulate.
**	object_instance	- AVL containing instance to manipulate.
**	iso_scope	- Matching information for instance qualification. (Not currently used).
**	filter		- AVL determining which instance(s) the operation should be performed on.
**	access_control	- AVL containing access control information.
**	synchronization	- Request to be best effort or atomic (not currently used).
**	modification_list - AVL listing the attribute(s) which should be set.
**	invoke_id	- Identifier associated with the request.
**	handle		- Address of information for returning data to the PE. 
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_PE_TIMEOUT
**
**  SET RETURN REPLIES:
**
**      MAN_C_SUCCESS
**	MAN_C_NO_SUCH_OBJECT_INSTANCE
**	MAN_C_ACCESS_DENIED
**	MAN_C_SET_LIST_ERROR
**	MAN_C_DIRECTIVE_NOT_SUPPORTED
**	MAN_C_INVALID_USE_OF_WILDCARD
**	MAN_C_CONSTRAINT_VIOLATION
**	MAN_C_INSUFFICIENT_RESOURCES
**	
**--
*/
man_status moi_set_attributes(  mom_handle,
				object_class, 
	    			object_instance,
				iso_scope,
				filter,
				access_control,
				synchronization,
				modification_list,
				invoke_id,
				handle )

man_binding_handle  mom_handle;
object_id	    *object_class;
avl		    *object_instance;
scope		    iso_scope;
avl		    *filter;
avl		    *access_control;
int		    synchronization;
avl		    *modification_list;
int		    invoke_id;
management_handle   *handle;

{
    man_status (*perform_set)();
    int class_code;

    object_id		*action=NULL;
    man_status		status;

#ifdef MOMGENDEBUG
    printf("In the SET routine!\n");
    printf("Instance AVL:\n");
    moss_print_dna_avl(object_instance);
    printf("Attribute List:\n");
    moss_print_dna_avl(modification_list);
#endif

    status = _get_class_code( object_class, &class_code );
    if (status == MAN_C_SUCCESS)
        switch( class_code )
            {
                case system_system_ID:
                    perform_set = system_perform_set;
                    break;
                case interfaces_interfaces_ID:
                    perform_set = interfaces_perform_set;
                    break;
                case ifEntry_ifEntry_ID:
                    perform_set = ifEntry_perform_set;
                    break;
                case at_at_ID:
                    perform_set = at_perform_set;
                    break;
                case atEntry_atEntry_ID:
                    perform_set = atEntry_perform_set;
                    break;
                case ip_ip_ID:
                    perform_set = ip_perform_set;
                    break;
                case ipAddrEntry_ipAddrEntry_ID:
                    perform_set = ipAddrEntry_perform_set;
                    break;
                case ipRouteEntry_ipRouteEntry_ID:
                    perform_set = ipRouteEntry_perform_set;
                    break;
                case ipNetToMediaEntry_ipNetToMediaEntry_ID:
                    perform_set = ipNetToMediaEntry_perform_set;
                    break;
                case icmp_icmp_ID:
                    perform_set = icmp_perform_set;
                    break;
                case tcp_tcp_ID:
                    perform_set = tcp_perform_set;
                    break;
                case tcpConnEntry_tcpConnEntry_ID:
                    perform_set = tcpConnEntry_perform_set;
                    break;
                case udp_udp_ID:
                    perform_set = udp_perform_set;
                    break;
                case udpEntry_udpEntry_ID:
                    perform_set = udpEntry_perform_set;
                    break;
                case egp_egp_ID:
                    perform_set = egp_perform_set;
                    break;
                case egpNeighEntry_egpNeighEntry_ID:
                    perform_set = egpNeighEntry_perform_set;
                    break;
                case snmp_snmp_ID:
                    perform_set = snmp_perform_set;
                    break;
	    }
    else 
        return (_error_exit("\n*** Error in set, class not supported\n", status));

    /*
     * This directive is handled by the generic "directive" routine,
     * which will apply the filter and scoping rules to all instances
     * of the proper class, and dispatch to the directive-specific
     * routine for matching entities.
     */
    status = moi_directive(mom_handle,
			   object_class,
			   object_instance,
			   iso_scope,
			   filter,
			   access_control,
			   NULL,		/* Directive OID */
			   modification_list,	/* Directive AVL */
			   perform_set,		/* Directive routine */
			   moss_send_set_reply,	/* Reply routine */
			   invoke_id,
			   handle,
			   action);
   
    return status;
}
