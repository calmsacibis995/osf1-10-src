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
static char *rcsid = "@(#)$RCSfile: sck_pei_sstb.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:33:08 $";
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
 * Common Agent PEI and EVD server stub.
 *
 * Module SCK_PEI_SSTB.C
 *
 * WRITTEN BY:
 *   Enterprise Management Frameworks
 *   Pat Mulligan  December 1992
 *
 * Overview:
 *   The Common Agent PEI/EVD server listens for PEI and EVD IPC procedure
 *   calls, unmarshalls the procedure's arguments and invokes the
 *   specified PEI/EVD procedure.
 *
 * History
 *      V1.1    December 1992  Pat Mulligan   -   Original Version
 *
 * MODULE CONTENTS:
 *
 * USER-LEVEL INTERFACE (Functions defined in this module for
 *                       reference elsewhere):
 *
 * Function Name           Synopsis
 * --------------------    --------
 * rpc_server_register_if  Register a pei/evd server interface.
 * rpc_server_listen       Bind the server to a socket and accept client
 *                         connections.  This function is named
 *                         rpc_server_listen to maintain the option of
 *                         linking the agent code againt the DCE/RPC
 *                         library.
 *
 * INTERNAL FUNCTIONS:
 *
 * Function Name                 Synopsis
 * -----------------------       --------
 * process_send_server_command   Unmarshall the functions arguments, call
 *                               the specified pei function and send the 
 *                               return status back to the client.
 * process_create_handle         Unmarshall the functions arguments, call
 *                               evd_create_queue_handle and send the return 
 *                               status back to the client.
 * process_delete_handle         Unmarshall the functions arguments, call
 *                               evd_delete_queue_handle and send the return
 *                               status back to the client.
 * process_post_event            Unmarshall the functions arguments, call
 *                               evd_post_event and send the return status
 *                               back to the client.
 * marshall_and_send_return_args Send the return status back to the client.
 */



/*
 *  Header files
 */

#include "man_data.h"
#include "man.h"
#include "nbase.h"
#include "pe.h"
#include "socket.h"
#include "evd_defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>

#if defined(__osf__)
/* KLUDGE.
 * On Ultrix pthread.h includes cma.h which includes dce/cma_errno.h
 * which includes errno.h.  On OSF dce/cma_errno.h is not present so we 
 * need to include errno.h here.
 */
#include <errno.h>
#endif

extern int   cma_socket();
extern int   cma_accept();
extern int   cma_read();
extern int   cma_write();
extern int   cma_close();
extern void  cma_lib_free();
extern void *cma_lib_malloc();

extern void object_id_from_xmit();
extern void object_id_free_xmit();
extern void avl_from_xmit();
extern void avl_free_xmit();
extern void avl_to_xmit();
extern void avl_free_inst();

/* Define prototypes for local module functions. */

static man_status process_send_server_command(
                    char         **pp_buf_idx,
                    int            fd,
                    unsigned int   pei_function);
static man_status process_create_handle(char *p_buf_idx, int fd);
static man_status process_delete_handle(char *p_buf_idx, int fd);
static man_status process_post_event   (char *p_buf_idx, int fd);
static man_status marshall_and_send_return_args(
                    int                fd,
                    man_status         status_to_send,
                    evd_queue_handle **pp_handle);

/* External variable needed by rpc_server_register_if() */

static char pe_ifspec[] = "snmp_pe";
char *pe_v1_0_s_ifspec = pe_ifspec;



void rpc_server_register_if(p_if_spec, mgr_type_uuid, mgr_epv, p_status)
unsigned char *p_if_spec;
uuid_p_t       mgr_type_uuid;
rpc_mgr_epv_t  mgr_epv;
unsigned int  *p_status;

/*
 * inputs:   p_if_spec     - Pointer to a character array which contains
 *                           the server's name.
 *
 *           mgr_type_uuid - Not used in the socket implementation. 
 * 
 *           mgr_epv       - Not used in the socket implementation.
 *          
 *           p_status      - Pointer to the return status.
 *
 * outputs: *p_status      - The return status.
 *
 * description:   Register a server interface.  This function creates
 *                a socket and constructs the name of the server.
 *
 * side effects:  The external variable sh will be updated.  sh needs to
 *                be an external variable because it is used by both
 *                rpc_server_register_if and rpc_server_listen and I
 *                can't change the function's argument lists.
 *                sh.s will contain the descriptor of the socket created.
 *                sh.name.sun_family will contain AF_UNIX. 
 *                sh.name.sun_path will contain the name of the server.
 *                sh.name_length will contain the length of the server name.
 */
{
#define UNDERSCORE "_"

  extern server_handle  sh;

  /*
   * global_pe_name is set equal to the executable name of the calling PE.
   */
  extern char          *global_pe_name;

  man_status   mstatus           = MAN_C_SUCCESS;
  char         msg[LINEBUFSIZE];

  /*
   * Create a UNIX domain stream socket for the server.
   */

  if ((sh.s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
    sprintf(msg,
            MSG(sck_msg001, "S001 - Error during socket: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    mstatus = MAN_C_PROCESSING_FAILURE;
    }

  if (mstatus == MAN_C_SUCCESS)
    {
    /* Build the socket name. */

    sh.name.sun_family = AF_UNIX;
    memset((void *)sh.name.sun_path, '\0', sizeof(sh.name.sun_path));
    strncpy(sh.name.sun_path, SOCKET_PATH, strlen(SOCKET_PATH));
    strncat(sh.name.sun_path, global_pe_name, strlen(global_pe_name));
    sh.name_length =  sizeof(sh.name.sun_path);
    mstatus = bind_socket(&sh);
    }

  if (mstatus == MAN_C_SUCCESS)
    {
    *p_status = error_status_ok;
    }
  else
    {
    *p_status = error_status_not_ok;
    }

return;
}



void rpc_server_listen(max_calls_exec, p_status)
unsigned int  max_calls_exec,
             *p_status;
/*
 * inputs:   max_calls_exec - Not used in the socket implementation.
 *
 *           p_status       - Pointer to the return status.
 *
 *           The external variable sh will be used as input.
 *           sh needs to be an external variable because it is
 *           shared between rpc_server_register_if and rpc_server_listen
 *           and i can't change the function's argument lists.
 *
 * outputs: *p_status       - The return status.
 *
 * description:   This function binds to a server socket and waits for a
 *                client to connect.  When it gets a message the server
 *                unmarshalls the message and dispatches to the
 *                requested function.
 */
{
  
  extern server_handle sh;
  unsigned int         arg_size            = 1,
                       function_to_call    = 0;
  int                  fd                  = 0,
                       server_state        = SERVER_STARTED;
  man_status           status              = MAN_C_SUCCESS;
  server_handle        accept_sh;
  char                *p_buf               = NULL, 
                      *p_buf_idx           = NULL,
                       msg[LINEBUFSIZE]; 


  /*
   * Block at accept() and wait for the client to connect().
   */

  accept_sh = sh;
  while (server_state == SERVER_STARTED) 
    {
      fd = accept(accept_sh.s, &accept_sh.name, &accept_sh.name_length); 
      if (fd == -1)
        {
        sprintf(msg,
                MSG(sck_msg003, "S003 - Error during accept: '%s'\n"),
                strerror(errno));
        socket_log(msg, LOG_ERR);
        status = MAN_C_PROCESSING_FAILURE;
        }

      if (status == MAN_C_SUCCESS)
        {
        status = read_msg_from_client(fd, &p_buf);
        }

      if (status == MAN_C_SUCCESS)
        {
        p_buf_idx = p_buf;

        /* 
         * Copy the first byte from the buffer, this indicates which
         * function to call.
         */
        memcpy((void *) &function_to_call, p_buf_idx, arg_size);
        p_buf_idx = p_buf_idx + arg_size;

        /* Unmarshall the arguments, call f(), send back the return
         * status and clean up.
         */
        switch (function_to_call)
          {
          case PEI_SEND_GET_REPLY    : 
          case PEI_SEND_SET_REPLY    : 
          case PEI_SEND_CREATE_REPLY : 
          case PEI_SEND_DELETE_REPLY : 
          case PEI_SEND_ACTION_REPLY : 
            status = process_send_server_command(&p_buf_idx, fd, 
                                                function_to_call);
            break;

          case EVD_CREATE_QUEUE_HANDLE :
            status = process_create_handle(p_buf_idx, fd);
            break;

          case EVD_DELETE_QUEUE_HANDLE :
            status = process_delete_handle(p_buf_idx, fd);
            break;

          case EVD_POST_EVENT :
            status = process_post_event(p_buf_idx, fd);
            break;

          case SERVER_STOPPED :
            server_state = SERVER_STOPPED;
            break;

          default :
            sprintf(msg,
                    MSG(sck_msg010, "S010 - Invalid function pe server"),
                    strerror(errno));
            socket_log(msg, LOG_ERR);

          } /* end-switch (function_to_call) */
          
          if (status != MAN_C_SUCCESS)
            {
            sprintf(msg,
                    MSG(sck_msg013, "S013 - pei/evd_*() call failed (%s)"),
                    sck_get_man_status(status));
            socket_log(msg, LOG_ERR);
            status = MAN_C_SUCCESS;
            }

        } /* end-if (status == MAN_C_SUCCESS) */
        else
        {
          status = MAN_C_SUCCESS;
        }

      if (fd != -1)
        {
        if (close(fd) == -1)
          {
          sprintf(msg,
                  MSG(sck_msg007, "S007 - Error during close: '%s'\n"),
                  strerror(errno));
          socket_log(msg, LOG_ERR);
          }
        } /* end-if (fd != -1) */

      if (p_buf != NULL)
        {
        free(p_buf);
        p_buf = NULL;
        }

    } /* end-while (server_state = SERVER_STARTED) */

    /*
     * If we are here then the server has been
     * stopped so close the socket and return.
     */

    if (close(sh.s) == -1)
      {
      /* Write to SYSLOG. */
        sprintf(msg,
                MSG(sck_msg007, "S007 - Error during close: '%s'\n"),
                strerror(errno));
        socket_log(msg, LOG_ERR);
      }

    *p_status = error_status_ok;

return;
}



static man_status process_send_server_command(pp_buf_idx, fd, pei_function)
char         **pp_buf_idx;
int            fd;
unsigned int   pei_function;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the byte stream
 *          that contains the functions arguments.
 *
 *          fd is the streams file descriptor used to send back to
 *          the client a return status.
 *
 *          pei_function is the function to call. 
 *
 * outputs: none.
 *
 * description:  This function unmarshalls the arguments, calls the
 *               requested function and sends a return status to the
 *               client.
 *
 */
{
  man_status          status              = MAN_C_SUCCESS;
  man_binding_handle  pe_handle           = NULL;    
  int                 invoke_identifier   = 0,
                      more_replies        = 0;
  reply_type          reply;
  object_id           object_class,
                      action_type,
                      action_response_type;                 
  avl                 object_instance,              
                     *p_object_instance   = &object_instance,
                      attribute_list,
                     *p_attribute_list    = &attribute_list;
  uid                 object_uid;                   
  mo_time             operation_time;    
  char                return_buf[4],
                      msg[LINEBUFSIZE];

  memset((void *)&object_class,         '\0', sizeof(object_class));
  memset((void *)&object_instance,      '\0', sizeof(object_instance));
  memset((void *)&attribute_list,       '\0', sizeof(attribute_list));
  if (pei_function == PEI_SEND_ACTION_REPLY)
    {
    memset((void *)&action_type,          '\0', sizeof(action_type));
    memset((void *)&action_response_type, '\0', sizeof(action_response_type));
    }

  status = unmarshall_man_binding_handle(pp_buf_idx, &pe_handle);

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_int(pp_buf_idx, &invoke_identifier);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_reply_type(pp_buf_idx, &reply);    
    }

  object_class.value = NULL;
  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(pp_buf_idx, &object_class);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_object_instance);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_uid(pp_buf_idx, &object_uid);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_mo_time(pp_buf_idx, &operation_time);
    }

  if (pei_function == PEI_SEND_ACTION_REPLY)
    {
    action_type.value = NULL;
    action_response_type.value = NULL;
    if (status == MAN_C_SUCCESS)
      {
      status = unmarshall_object_id(pp_buf_idx, &action_type);
      }
    if (status == MAN_C_SUCCESS)
      {
      status = unmarshall_object_id(pp_buf_idx, &action_response_type);
      }
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_attribute_list);
    }

  if (pei_function != PEI_SEND_CREATE_REPLY)
    {
    if (status == MAN_C_SUCCESS)
      {
      status = unmarshall_int(pp_buf_idx, &more_replies);
      }
    }

  if (status == MAN_C_SUCCESS)
    {
    switch (pei_function)
      {
      case PEI_SEND_GET_REPLY    : 

         status = pei_send_get_reply(pe_handle, invoke_identifier, 
                     reply, &object_class, p_object_instance, &object_uid,
                     operation_time, p_attribute_list, more_replies); 
         break;

      case PEI_SEND_SET_REPLY    : 

         status = pei_send_set_reply(pe_handle, invoke_identifier,
                     reply, &object_class, p_object_instance, &object_uid,
                     operation_time, p_attribute_list, more_replies);
         break;

      case PEI_SEND_CREATE_REPLY : 
 
         status = pei_send_create_reply(pe_handle, invoke_identifier,
                     reply, &object_class, p_object_instance, &object_uid,
                     operation_time, p_attribute_list);
         break;

      case PEI_SEND_DELETE_REPLY : 

         status = pei_send_delete_reply(pe_handle, invoke_identifier,
                     reply, &object_class, p_object_instance, &object_uid,
                     operation_time, p_attribute_list, more_replies);
         break;

      case PEI_SEND_ACTION_REPLY : 

         status = pei_send_action_reply(pe_handle, invoke_identifier, 
                     reply, &object_class, p_object_instance, &object_uid,
                     operation_time, &action_type, &action_response_type,
                     p_attribute_list, more_replies);
         break;
      }
    } /* end-if (status == MAN_C_SUCCESS) */
  

  /* Release all allocated memory. */

  if (pe_handle != NULL)
    {
    free(pe_handle);
    }

  if (object_class.value != NULL) free(object_class.value);

  if (pei_function == PEI_SEND_ACTION_REPLY)
    {
    if (action_type.value != NULL) free(action_type.value);

    if (action_response_type.value != NULL) free(action_response_type.value);
    }

  /* 
   * Free the memory attached to the avl's.  Can't use moss_free_avl()
   * because part of the avl is on the stack.
   */

  avl_free_inst(p_object_instance);

  avl_free_inst(p_attribute_list);

  memcpy(return_buf, (void *) &status, sizeof(status));
  if (write (fd, return_buf, sizeof(return_buf)) == -1)
    {
    /* Write to SYSLOG. */
    sprintf(msg,
            MSG(sck_msg006, "S006 - Error during write: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    status = MAN_C_PROCESSING_FAILURE;
    }

return(status);
}



static man_status process_create_handle(p_buf_idx, fd)
char         *p_buf_idx;
int           fd;
/*
 * inputs:  p_buf_idx is a pointer into the byte stream that contains
 *          function's arguments.
 *
 *          fd is the streams file descriptor used to send back to
 *          the client a return status.
 *
 * outputs: none.
 *
 * description:  This function unmarshalls the arguments, calls the
 *               requested function and sends a return status to the
 *               client.
 *
 */
{
  man_status             status           = MAN_C_SUCCESS,
                         send_status      = MAN_C_SUCCESS;
  evd_queue_handle      *p_evd_handle     = NULL;
  avl                    queue_name,
                        *p_queue_name     = &queue_name;
  evd_queue_access_mode  access_mode      = 0;
  size_t                 arg_size         = 0,
                         size_t_size      = sizeof(size_t);

  memset((void *)&queue_name, '\0', sizeof(queue_name));

  /*
   * Because evd_queue_handle is an output argument it has no value
   * to unmarshall.  Just copy the argument size, which acts as
   * a place holder, from the buffer.
   */
  GET_SIZE_T_ARG(arg_size, p_buf_idx, size_t_size);

  status = unmarshall_avl(&p_buf_idx, p_queue_name);

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_access_mode(&p_buf_idx, &access_mode);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = evd_create_queue_handle(&p_evd_handle, p_queue_name, access_mode);;
    }

  /* 
   * Return the status and queue handle value.
   */
  send_status = marshall_and_send_return_args(fd, status, &p_evd_handle);

  if ((status == MAN_C_SUCCESS) && (send_status != MAN_C_SUCCESS))
    {
    status = send_status;
    }

  /*
   * Free the memory attached to the avl's.  Can't use moss_free_avl()
   * because part of the avl is on the stack.
   */
  avl_free_inst(p_queue_name);

return(status);
}



static man_status process_delete_handle(p_buf_idx, fd)
char         *p_buf_idx;
int           fd;
/*
 * inputs:  p_buf_idx is a pointer into the byte stream that contains
 *          function's arguments.
 *
 *          fd is the streams file descriptor used to send back to
 *          the client a return status.
 *
 * outputs: none.
 *
 * description:  This function unmarshalls the arguments, calls the
 *               requested function and sends a return status to the
 *               client.
 *
 */
{
  man_status           status           = MAN_C_SUCCESS;
  evd_queue_handle     evd_handle,
                      *p_evd_handle     = &evd_handle;
  char                 return_buf[sizeof(size_t) + sizeof(man_status)],
                       msg[LINEBUFSIZE];

  status = unmarshall_evd_queue_handle(&p_buf_idx, p_evd_handle);

  if (status == MAN_C_SUCCESS)
    {
    status = evd_delete_queue_handle((evd_queue_handle **)p_evd_handle);
    }

  /* Send back the return status. */

  p_buf_idx = (void *) return_buf;
  status = marshall_mstatus(&p_buf_idx, &status, sizeof(man_status));

  if (write (fd, return_buf, sizeof(return_buf)) == -1)
    {
    sprintf(msg,
            MSG(sck_msg006, "S006 - Error during write: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    status = MAN_C_PROCESSING_FAILURE;
    }

return(status);
}



static man_status process_post_event(p_buf_idx, fd)
char         *p_buf_idx;
int           fd;
/*
 * inputs:  p_buf_idx is a pointer into the byte stream that contains
 *          function's arguments.
 *
 *          fd is the streams file descriptor used to send back to
 *          the client a return status.
 *
 * outputs: none.
 *
 * description:  This function unmarshalls the arguments, calls the
 *               requested function and sends a return status to the
 *               client.
 *
 */
{
  man_status       status            = MAN_C_SUCCESS;
  evd_queue_handle evd_handle;
  avl              instance_name,
                   event_parameters;
  object_id        object_class,
                   event_type;
  mo_time          event_time;
  uid              event_uid,
                   mo_uid;
  char             return_buf[sizeof(size_t) + sizeof(man_status) +
                              sizeof(size_t) + sizeof(uid)],
                   msg[LINEBUFSIZE]; 

  memset((void *)&instance_name,    '\0', sizeof(instance_name));
  memset((void *)&event_parameters, '\0', sizeof(event_parameters));
  memset((void *)&object_class,     '\0', sizeof(object_class));
  memset((void *)&event_type,       '\0', sizeof(event_type));

  status = unmarshall_evd_queue_handle(&p_buf_idx, &evd_handle);

  /*
   * Memory is allocated in unmarshall_object_id()
   * must free(object_class.value)
   */
  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(&p_buf_idx, &object_class);
    }

  /*
   * Memory is allocated in unmarshall_avl().  The caller
   * must free the memory, use avl_free_inst().
   */
  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(&p_buf_idx, &instance_name);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_mo_time(&p_buf_idx, &event_time);
    }

  /*
   * Memory is allocated in unmarshall_object_id()
   * must free(event_type.value)
   */
  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(&p_buf_idx, &event_type);
    }

  /*
   * Memory is allocated in unmarshall_avl().  The caller
   * must free the memory, use avl_free_inst().
   */
  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(&p_buf_idx, &event_parameters);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_uid(&p_buf_idx, &event_uid);
    }

  /* event_uid is an output argument */

  memset((void *)&event_uid, '\0', sizeof(event_uid));

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_uid(&p_buf_idx, &mo_uid);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = evd_post_event((evd_queue_handle *)evd_handle, &object_class, 
                            &instance_name, &event_time, &event_type, 
                            &event_parameters, &event_uid, &mo_uid);
    }

  /* Send back the return status and the output argument event_uid. */

  p_buf_idx = (void *) return_buf;
  status = marshall_mstatus(&p_buf_idx, &status, sizeof(man_status));
  if (status == MAN_C_SUCCESS)
    {
    status = marshall_uid(&p_buf_idx, &event_uid, sizeof(uid));
    }

  if (write (fd, return_buf, sizeof(return_buf)) == -1)
    {
    sprintf(msg,
            MSG(sck_msg006, "S006 - Error during write: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    status = MAN_C_PROCESSING_FAILURE;
    }

  /* Release allocated memory. */

  avl_free_inst(&instance_name);
  avl_free_inst(&event_parameters);
  if (object_class.value != NULL) free(object_class.value);
  if (event_type.value != NULL) free(event_type.value);

return(status);
}



static man_status marshall_and_send_return_args(fd, status_to_send, pp_handle)
int                fd;
man_status         status_to_send;
evd_queue_handle **pp_handle;
/*
 * inputs:  fd is the streams file descriptor used to send back to
 *             the client the return arguments.
 *         
 *          status_to_send is the return status to be sent back to the client.
 *
 *          pp_handle is the address of a pointer to the evd_queue_handle.
 *
 * outputs: none.
 *
 * description:  This function sends the return arguments to the client.
 *
 */
{
  size_t        handle_size       = 0,
                status_size       = 0,
                buf_size          = 0;
  man_status    status            = MAN_C_SUCCESS;
  char         *p_ret_buf         = NULL,
               *p_buf_idx         = NULL,
                msg[LINEBUFSIZE];

  handle_size = sizeof(*pp_handle);
  status_size = sizeof(status_to_send);

  buf_size = (2 * sizeof(size_t)) + status_size + handle_size;
  p_ret_buf = (char *) malloc(buf_size);
  p_buf_idx = p_ret_buf;

  status = marshall_mstatus(&p_buf_idx, &status_to_send, status_size);

  /*
   * We marshall and send the value of the pointer to evd_queue_handle
   * not the value of evd_queue_handle as one might expect.
   */
  if (status == MAN_C_SUCCESS)
    {
    status = marshall_evd_queue_handle(&p_buf_idx, pp_handle, handle_size);
    }

  if (write (fd, p_ret_buf, buf_size) == -1)
    {
    sprintf(msg,
            MSG(sck_msg006, "S006 - Error during write: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    }

  if (p_ret_buf != NULL)
    {
    free(p_ret_buf);
    }

return(status);
}
