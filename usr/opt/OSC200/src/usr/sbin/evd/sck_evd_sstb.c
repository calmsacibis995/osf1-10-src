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
static char *rcsid = "@(#)$RCSfile: sck_evd_sstb.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:31:39 $";
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
 * Common Agent EVD server stub.
 *
 * Module SCK_EVD_SSTB.C 
 *
 * WRITTEN BY:
 *   Enterprise Management Frameworks  
 *   Pat Mulligan  February 1993  
 *
 * Overview:
 *   The Common Agent EVD server listens for EVD IPC procedure
 *   calls, unmarshalls the procedure's arguments and invokes the
 *   specified EVD procedure.
 *
 * History
 *   V1.0    February 1993  Pat Mulligan
 *
 * MODULE CONTENTS:
 *
 * USER-LEVEL INTERFACE (Functions defined in this module for 
 *                       reference elsewhere):
 *
 * Function Name           Synopsis
 * ----------------------  --------
 * rpc_server_register_if  Register an evd server interface.
 * rpc_server_listen       Bind the server to a socket and accept client
 *                         connections.  This function is named
 *                         rpc_server_listen to maintain the option of
 *                         linking the agent code againt the DCE/RPC
 *                         library.
 *
 * MODULE INTERNAL FUNCTIONS:
 *
 * Function Name              Synopsis
 * --------------------       --------
 * process_create_handle      Unmarshall the evd_create_queue_handle()
 *                            arguments call the function and send the 
 *                            return status back to the client.
 *
 * process_delete_handle      Unmarshall the evd_delete_queue_handle()
 *                            arguments call the function and send the 
 *                            return status  back to the client.
 *
 * process_post_event         Unmarshall the evd_post_event() arguments
 *                            call the function and send the return status
 *                            back to the client.
 */



/*
 *  Header files
 */

#include "man_data.h"
#include "man.h"
#include "nbase.h"
#include "ev.h"
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
 * which errno.h.  On OSF dce/cma_errno.h is not present so we include
 * errno.h here.
 */
#include <errno.h>
#endif

extern int   cma_socket();
extern int   cma_accept();
extern int   cma_read();
extern int   cma_write();
extern int   cma_close();
extern void *cma_lib_malloc();
extern       cma_lib_free();

extern void object_id_from_xmit();
extern void object_id_free_xmit();
extern void avl_from_xmit();
extern void avl_free_xmit();
extern void avl_to_xmit();
extern void avl_free_inst();

extern void  socket_log();
extern char *sck_get_man_status();

extern man_status bind_socket();
extern man_status read_msg_from_client();
extern man_status unmarshall_object_id();
extern man_status unmarshall_avl();
extern man_status unmarshall_access_mode();
extern man_status unmarshall_evd_queue_handle();
extern man_status unmarshall_mo_time();
extern man_status unmarshall_uid();
extern man_status marshall_evd_queue_handle();
extern man_status marshall_mstatus();
extern man_status marshall_avl();

man_status process_create_handle();
man_status process_delete_handle();
man_status process_post_event();
man_status marshall_and_send_return_args();

static char evd_ifspec[] = "evd";
char *evd_v11_0_s_ifspec = evd_ifspec;



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

  man_status   mstatus           = MAN_C_SUCCESS;
  size_t       image_name_length = 0,
               space_left_in_buf = 0;
  char         image_name[sizeof(sh.name.sun_path)],
               s_str[8],
               msg[LINEBUFSIZE];

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
    strncat(sh.name.sun_path, (char *) p_if_spec, strlen((char *)p_if_spec));
    sh.name_length =  sizeof(sh.name.sun_path);
    }

  if (mstatus = MAN_C_SUCCESS)
    {
    *p_status = error_status_ok;
    }
  else
    {
    *p_status = mstatus;
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
 *           and I can't change the function's argument lists.
 *
 * outputs: *p_status       - The return status.
 *
 * description:   This function binds the server to a socket and waits for a
 *                client to connect.  When it gets a message the server
 *                unmarshalls the message and dispatches to the
 *                requested function.
 */
{

  extern server_handle sh;
  unsigned int         fd                  = 0,
                       arg_size            = 1,
                       function_to_call    = 0;
  int                  server_state        = SERVER_STARTED;
  man_status           status              = MAN_C_SUCCESS;
  char                *p_buf               = NULL,
                      *p_buf_idx           = NULL, 
                       msg[LINEBUFSIZE];


  status = bind_socket(&sh);

  /*
   * Block at accept() and wait for the client to connect().
   */

  if (status == MAN_C_SUCCESS)
    {
    while (server_state == SERVER_STARTED)
      {
      fd = accept(sh.s, &sh.name, &sh.name_length);
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
         * Copy the first byte from the buffer, this indicates 
         * which function to call.
         */ 
        memcpy((void *) &function_to_call, p_buf_idx, arg_size);
        p_buf_idx = p_buf_idx + arg_size;

        switch (function_to_call)
          {
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
                    MSG(sck_msg014, "S014 - Invalid function evd server"),
                    strerror(errno));
            socket_log(msg, LOG_ERR);

          } /* end-switch (function_to_call) */

          if (status != MAN_C_SUCCESS)
            {
            sprintf(msg,
                    MSG(sck_msg015, "S015 - evd_*() call failed (%s)"),
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

    } /* end-if  (status == MAN_C_SUCCESS) */

    /*
     * If we are here then the bind() failed or the server has been
     * stopped so close the socket and return.
     */

    if (close(sh.s) == -1)
      {
        sprintf(msg,
                MSG(sck_msg007, "S007 - Error during close: '%s'\n"),
                strerror(errno));
        socket_log(msg, LOG_ERR);
      }

    *p_status = error_status_ok;

return;
}


man_status process_create_handle(p_buf_idx, fd)
char         *p_buf_idx;
unsigned int  fd;
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
  char                   msg[LINEBUFSIZE]; 

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



man_status process_delete_handle(p_buf_idx, fd)
char         *p_buf_idx;
unsigned int  fd;
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



man_status process_post_event(p_buf_idx, fd)
char         *p_buf_idx;
unsigned int  fd;
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
  man_status             status            = MAN_C_SUCCESS,
                         send_status       = MAN_C_SUCCESS;
  evd_queue_handle       evd_handle;
  avl                    instance_name,
                         event_parameters;
  object_id              object_class,
                         event_type;
  mo_time                event_time;
  uid                    event_uid,
                         mo_uid;
  char                   return_buf[sizeof(size_t) + sizeof(man_status)],
                         msg[LINEBUFSIZE]; 

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

  /* Send back to return status. */

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

  /* Release allocated memory. */

  avl_free_inst(&instance_name);
  avl_free_inst(&event_parameters);
  if (object_class.value != NULL) free(object_class.value);
  if (event_type.value != NULL) free(event_type.value);

return(status);
}



man_status marshall_and_send_return_args(fd, status_to_send, pp_handle)
unsigned int       fd;
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
   * not the value of evd_queue_handle as one would expect.
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
