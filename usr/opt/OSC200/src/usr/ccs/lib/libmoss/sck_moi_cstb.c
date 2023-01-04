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
static char *rcsid = "@(#)$RCSfile: sck_moi_cstb.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 16:00:31 $";
#endif
/*
 **  Copyright (c) Digital Equipment Corporation, 1992, 1993
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
 * Common Agent MOI client stub.
 *
 * Module SCK_MOI_CSTB.C 
 *
 * WRITTEN BY:
 *   Enterprise Management Frameworks
 *   Pat Mulligan  December 1992
 *
 * Overview:
 *  The Common Agent MOI client stub provides stubs for the five  
 *  moi functions; moi_get_attributes, moi_set_attributes, 
 *  moi_create_instance, moi_delete_instance and moi_invoke_action.
 *  These stubs are liked into the PEs to provide transparent
 *  IPC function calls.  Each stub marshalls the functions arguments,
 *  connects to the moi server and transmits the function name and 
 *  argumets and receives a return response.  This response is then 
 *  returned to the caller.
 *
 * History
 *      V1.0    December 1992  Pat Mulligan
 *
 * MODULE CONTENTS:
 *
 * USER-LEVEL INTERFACE (Functions defined in this module for
 *                       reference elsewhere):
 *
 * Function Name           Synopsis
 * --------------------    --------
 * moi_get_attributes      Client stub for moi_get_attributes.
 * moi_set_attributes      Client stub for moi_set_attributes.
 * moi_create_instance     Client stub for moi_create_instance.
 * moi_delete_instance     Client stub for moi_delete_instance.
 * moi_invoke_action       Client stub for moi_invoke_action.
 *
 * INTERNAL FUNCTIONS:
 *
 * Function Name               Synopsis
 * --------------------------  --------
 * process_moi_client_command  Marshall the moi function's arguments,
 *                             connect to the moi server and send the
 *                             function arguments, then receive a
 *                             return status from the server.
 */


      
/*
 *  Header files
 */

#include "man_data.h"
#include "man.h"
#include "socket.h"
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <pthread.h>

#if defined(__osf__)
/* KLUDGE.
 * On Ultrix pthread.h includes cma.h which includes dce/cma_errno.h
 * which includes errno.h.  On OSF dce/cma_errno.h is not present so we
 * include errno.h here.
 */
#include <errno.h>
#endif

extern int   cma_accept();
extern int   cma_read();
extern int   cma_write();
extern int   cma_close();
extern       cma_lib_free();
extern void *cma_lib_malloc();

extern void object_id_to_xmit();
extern void object_id_free_xmit();
extern void avl_to_xmit();
extern void avl_free_xmit();
extern void avl_from_xmit();

/* Define prototypes for local module functions. */

static man_status process_moi_client_command(
                    int                 moi_function,
                    man_binding_handle  mom_handle,
                    object_id          *p_object_class,
                    avl                *p_object_instance,
                    scope               iso_scope,
                    avl                *p_filter,
                    avl                *p_superior_instance, 
                    avl                *p_access_control,
                    avl                *p_reference_instance,
                    int                 synchronization,
                    object_id          *p_action_type,
                    avl                *p_action_information,
                    avl                *p_attribute_list,
                    int                 invoke_identifier,
                    management_handle  *p_return_routine);




man_status moi_get_attributes(
                   mom_handle,
                   p_object_class,
                   p_object_instance,
                   iso_scope,
                   p_filter,
                   p_access_control,
                   synchronization,
                   p_attribute_list,
                   invoke_identifier,
                   p_return_routine
                   )
man_binding_handle  mom_handle;
object_id          *p_object_class;
avl                *p_object_instance;
scope               iso_scope;
avl                *p_filter;
avl                *p_access_control;
int                 synchronization;
avl                *p_attribute_list;
int                 invoke_identifier;
management_handle  *p_return_routine;
/*
 * inputs:  mom_handle is the binding handle for the target object.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          iso_scope is matching info for instance qualification.
 *          p_filter is a pointer to a filter qualifying the operation.
 *          p_access_control is a pointer to access information.
 *          synchronization specifies the method for synchronizing the 
 *            management operation.
 *          p_attribute_list is a pointer to attribute identifier list 
 *            for the mangement operation.
 *          invoke_identifier is an identifier associated with the request. 
 *          p_return_routine is a pointer to the reply data.
 *
 * outputs: none.
 *
 * description:   This function calls process_moi_client_command().
 *                process_moi_client_command marshalls the moi function's
 *                arguments, connects to the moi server and sends the
 *                name of the function to be invoked along with its
 *                arguments it then receives a return status from the
 *                server.
 */
{
  man_status      status               = MAN_C_SUCCESS;
  avl            *p_superior_instance  = NULL,
                 *p_reference_instance = NULL,
                 *p_action_information = NULL;
  object_id      *p_action_type        = NULL;

  /*
   * Call process_moi_client_command() to connect to the mold server
   * marshall and send the name of the function to be invoked along 
   * with its arguments and receive a return status.
   */

  status = process_moi_client_command(
              MOI_GET_ATTRIBUTES, 
              mom_handle, 
              p_object_class, 
              p_object_instance, 
              iso_scope, 
              p_filter, 
              p_superior_instance,     /* not used by this f() */
              p_access_control,
              p_reference_instance,    /* not used by this f() */
              synchronization, 
              p_action_type,           /* not used by this f() */
              p_action_information,    /* not used by this f() */
              p_attribute_list, 
              invoke_identifier,
              p_return_routine);

return(status);
}



man_status moi_set_attributes(
                   mom_handle,
                   p_object_class,
                   p_object_instance,
                   iso_scope,
                   p_filter,
                   p_access_control,
                   synchronization,
                   p_attribute_list,
                   invoke_identifier,
                   p_return_routine
                  )
man_binding_handle  mom_handle;
object_id          *p_object_class;
avl                *p_object_instance;
scope               iso_scope;
avl                *p_filter;
avl                *p_access_control;
int                 synchronization;
avl                *p_attribute_list;
int                 invoke_identifier;
management_handle  *p_return_routine;
/*
 * inputs:  mom_handle is the binding handle for the target object.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          iso_scope is matching info for instance qualification.
 *          p_filter is a pointer to a filter qualifying the operation.
 *          p_access_control is a pointer to access information.
 *          synchronization specifies the method for synchronizing the
 *            management operation.
 *          p_attribute_list is a pointer to attribute identifier list
 *            for the mangement operation.
 *          invoke_identifier is an identifier associated with the request.
 *          p_return_routine is a pointer to the reply data.
 *
 * outputs: none.
 *
 * description:   This function calls process_moi_client_command().
 *                process_moi_client_command marshalls the moi function's
 *                arguments, connects to the moi server and sends the
 *                name of the function to be invoked along with its
 *                arguments it then receives a return status from the
 *                server.
 */
{
  man_status      status               = MAN_C_SUCCESS;
  avl            *p_superior_instance  = NULL,
                 *p_reference_instance = NULL,
                 *p_action_information = NULL;
  object_id      *p_action_type        = NULL;

  /*
   * Call process_moi_client_command() to connect to the mold server
   * marshall and send the name of the function to be invoked along
   * with its arguments and receive a return status.
   */

  status = process_moi_client_command(
              MOI_SET_ATTRIBUTES, 
              mom_handle, 
              p_object_class, 
              p_object_instance, 
              iso_scope, 
              p_filter, 
              p_superior_instance,   /* not used by this f() */
              p_access_control,
              p_reference_instance,  /* not used by this f() */
              synchronization, 
              p_action_type,           /* not used by this f() */
              p_action_information,    /* not used by this f() */
              p_attribute_list, 
              invoke_identifier,
              p_return_routine);

return(status);
}



man_status moi_create_instance(
                    mom_handle,
                    p_object_class,
                    p_object_instance,
                    p_superior_instance,
                    p_access_control,
                    p_reference_instance,
                    p_attribute_list,
                    invoke_identifier,
                    p_return_routine
                   )
/*
 * inputs:  mom_handle is the binding handle for the target object.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          p_superior_instance is a pointer 
 *          p_access_control is a pointer to access information.
 *          p_reference_instance is a pointer to 
 *          p_attribute_list is a pointer to attribute identifier list
 *            for the mangement operation.
 *          invoke_identifier is an identifier associated with the request.
 *          p_return_routine is a pointer to the reply data.
 *
 * outputs: none.
 *
 * description:   This function calls process_moi_client_command().
 *                process_moi_client_command marshalls the moi function's
 *                arguments, connects to the moi server and sends the
 *                name of the function to be invoked along with its
 *                arguments it then receives a return status from the
 *
 */
man_binding_handle  mom_handle ;
object_id          *p_object_class ;
avl                *p_object_instance ;
avl                *p_superior_instance ;
avl                *p_access_control ;
avl                *p_reference_instance ;
avl                *p_attribute_list ;
int                 invoke_identifier ;
management_handle  *p_return_routine ;
{
  man_status  status               = MAN_C_SUCCESS;
  scope       iso_scope            = 0;  
  avl        *p_filter             = NULL,
             *p_action_information = NULL;
  object_id  *p_action_type        = NULL;
  int         synchronization      = 0;
 
  /*
   * Call process_moi_client_command() to connect to the mold server
   * marshall and send the name of the function to be invoked along
   * with its arguments and receive a return status.
   */
 
  status = process_moi_client_command(
              MOI_CREATE_INSTANCE, 
              mom_handle, 
              p_object_class, 
              p_object_instance, 
              iso_scope,             /* not used by this f() */
              p_filter,              /* not used by this f() */
              p_superior_instance,   
              p_access_control,
              p_reference_instance,  
              synchronization,       /* not used by this f() */
              p_action_type,         /* not used by this f() */
              p_action_information,  /* not used by this f() */
              p_attribute_list, 
              invoke_identifier,
              p_return_routine);

return(status);
}



man_status moi_delete_instance(
                    mom_handle,
                    p_object_class,
                    p_object_instance,
                    iso_scope,
                    p_filter,
                    p_access_control,
                    synchronization,
                    invoke_identifier,
                    p_return_routine
                   )
/*
 * inputs:  mom_handle is the binding handle for the target object.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          iso_scope is matching info for instance qualification.
 *          p_filter is a pointer to a filter qualifying the operation.
 *          p_access_control is a pointer to access information.
 *          synchronization specifies the method for synchronizing the
 *            management operation.
 *          invoke_identifier is an identifier associated with the request.
 *          p_return_routine is a pointer to the reply data.
 *
 * outputs: none.
 *
 * description:   This function calls process_moi_client_command().
 *                process_moi_client_command marshalls the moi function's
 *                arguments, connects to the moi server and sends the
 *                name of the function to be invoked along with its
 *                arguments it then receives a return status from the
 *
 */
man_binding_handle  mom_handle ;
object_id          *p_object_class ;
avl                *p_object_instance ;
scope               iso_scope ;
avl                *p_filter ;
avl                *p_access_control ;
int                 synchronization ;
int                 invoke_identifier ;
management_handle  *p_return_routine ;
{
  man_status      status               = MAN_C_SUCCESS;
  avl            *p_superior_instance  = NULL,
                 *p_reference_instance = NULL,
                 *p_attribute_list     = NULL,
                 *p_action_information = NULL;
  object_id      *p_action_type        = NULL;

  /*
   * Call process_moi_client_command() to connect to the mold server
   * marshall and send the name of the function to be invoked along
   * with its arguments and receive a return status.
   */

  status = process_moi_client_command(
              MOI_DELETE_INSTANCE, 
              mom_handle, 
              p_object_class, 
              p_object_instance, 
              iso_scope, 
              p_filter, 
              p_superior_instance,   /* not used by this f() */
              p_access_control,
              p_reference_instance,  /* not used by this f() */
              synchronization,
              p_action_type,         /* not used by this f() */
              p_action_information,  /* not used by this f() */
              p_attribute_list,      /* not used by this f() */
              invoke_identifier,
              p_return_routine);

return(status);
}



man_status moi_invoke_action(
                  mom_handle,
                  p_object_class,
                  p_object_instance,
                  iso_scope,
                  p_filter,
                  p_access_control,
                  synchronization,
                  p_action_type,
                  p_action_information,
                  invoke_identifier,
                  p_return_routine
                 )
/*
 * inputs:  mom_handle is the binding handle for the target object.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          iso_scope is matching info for instance qualification.
 *          p_filter is a pointer to a filter qualifying the operation.
 *          p_access_control is a pointer to access information.
 *          synchronization specifies the method for synchronizing the
 *            management operation.
 *          p_action_type is a pointer to the action to be performed.
 *          p_action_information is a pointer to the arguments for the 
 *            action.
 *          invoke_identifier is an identifier associated with the request.
 *          p_return_routine is a pointer to the reply data.
 *
 * outputs: none.
 *
 * description:   This function calls process_moi_client_command().
 *                process_moi_client_command marshalls the moi function's
 *                arguments, connects to the moi server and sends the
 *                name of the function to be invoked along with its
 *                arguments it then receives a return status from the
 *
 */
man_binding_handle  mom_handle;
object_id          *p_object_class;
avl                *p_object_instance;
scope               iso_scope;
avl                *p_filter;
avl                *p_access_control;
int                 synchronization;
object_id          *p_action_type;
avl                *p_action_information;
int                 invoke_identifier;
management_handle  *p_return_routine;
{  
  man_status      status               = MAN_C_SUCCESS;
  avl            *p_superior_instance  = NULL,
                 *p_reference_instance = NULL,
                 *p_attribute_list     = NULL;

  /*
   * Call process_moi_client_command() to connect to the mold server
   * marshall and send the name of the function to be invoked along
   * with its arguments and receive a return status.
   */

  status = process_moi_client_command(
              MOI_INVOKE_ACTION, 
              mom_handle, 
              p_object_class, 
              p_object_instance, 
              iso_scope, 
              p_filter, 
              p_superior_instance,     /* not used by this f() */
              p_access_control,
              p_reference_instance,    /* not used by this f() */
              synchronization, 
              p_action_type,        
              p_action_information, 
              p_attribute_list,        /* not used by this f() */
              invoke_identifier,
              p_return_routine);

return(status);
}



static man_status process_moi_client_command(
                   moi_function,
                   mom_handle,
                   p_object_class,
                   p_object_instance,
                   iso_scope,
                   p_filter,
                   p_superior_instance,  
                   p_access_control,
                   p_reference_instance, 
                   synchronization,
                   p_action_type,
                   p_action_information,
                   p_attribute_list,
                   invoke_identifier,
                   p_return_routine
                   )
int                 moi_function;
man_binding_handle  mom_handle;
object_id          *p_object_class;
avl                *p_object_instance;
scope               iso_scope;
avl                *p_filter;
avl                *p_superior_instance; 
avl                *p_access_control;
avl                *p_reference_instance;
int                 synchronization;
object_id          *p_action_type;
avl                *p_action_information;
avl                *p_attribute_list;
int                 invoke_identifier;
management_handle  *p_return_routine;
/*
 * inputs:  moi_function is the moi function to call.
 *          mom_handle is the binding handle for the target object.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          iso_scope is matching info for instance qualification.
 *          p_filter is a pointer to a filter qualifying the operation.
 *          p_superior_instance is a pointer to 
 *          p_access_control is a pointer to access information.
 *          p_reference_instance is a pointer to
 *          synchronization specifies the method for synchronizing the
 *            management operation.
 *          p_action_type is a pointer to the action to be performed.
 *          p_action_information is a pointer to the arguments for the
 *            action.
 *          p_attribute_list is a pointer to attribute identifier list
 *            for the mangement operation.
 *          invoke_identifier is an identifier associated with the request.
 *          p_return_routine is a pointer to the reply data.
 *
 * outputs: none.
 *
 * description:   All of the moi_* calls are funneled to this function.
 *                This function marshalls the moi function's arguments,
 *                connects to the moi server and sends the name of the 
 *                function to be invoked along with its arguments, it 
 *                then receives a return status from the server and
 *                returns this status to the function's caller.
 *
 */
{
  man_status      status                   = MAN_C_SUCCESS;
  server_handle   mom_sh;
  size_t          mom_handle_size          = 0,
                  object_class_size        = 0,
                  object_instance_size     = 0,
                  iso_scope_size           = 0,
                  filter_size              = 0,
                  superior_instance_size   = 0,
                  access_control_size      = 0,
                  reference_instance_size  = 0,  
                  synchronization_size     = 0,
                  attribute_list_size      = 0,
                  invoke_id_size           = 0,
                  return_routine_size      = 0,
                  action_type_size         = 0,
                  action_info_size         = 0,
                  buf_size                 = 0,
                  num_args                 = 0,
                  size_t_size              = sizeof(size_t);
  obj_id_trans_t *p_xmit_obj               = NULL,
                 *p_action_type_xmit       = NULL;
  avl_trans_t    *p_object_instance_xmit   = NULL,
                 *p_filter_xmit            = NULL,
                 *p_superior_instance_xmit = NULL,
                 *p_access_control_xmit    = NULL,
                 *p_ref_instance_xmit      = NULL,  
                 *p_attribute_list_xmit    = NULL,
                 *p_action_info_xmit       = NULL;
  char           *p_buf                    = NULL,
                 *p_buf_idx                = NULL;

  /* Determine the number of bytes to send. */

  if (mom_handle != NULL) mom_handle_size = sizeof(*mom_handle);
  iso_scope_size = sizeof(iso_scope);
  synchronization_size = sizeof(synchronization);
  invoke_id_size = sizeof(invoke_identifier);
  if (p_return_routine != NULL) 
    {
    return_routine_size = sizeof(*p_return_routine);
    }

  /* 
   * Flatten the object_ids and AVLs into a transmittable format
   * and find their sizes.
   */

  if (p_object_class != NULL) 
    {
    object_id_to_xmit(p_object_class, &p_xmit_obj);
    if (p_xmit_obj != NULL)
      {
      object_class_size = p_xmit_obj->count * sizeof(p_xmit_obj->value) +
                          sizeof(p_xmit_obj->count);
      }
    }

  if (p_object_instance != NULL)
    {
    avl_to_xmit(p_object_instance, &p_object_instance_xmit);
    if (p_object_instance_xmit != NULL)
      {
      object_instance_size = p_object_instance_xmit->buf_len *
                             sizeof(p_object_instance_xmit->avl);
      }
    }

  if (p_access_control != NULL)
    {
    avl_to_xmit(p_access_control, &p_access_control_xmit);
    if (p_access_control_xmit != NULL)
      {
      access_control_size = p_access_control_xmit->buf_len *
                            sizeof(p_access_control_xmit->avl);
      }
    }

  if (moi_function == MOI_CREATE_INSTANCE)  
    {
    if (p_superior_instance != NULL)
      {
      avl_to_xmit(p_superior_instance, &p_superior_instance_xmit);
      if (p_superior_instance_xmit != NULL)
        {
        superior_instance_size = p_superior_instance_xmit->buf_len *
                                 sizeof(p_superior_instance_xmit->avl);
        }
      }
    if (p_reference_instance != NULL)
      {
      avl_to_xmit(p_reference_instance, &p_ref_instance_xmit);
      if (p_ref_instance_xmit != NULL)
        {
        reference_instance_size = p_ref_instance_xmit->buf_len *
                                  sizeof(p_ref_instance_xmit->avl);
        }
      }
    }
  else
    {
    if (p_filter != NULL)
      {
      avl_to_xmit(p_filter, &p_filter_xmit);
      if (p_filter_xmit != NULL)
        {
        filter_size = p_filter_xmit->buf_len * sizeof(p_filter_xmit->avl);
        }
      }
    }

  if ( (moi_function == MOI_GET_ATTRIBUTES)   ||
       (moi_function == MOI_SET_ATTRIBUTES)   ||
       (moi_function == MOI_CREATE_INSTANCE) )
    {
    if (p_attribute_list != NULL)
      {
      avl_to_xmit(p_attribute_list, &p_attribute_list_xmit);
      if (p_attribute_list_xmit != NULL)
        {
        attribute_list_size = p_attribute_list_xmit->buf_len *
                              sizeof(p_attribute_list_xmit->avl);
        }
      }
    }

  if (moi_function == MOI_INVOKE_ACTION)
    {
    if (p_action_type != NULL)
      {
      object_id_to_xmit(p_action_type, &p_action_type_xmit);
      if (p_action_type_xmit != NULL)
        {
        action_type_size = p_action_type_xmit->count * 
                           sizeof(p_action_type_xmit->value) +
                           sizeof(p_action_type_xmit->count);
        }
      }
    if (p_action_information != NULL)
      {
      avl_to_xmit(p_action_information, &p_action_info_xmit);
      if (p_action_info_xmit != NULL)
        {
        action_info_size = p_action_info_xmit->buf_len *
                           sizeof(p_action_info_xmit->avl);
        }
      }
    }

  if ( (moi_function == MOI_GET_ATTRIBUTES)   ||
       (moi_function == MOI_SET_ATTRIBUTES) )
    {
    num_args = 10;
    }
  else if ( (moi_function == MOI_CREATE_INSTANCE)  ||
            (moi_function == MOI_DELETE_INSTANCE) )
         {
         num_args = 9;
         }
       else if (moi_function == MOI_INVOKE_ACTION)
              {
              num_args = 11;
              }
            else
              {
              status = MAN_C_FAILURE;
              }

  /* Determine the total size of the buffer to send. */

  buf_size = 1 + (num_args * size_t_size) + mom_handle_size +
             object_class_size + object_instance_size + 
             access_control_size + invoke_id_size + 
             return_routine_size;

  if (moi_function == MOI_CREATE_INSTANCE)
    {
    buf_size = buf_size + superior_instance_size + reference_instance_size;
    }
  else
    {
    buf_size = buf_size + iso_scope_size + filter_size + synchronization_size;
    }

  if ( (moi_function == MOI_GET_ATTRIBUTES)   ||
       (moi_function == MOI_SET_ATTRIBUTES)   ||
       (moi_function == MOI_CREATE_INSTANCE) )
    {
    buf_size = buf_size + attribute_list_size;
    }

  if (moi_function == MOI_INVOKE_ACTION)
    {
    buf_size = buf_size + action_type_size + action_info_size;
    }

  p_buf = (char *) malloc(buf_size);
  p_buf_idx = p_buf;

  /* Copy into the buffer the function that the server will invoke. */

  switch(moi_function)
    {
    case MOI_GET_ATTRIBUTES:  *p_buf_idx = MOI_GET_ATTRIBUTES;  break;
    case MOI_SET_ATTRIBUTES:  *p_buf_idx = MOI_SET_ATTRIBUTES;  break;
    case MOI_CREATE_INSTANCE: *p_buf_idx = MOI_CREATE_INSTANCE; break;
    case MOI_DELETE_INSTANCE: *p_buf_idx = MOI_DELETE_INSTANCE; break;
    case MOI_INVOKE_ACTION:   *p_buf_idx = MOI_INVOKE_ACTION;   break;
    default: status = MAN_C_FAILURE;
    }

  p_buf_idx++;

  /* Copy into the buffer the flattened arguments. */

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_man_binding_handle(&p_buf_idx, mom_handle,
                                         mom_handle_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_object_id(&p_buf_idx, p_xmit_obj, object_class_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_avl(&p_buf_idx, p_object_instance_xmit,
                          object_instance_size);
    }
 
  if (moi_function == MOI_CREATE_INSTANCE)
    {
    if (status == MAN_C_SUCCESS)
      {
      status = marshall_avl(&p_buf_idx, p_superior_instance_xmit, filter_size);
      }
    }
  else 
    {
    if (status == MAN_C_SUCCESS)
      {
      status = marshall_int(&p_buf_idx, iso_scope, iso_scope_size);
      }
    if (status == MAN_C_SUCCESS)
      {
      status = marshall_avl(&p_buf_idx, p_filter_xmit, filter_size);
      }
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_avl(&p_buf_idx, p_access_control_xmit,
                          access_control_size);
    }

  if (moi_function != MOI_CREATE_INSTANCE) 
    {
    if (status == MAN_C_SUCCESS)
      {
      status = marshall_int(&p_buf_idx, synchronization, synchronization_size);
      }
    }

  if (moi_function == MOI_INVOKE_ACTION)
    {
    if (status == MAN_C_SUCCESS)
      {
      status = marshall_object_id(&p_buf_idx, p_action_type_xmit, 
                                  action_type_size);
      }
    if (status == MAN_C_SUCCESS)
      {
      status = marshall_avl(&p_buf_idx, p_action_info_xmit, action_info_size);
      }
    }

  if ( (moi_function == MOI_GET_ATTRIBUTES)   ||
       (moi_function == MOI_SET_ATTRIBUTES)   ||
       (moi_function == MOI_CREATE_INSTANCE) )
    {
    if (status == MAN_C_SUCCESS)
      {
      status = marshall_avl(&p_buf_idx, p_attribute_list_xmit,
                            attribute_list_size);
      }
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_int(&p_buf_idx, invoke_identifier, invoke_id_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_man_handle(&p_buf_idx, p_return_routine,
                                 return_routine_size); 
    }

  /* Connect to the mom server. */

  if (status == MAN_C_SUCCESS)
    {
    memset((void *)mom_sh.name.sun_path, '\0', sizeof(mom_sh.name.sun_path));
    strcpy(mom_sh.name.sun_path, mom_handle->socket_address);
    status = connect_to_server(&mom_sh);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = send_msg_to_server_get_status(mom_sh.s, buf_size, p_buf);
    }

  close(mom_sh.s);

  if (p_buf != NULL)
    {
    free(p_buf);
    }
                    
return(status);
}
