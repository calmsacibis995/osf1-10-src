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
static char *rcsid = "@(#)$RCSfile: sck_pei_cstb.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 16:01:38 $";
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
 * Common Agent PEI client stub.
 *
 * Module SCK_PEI_CSTB.C 
 *
 * WRITTEN BY:
 *   Enterprise Management Frameworks
 *   Pat Mulligan  December 1992
 *
 * Overview:
 *  The Common Agent PEI client stub provides stubs for the following
 *  pei functions; pei_send_get_reply, pei_send_set_reply,
 *  pei_send_create_reply, pei_send_delete_reply, and pei_send_action_reply.
 *  These stubs are linked into the MOMs to provide transparent
 *  IPC function calls.  Each stub marshalls the functions arguments,
 *  connects to the pei server and transmits the function name and
 *  arguments and receives a return response.  This response is then
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
 * Function Name            Synopsis
 * --------------------     --------
 * pei_send_get_reply       Client stub for pei_send_get_reply.
 * pei_send_set_reply       Client stub for pei_send_set_reply.
 * pei_send_create_reply    Client stub for pei_send_create_reply.
 * pei_send_delete_reply    Client stub for pei_send_delete_reply.
 * pei_send_action_reply    Client stub for pei_send_action_reply.
 *
 * INTERNAL FUNCTIONS:
 *
 * Function Name                Synopsis
 * --------------------------   --------
 * process_send_client_command  Marshall the pei function's arguments,
 *                              connect to the pei server and send the
 *                              function arguments, then receive a
 *                              return status from the server.
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
 * which errno.h.  On OSF dce/cma_errno.h is not present so we include
 * errno.h here.
 */
#include <errno.h>
#endif

extern int   cma_accept();
extern int   cma_read();
extern int   cma_write();
extern int   cma_close();
extern int   cma_lib_free();
extern void *cma_lib_malloc();

extern void object_id_to_xmit();
extern void object_id_free_xmit();
extern void avl_to_xmit();
extern void avl_free_xmit();
extern void avl_from_xmit();

/* Define prototypes for local module functions. */

static man_status process_send_client_command (
                    int                pei_function,
                    man_binding_handle pe_handle,
                    int                invoke_identifier,
                    reply_type         reply,
                    object_id         *p_object_class,
                    avl               *p_object_instance,
                    uid               *p_object_uid,
                    mo_time            operation_time,
                    object_id         *p_action_type,
                    object_id         *p_action_response_type,
                    avl               *p_attribute_list,
                    int                more_replies);

static char pec_ifspec[] = "snmp_pe";
char *pec_v1_0_s_ifspec = pec_ifspec;



man_status pei_send_get_reply (
                   pe_handle,
                   invoke_identifier,
                   reply,
                   p_object_class,
                   p_object_instance,
                   p_object_uid,
                   operation_time,
                   p_attribute_list,
                   more_replies
                   )
man_binding_handle pe_handle; 
int                invoke_identifier;
reply_type         reply; 
object_id         *p_object_class;
avl               *p_object_instance;
uid               *p_object_uid; 
mo_time            operation_time;
avl               *p_attribute_list;
int                more_replies;
/*
 * inputs:  pe_handle is the binding handle for the protocol engine.
 *          invoke_identifier is an identifier associated with the request.
 *          reply is the type of reply data.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          p_object_uid is a pointer to the unique identifier assigned
 *           to the instance of the object.
 *          operation_time specifies the return timestamp.
 *          p_attribute_list is a pointer to attribute identifier list
 *            for the mangement operation.
 *          more_replies indicates whether more return info exists.
 *
 * outputs: none.
 *
 * description:   This function calls process_send_client_command().
 *                process_send_client_command marshalls the pei function's
 *                arguments, connects to the pei server and sends the
 *                name of the function to be invoked along with its
 *                arguments, it then receives a return status from the
 *                server.
 */
{
  man_status      status                 = MAN_C_SUCCESS;
  object_id      *p_action_type          = NULL,
                 *p_action_response_type = NULL;

  /*
   * Call process_send_client_command() to connect to the pe server
   * send the name of the function to be invoke along with its 
   * arguments and receive a return status.
   */

  status = process_send_client_command(
		PEI_SEND_GET_REPLY,
		pe_handle, 
		invoke_identifier,
                reply, 
		p_object_class, 
		p_object_instance, 
		p_object_uid,
                operation_time,
                p_action_type,            /* Not used by this f(). */
                p_action_response_type,   /* Not used by this f(). */
		p_attribute_list, 
		more_replies);

return(status);
}



man_status pei_send_set_reply (
                   pe_handle,
                   invoke_identifier,
                   reply,
                   p_object_class,
                   p_object_instance,
                   p_object_uid,
                   operation_time,
                   p_attribute_list,
                   more_replies
                   )
man_binding_handle pe_handle;          
int                invoke_identifier; 
reply_type         reply;            
object_id         *p_object_class;     
avl               *p_object_instance;  
uid               *p_object_uid;     
mo_time            operation_time;  
avl               *p_attribute_list;  
int               more_replies;    
/*
 * inputs:  pe_handle is the binding handle for the protocol engine.
 *          invoke_identifier is an identifier associated with the request.
 *          reply is the type of reply data.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          p_object_uid is a pointer to the unique identifier assigned
 *           to the instance of the object.
 *          operation_time specifies the return timestamp.
 *          p_attribute_list is a pointer to attribute identifier list
 *            for the mangement operation.
 *          more_replies indicates whether more return info exists.
 *
 * outputs: none.
 *
 * description:   This function calls process_send_client_command().
 *                process_send_client_command marshalls the pei function's
 *                arguments, connects to the pei server and sends the
 *                name of the function to be invoked along with its
 *                arguments, it then receives a return status from the
 *                server.
 */
{
  man_status      status                 = MAN_C_SUCCESS;
  object_id      *p_action_type          = NULL,
                 *p_action_response_type = NULL;

  /*
   * Call process_send_client_command() to connect to the pe server
   * send the name of the function to be invoke along with its
   * arguments and receive a return status.
   */

  status = process_send_client_command(
		PEI_SEND_SET_REPLY,
		pe_handle, 
		invoke_identifier,
                reply, 
		p_object_class, 
		p_object_instance, 
		p_object_uid,
                operation_time,
                p_action_type,           /* Not used by this f(). */
                p_action_response_type,  /* Not used by this f(). */
		p_attribute_list, 
		more_replies);

return(status);
}



man_status pei_send_create_reply (
                   pe_handle,
                   invoke_identifier,
                   reply,
                   p_object_class,
                   p_object_instance,
                   p_object_uid,
                   operation_time,
                   p_attribute_list 
                   )
man_binding_handle pe_handle;         
int                invoke_identifier;
reply_type         reply;              
object_id         *p_object_class;    
avl               *p_object_instance; 
uid               *p_object_uid;     
mo_time            operation_time;  
avl               *p_attribute_list;
/*
 * inputs:  pe_handle is the binding handle for the protocol engine.
 *          invoke_identifier is an identifier associated with the request.
 *          reply is the type of reply data.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          p_object_uid is a pointer to the unique identifier assigned
 *           to the instance of the object.
 *          operation_time specifies the return timestamp.
 *          p_attribute_list is a pointer to attribute identifier list
 *            for the mangement operation.
 *
 * outputs: none.
 *
 * description:   This function calls process_send_client_command().
 *                process_send_client_command marshalls the pei function's
 *                arguments, connects to the pei server and sends the
 *                name of the function to be invoked along with its
 *                arguments, it then receives a return status from the
 *                server.
 */
{
  man_status   status                 = MAN_C_SUCCESS;
  object_id   *p_action_type          = NULL,
              *p_action_response_type = NULL;
  int          more_replies           = 0;         

  /*
   * Call process_send_client_command() to connect to the pe server
   * send the name of the function to be invoke along with its
   * arguments and receive a return status.
   */

  status = process_send_client_command(
		PEI_SEND_CREATE_REPLY,
		pe_handle, 
		invoke_identifier,
                reply, 
		p_object_class, 
		p_object_instance, 
		p_object_uid,
                operation_time,
                p_action_type,              /* Not used by this f(). */
                p_action_response_type,     /* Not used by this f(). */
		p_attribute_list, 
		more_replies);              /* Not used by this f(). */

return(status);
}



man_status pei_send_delete_reply (
                   pe_handle,
                   invoke_identifier,
                   reply,
                   p_object_class,
                   p_object_instance,
                   p_object_uid,
                   operation_time,
                   p_attribute_list,
                   more_replies
                   )
man_binding_handle pe_handle;          
int                invoke_identifier;  
reply_type         reply;             
object_id         *p_object_class;    
avl               *p_object_instance; 
uid               *p_object_uid;     
mo_time            operation_time;   
avl               *p_attribute_list;
int                more_replies;    
/*
 * inputs:  pe_handle is the binding handle for the protocol engine.
 *          invoke_identifier is an identifier associated with the request.
 *          reply is the type of reply data.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          p_object_uid is a pointer to the unique identifier assigned
 *           to the instance of the object.
 *          operation_time specifies the return timestamp.
 *          p_attribute_list is a pointer to attribute identifier list
 *            for the mangement operation.
 *          more_replies indicates whether more return info exists.
 *
 * outputs: none.
 *
 * description:   This function calls process_send_client_command().
 *                process_send_client_command marshalls the pei function's
 *                arguments, connects to the pei server and sends the
 *                name of the function to be invoked along with its
 *                arguments, it then receives a return status from the
 *                server.
 */
{
  man_status      status                 = MAN_C_SUCCESS;
  object_id      *p_action_type          = NULL,
                 *p_action_response_type = NULL;

  /*
   * Call process_send_client_command() to connect to the pe server
   * send the name of the function to be invoke along with its
   * arguments and receive a return status.
   */

  status = process_send_client_command(
		PEI_SEND_DELETE_REPLY,
		pe_handle, 
		invoke_identifier,
                reply, 
		p_object_class, 
		p_object_instance, 
		p_object_uid,
                operation_time,
                p_action_type,
                p_action_response_type,    /* Not used by this f(). */
		p_attribute_list,          /* Not used by this f(). */
		more_replies);

return(status);
}



man_status pei_send_action_reply (
                   pe_handle,
                   invoke_identifier,
                   reply,
                   p_object_class,
                   p_object_instance,
                   p_object_uid,
                   operation_time,
                   p_action_type,
                   p_action_response_type,
                   p_attribute_list,
                   more_replies
                   )
man_binding_handle pe_handle;          
int                invoke_identifier; 
reply_type         reply;            
object_id         *p_object_class;  
avl               *p_object_instance;
uid               *p_object_uid;    
mo_time            operation_time; 
object_id         *p_action_type;
object_id         *p_action_response_type;
avl               *p_attribute_list;   
int                more_replies;      
/*
 * inputs:  pe_handle is the binding handle for the protocol engine.
 *          invoke_identifier is an identifier associated with the request.
 *          reply is the type of reply data.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          p_object_uid is a pointer to the unique identifier assigned
 *           to the instance of the object.
 *          operation_time specifies the return timestamp.
 *          p_action_type is a pointer to action to perform.
 *          p_action_response_type is a pointer to response code.
 *          p_attribute_list is a pointer to attribute identifier list
 *            for the mangement operation.
 *          more_replies indicates whether more return info exists.
 *
 * outputs: none.
 *
 * description:   This function calls process_send_client_command().
 *                process_send_client_command marshalls the pei function's
 *                arguments, connects to the pei server and sends the
 *                name of the function to be invoked along with its
 *                arguments, it then receives a return status from the
 *                server.
 */
{
  man_status      status              = MAN_C_SUCCESS;

  /*
   * Call process_send_client_command() to connect to the pe server
   * send the name of the function to be invoke along with its
   * arguments and receive a return status.
   */

  status = process_send_client_command(
 		PEI_SEND_ACTION_REPLY,
		pe_handle, 
		invoke_identifier,
                reply, 
		p_object_class, 
		p_object_instance, 
		p_object_uid,
                operation_time, 
                p_action_type,
                p_action_response_type,
		p_attribute_list, 
		more_replies);

return(status);
}



static man_status process_send_client_command (
                   pei_function,
                   pe_handle,
                   invoke_identifier,
                   reply,
                   p_object_class,
                   p_object_instance,
                   p_object_uid,
                   operation_time,
                   p_action_type,
                   p_action_response_type,
                   p_attribute_list,
                   more_replies
                   )
int                pei_function;       /* caller function */
man_binding_handle pe_handle;          /* binding handle */
int                invoke_identifier;  /* the operation id */
reply_type         reply;              /* type of reply data */
object_id         *p_object_class;     /* the object class id */
avl               *p_object_instance;  /* the object instance name */
uid               *p_object_uid;       /* uid of the instance */
mo_time            operation_time;     /* time operation was performed */
object_id         *p_action_type;
object_id         *p_action_response_type;
avl               *p_attribute_list;   /* return data */
int                more_replies;       /* boolean indication more data */
/*
 * inputs:  pei_function indicates which pei function to call.
 *          pe_handle is the binding handle for the protocol engine.
 *          invoke_identifier is an identifier associated with the request.
 *          reply is the type of reply data.
 *          p_object_class is a pointer to class spec for the object.
 *          p_object_instance is a pointer to the name of the object.
 *          p_object_uid is a pointer to the unique identifier assigned
 *           to the instance of the object.
 *          operation_time specifies the return timestamp.
 *          p_action_type is a pointer to the action to perform.
 *          p_action_response_type is a pointer to the respose code.
 *          p_attribute_list is a pointer to attribute identifier list
 *            for the mangement operation.
 *          more_replies indicates whether more return info exists.
 *
 * outputs: none.
 *
 * description:   This function marshalls the pei function's arguments,
 *                connects to the pei server and sends the name of the
 *                function to be invoked along with its arguments, it
 *                then receives a return status from the server.
 */
{
  man_status      status                   = MAN_C_SUCCESS;
  server_handle   pe_sh;
  size_t          pe_handle_size           = 0,
                  invoke_id_size           = 0,
                  reply_size               = 0,
                  object_class_size        = 0,
                  object_instance_size     = 0,
                  object_uid_size          = 0,
                  operation_time_size      = 0,
                  action_type_size         = 0,
                  action_resp_type_size    = 0,
                  attribute_list_size      = 0,
                  more_replies_size        = 0,
                  buf_size                 = 0,
                  num_args                 = 0,
                  size_t_size              = sizeof(size_t);
  obj_id_trans_t *p_xmit_obj               = NULL,
                 *p_action_type_xmit       = NULL,
                 *p_action_resp_xmit       = NULL;
  avl_trans_t    *p_obj_instance_xmit_avl  = NULL,
                 *p_attrib_list_xmit_avl   = NULL;
  char           *p_buf                    = NULL,
                 *p_buf_idx                = NULL;

  /* Determine the number of bytes to send. */

  if (pe_handle != NULL)    pe_handle_size  = sizeof(*pe_handle);
  if (p_object_uid != NULL) object_uid_size = sizeof(*p_object_uid);

  invoke_id_size      = sizeof(invoke_identifier);
  reply_size          = sizeof(reply);
  operation_time_size = sizeof(operation_time.utc);
  more_replies_size   = sizeof(more_replies);

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

  if (pei_function == PEI_SEND_ACTION_REPLY)
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
    if (p_action_response_type != NULL)
      {
      object_id_to_xmit(p_action_response_type, &p_action_resp_xmit);
      if (p_action_resp_xmit != NULL)
        {
        action_resp_type_size = p_action_resp_xmit->count * 
                                sizeof(p_action_resp_xmit->value) +
                                sizeof(p_action_resp_xmit->count);
        }
      }
    }

  if (p_object_instance != NULL)
    {
    avl_to_xmit(p_object_instance, &p_obj_instance_xmit_avl);
    if (p_obj_instance_xmit_avl != NULL)
      {
      object_instance_size = p_obj_instance_xmit_avl->buf_len *
                             sizeof(p_obj_instance_xmit_avl->avl);
      }
    }

  if (p_attribute_list != NULL)
    {
    avl_to_xmit(p_attribute_list, &p_attrib_list_xmit_avl);
    if (p_attrib_list_xmit_avl != NULL)
      {
      attribute_list_size = p_attrib_list_xmit_avl->buf_len *
                            sizeof(p_attrib_list_xmit_avl->avl);
      }
    }

  if ((pei_function == PEI_SEND_GET_REPLY)    ||
      (pei_function == PEI_SEND_SET_REPLY)    ||
      (pei_function == PEI_SEND_DELETE_REPLY))
    {
    num_args = 9;
    }
  else 
    if (pei_function == PEI_SEND_ACTION_REPLY)
      {
      num_args = 11;
      }
    else
      if (pei_function == PEI_SEND_CREATE_REPLY)
        {
        num_args = 8;
        } 

  /* Determine the total size of the buffer to Send. */

  buf_size = 1 + (num_args * size_t_size) + pe_handle_size +
             invoke_id_size + reply_size + object_class_size +
             object_instance_size + object_uid_size + operation_time_size +
             attribute_list_size;

  if (pei_function != PEI_SEND_CREATE_REPLY)
    {
    buf_size = buf_size + more_replies_size;
    }

  if (pei_function == PEI_SEND_ACTION_REPLY)
    {
    buf_size = buf_size + action_type_size + action_resp_type_size;
    }

  p_buf = (char *) malloc(buf_size);
  p_buf_idx = p_buf;

  /* Copy into the buffer the function that the server will invoke. */

  switch(pei_function)
    {
    case PEI_SEND_GET_REPLY:    *p_buf_idx = PEI_SEND_GET_REPLY;    break;
    case PEI_SEND_SET_REPLY:    *p_buf_idx = PEI_SEND_SET_REPLY;    break;
    case PEI_SEND_CREATE_REPLY: *p_buf_idx = PEI_SEND_CREATE_REPLY; break;
    case PEI_SEND_DELETE_REPLY: *p_buf_idx = PEI_SEND_DELETE_REPLY; break;
    case PEI_SEND_ACTION_REPLY: *p_buf_idx = PEI_SEND_ACTION_REPLY; break;
    default: status = MAN_C_FAILURE;
    }

  p_buf_idx++;

  /* Copy into the buffer the flattened arguments. */

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_man_binding_handle(&p_buf_idx, pe_handle, pe_handle_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_int(&p_buf_idx, invoke_identifier, invoke_id_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_reply_type(&p_buf_idx, reply, reply_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_object_id(&p_buf_idx, p_xmit_obj, object_class_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_avl(&p_buf_idx, p_obj_instance_xmit_avl,
                          object_instance_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_uid(&p_buf_idx, p_object_uid, object_uid_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_mo_time(&p_buf_idx, &operation_time, 
                              operation_time_size);
    }

  if (pei_function == PEI_SEND_ACTION_REPLY)
    {
    if (status == MAN_C_SUCCESS)
      {
      status = marshall_object_id(&p_buf_idx, p_action_type_xmit,
                                  action_type_size);
      }
    if (status == MAN_C_SUCCESS)
      {
      status = marshall_object_id(&p_buf_idx, p_action_resp_xmit,
                                  action_resp_type_size);
      }
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_avl(&p_buf_idx, p_attrib_list_xmit_avl, 
                          attribute_list_size);
    }

  if (pei_function != PEI_SEND_CREATE_REPLY)
    {
    if (status == MAN_C_SUCCESS)
      {
      status =  marshall_int(&p_buf_idx, more_replies, more_replies_size);
      }
    }

  /* Connect to the pe server. */

  if (status == MAN_C_SUCCESS)
    {
    memset((void *)pe_sh.name.sun_path, '\0', sizeof(pe_sh.name.sun_path));
    strcpy(pe_sh.name.sun_path, pe_handle->socket_address);
    status = connect_to_server(&pe_sh);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = send_msg_to_server_get_status(pe_sh.s, buf_size, p_buf);
    }

  close(pe_sh.s);

  if (p_buf != NULL)
    {
    free(p_buf);
    }

return(status);
}
