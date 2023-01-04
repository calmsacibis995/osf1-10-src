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
static char *rcsid = "@(#)$RCSfile: build_action_action.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:20:14 $";
#endif
	{
            /*-insert-code-action-case-*/

	    status =  moi_directive(mom_handle,
				    object_class,
				    object_instance,
				    iso_scope,		
				    filter,
				    access_control,
				    NULL,
				    action_information,  
				    action_routine,
      				    moss_send_action_reply,
      				    invoke_id,
		    		    return_routine,
    				    action_type);
	    break;
	}
