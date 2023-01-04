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
static char *rcsid = "@(#)$RCSfile: get.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/05/20 18:32:30 $";
#endif
/*
**++
**  FACILITY:  ZKO3-3
**
**  Copyright (c) 1993  Digital Equipment Corporation
**
**  MODULE DESCRIPTION:
**
**	GET.C
**
**      This module is part of the Managed Object Module (MOM)
**	for internet.
**	It provides the entry point for the GET function.
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
  system_perform_get(),
  interfaces_perform_get(),
  ifEntry_perform_get(),
  at_perform_get(),
  atEntry_perform_get(),
  ip_perform_get(),
  ipAddrEntry_perform_get(),
  ipRouteEntry_perform_get(),
  ipNetToMediaEntry_perform_get(),
  icmp_perform_get(),
  tcp_perform_get(),
  tcpConnEntry_perform_get(),
  udp_perform_get(),
  udpEntry_perform_get(),
  egp_perform_get(),
  egpNeighEntry_perform_get(),
  snmp_perform_get();
  

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      moi_get_attributes 
**
**	This routine calls the moi_directive routine with the class specific
**	perform_get and moss_send_get_reply routines as arguments.
**
**  FORMAL PARAMETERS:
**
**	mom_handle	- Management handle.
**	object_class	- OID of the class to retrieve information about.
**	object_instance	- AVL containg instance name of request.
**	iso_scope	- Matching information for instance qualification. (Not currently used).
**	filter		- AVL containing information for qualifying the request.
**	access_control	- AVL containing access control information for the request.
**	synchronization	- Request to be best effort or atomic (not currently used).
**	attribute_list	- AVL containing attribute identifier list for the request.
**	invoke_id	- Identifier associated with the request.
**	handle		- Address of information for returning data to the PE.
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_PE_TIMEOUT
**
**  GET RETURN REPLIES:
**
**      MAN_C_SUCCESS
**	MAN_C_NO_SUCH_OBJECT_INSTANCE
**	MAN_C_ACCESS_DENIED
**	MAN_C_GET_LIST_ERROR
**	MAN_C_DIRECTIVE_NOT_SUPPORTED
**	MAN_C_INVALID_USE_OF_WILDCARD
**	MAN_C_CONSTRAINT_VIOLATION
**	MAN_C_INSUFFICIENT_RESOURCES
**
**--
*/
man_status moi_get_attributes(  mom_handle,
				object_class, 
	    			object_instance,
				iso_scope,
				filter,
				access_control,
				synchronization,
				attribute_list,
				invoke_id,
				handle)

man_binding_handle  mom_handle;
object_id	    *object_class;
avl		    *object_instance;
scope		    iso_scope;
avl		    *filter;
avl		    *access_control;
int		    synchronization;
avl		    *attribute_list;
int		    invoke_id;
management_handle   *handle;

{

    man_status (*perform_get)();
    int class_code;

    man_status	    status;

#ifdef MOMGENDEBUG
    printf("In the GET routine\n");
    printf("Instance AVL:\n");
    moss_print_dna_avl(object_instance);
    printf("Attribute List:\n");
    moss_print_dna_avl(attribute_list);
#endif

    status = _get_class_code( object_class, &class_code );
    if (status == MAN_C_SUCCESS)
        switch( class_code )
/**+++
**   Used for multiple classes, can be removed
**   if your object supports only one class.
**/            
            {
/**+++
**   Internal constant that the MOM Generator uses
**   for each class.
**/            

                case system_system_ID:
                    perform_get = system_perform_get;
                    break;
                case interfaces_interfaces_ID:
                    perform_get = interfaces_perform_get;
                    break;
                case ifEntry_ifEntry_ID:
                    perform_get = ifEntry_perform_get;
                    break;
                case at_at_ID:
                    perform_get = at_perform_get;
                    break;
                case atEntry_atEntry_ID:
                    perform_get = atEntry_perform_get;
                    break;
                case ip_ip_ID:
                    perform_get = ip_perform_get;
                    break;
                case ipAddrEntry_ipAddrEntry_ID:
                    perform_get = ipAddrEntry_perform_get;
                    break;
                case ipRouteEntry_ipRouteEntry_ID:
                    perform_get = ipRouteEntry_perform_get;
                    break;
                case ipNetToMediaEntry_ipNetToMediaEntry_ID:
                    perform_get = ipNetToMediaEntry_perform_get;
                    break;
                case icmp_icmp_ID:
                    perform_get = icmp_perform_get;
                    break;
                case tcp_tcp_ID:
                    perform_get = tcp_perform_get;
                    break;
                case tcpConnEntry_tcpConnEntry_ID:
                    perform_get = tcpConnEntry_perform_get;
                    break;
                case udp_udp_ID:
                    perform_get = udp_perform_get;
                    break;
                case udpEntry_udpEntry_ID:
                    perform_get = udpEntry_perform_get;
                    break;
                case egp_egp_ID:
                    perform_get = egp_perform_get;
                    break;
                case egpNeighEntry_egpNeighEntry_ID:
                    perform_get = egpNeighEntry_perform_get;
                    break;
                case snmp_snmp_ID:
                    perform_get = snmp_perform_get;
                    break;
	    }
    else 
        return (_error_exit("\n*** Error in get, class not supported\n", status));

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
			   attribute_list,	/* Directive AVL */
			   perform_get,		/* Directive routine */
			   moss_send_get_reply,	/* Reply routine */
			   invoke_id,
			   handle,
			   NULL);
   
    return status;
}
