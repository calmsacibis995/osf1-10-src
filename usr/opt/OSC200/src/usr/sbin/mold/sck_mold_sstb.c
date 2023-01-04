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
static char *rcsid = "@(#)$RCSfile: sck_mold_sstb.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:08:26 $";
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
 * Common Agent MOLD server stub.
 *
 * Module SCK_MOLD_SSTB.C 
 *
 * WRITTEN BY:
 *   Enterprise Management Frameworks  
 *   Pat Mulligan  December 1992  
 *
 * Overview:
 *   The Common Agent MOLD server listens for MOLD IPC procedure
 *   calls, unmarshalls the procedure's arguments and invokes the
 *   specified MOLD procedure.
 *
 * History
 *   V1.0    December 1992  Pat Mulligan
 *
 * MODULE CONTENTS:
 *
 * USER-LEVEL INTERFACE (Functions defined in this module for 
 *                       reference elsewhere):
 *
 * Function Name           Synopsis
 * --------------------    --------
 * rpc_server_register_if  Register a mold server interface.
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
 * process_register_mom       Proccess the mold_register_mom() arguments
 *                            call the function and send the return status
 *                            back to the client.
 *
 * process_deregister_mom     Process the mold_deregister_mom() arguments 
 *                            call the function and send the return status
 *                            back to the client.
 *
 * process_find_mom           Process the mold_find_mom() arguments
 *                            call the function and send the return status
 *                            back to the client.
 *
 * process_find_next_mom      Process the mold_find_next_mom() arguments
 *                            call the function and send the return status
 *                            back to the client.
 *
 * unmarshall_register_args   Extract from the transmitted data stream the
 *                            mold_register_mom() arguments.
 *
 * unmarshall_deregister_args Extract from the transmitted data stream the
 *                            mold_deregister_mom() arguments.
 *
 * unmarshall_find_args       Extract from the transmitted data stream the
 *                            mold_find_mom() arguments.
 *
 * unmarshall_find_next_args  Extract from the transmitted data stream the
 *                            mold_find_next_mom() arguments.
 *
 * marshall_and_send_return_args  Flatten and send the return arguments
 *                                to the client.
 */



/*
 *  Header files
 */

#include "man_data.h"
#include "man.h"
#include "nbase.h"
#include "mold.h"
#include "socket.h"
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
extern void *cma_lib_malloc();
extern void  cma_lib_free();

extern void object_id_from_xmit();
extern void object_id_free_xmit();
extern void avl_from_xmit();
extern void avl_free_xmit();
extern void avl_to_xmit();
extern void avl_free_inst();

/* Define prototypes for local module functions. */

static man_status process_register_mom  (char *p_buf_idx, int fd);
static man_status process_deregister_mom(char *p_buf_idx, int fd);
static man_status process_find_mom      (char *p_buf_idx, int fd);
static man_status process_find_next_mom (char *p_buf_idx, int fd);
static man_status unmarshall_register_args(
                    char               *p_buf_idx,
                    man_binding_handle *p_mold_handle,
                    management_handle  *p_man_handle,
                    object_id          *p_parent_id,
                    object_id          *p_mom_id,
                    unsigned int       *p_registration_type,
                    unsigned int       *p_supported_interface,
                    unsigned int       *p_pid);
static man_status unmarshall_deregister_args(
                    char               *p_buf_idx,
                    man_binding_handle *p_mold_handle,
                    management_handle  *p_man_handle,
                    object_id          *p_obj_class);
static man_status unmarshall_find_args(
                    char               *p_buf_idx,
                    man_binding_handle *p_mold_handle,
                    object_id          *p_base_class,
                    unsigned int       *p_containment_level,
                    avl                *p_return_info);    
static man_status unmarshall_find_next_args(
                    char               *p_buf_idx,
                    man_binding_handle *p_mold_handle,
                    object_id          *p_current_class,
                    avl                *p_return_info);
static man_status marshall_and_send_return_args(
                    int          fd,
                    man_status   mstatus,
                    avl         *p_return_info);
static void keepalive_connect(int *p_fd);

static char mold_ifspec[] = "mold";

/* External variable needed by rpc_server_register_if() */

char *mold_v1_0_s_ifspec = mold_ifspec;

static pthread_mutex_t thread_fd_mutex; 



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
    strncat(sh.name.sun_path, (char *) p_if_spec, strlen((char *)p_if_spec));
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
  unsigned int         arg_size            = 1,
                       function_to_call    = 0;
  int                  fd                  = 0,
                       thread_fd           = 0,
                       server_state        = SERVER_STARTED,
                       stat                = 0,
                       keepalive_this_fd   = FALSE;
  man_status           status              = MAN_C_SUCCESS;
  server_handle        accept_sh;
  pthread_t            pe_mold_thread;  
  char                *p_buf               = NULL,
                      *p_buf_idx           = NULL, 
                       msg[LINEBUFSIZE];

  if (pthread_mutex_init(&thread_fd_mutex, pthread_mutexattr_default) == -1) 
    {
    sprintf(msg,
            MSG(sck_msg016, "S016 - unable to init mutex: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    }

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
         * Copy the first byte from the buffer, this indicates 
         * which function to call.
         */ 
        memcpy((void *) &function_to_call, p_buf_idx, arg_size);
        p_buf_idx = p_buf_idx + arg_size;

        switch (function_to_call)
          {
          case MOLD_REGISTER_MOM :
            status = process_register_mom(p_buf_idx, fd);
            break;

          case MOLD_DEREGISTER_MOM :
            status = process_deregister_mom(p_buf_idx, fd);
            /*
             * Don't log MAN_C_NO_SUCH_CLASS errors, the rfc1213_mom
             * deregisteres all 18 classes before registering them.
             */
            if (status == MAN_C_NO_SUCH_CLASS) status = MAN_C_SUCCESS;
            break;

          case MOLD_FIND_MOM :
            status = process_find_mom(p_buf_idx, fd);
            /*
             * Don't log MAN_C_NO_SUCH_CLASS errors, the PE
             * uses this status to roll to the next attrib or class.
             */
            if (status == MAN_C_NO_SUCH_CLASS) status = MAN_C_SUCCESS;
            /* 
             * Start a thread to maintain a dedicated pipe from the
             * pe to the mold this will handle subsequent mold_find_mom()
             * calls without having to reconnect.
             */
            if (pthread_mutex_lock(&thread_fd_mutex) == -1) 
              {
              sprintf(msg,
                      MSG(sck_msg018, "S018 - unable to lock mutex: '%s'\n"),
                      strerror(errno));
              socket_log(msg, LOG_ERR);
              }
            thread_fd = fd;
            if (pthread_mutex_unlock(&thread_fd_mutex) == -1) 
              {
              sprintf(msg,
                      MSG(sck_msg019, "S019 - unable to unlock mutex: '%s'\n"),
                      strerror(errno));
              socket_log(msg, LOG_ERR);
              }
            stat = pthread_create(&pe_mold_thread,
                                  pthread_attr_default,
                                  (pthread_startroutine_t) keepalive_connect,
                                  (pthread_addr_t) &thread_fd);
            if (stat != -1)
              {
              keepalive_this_fd = TRUE;
              }
            break;

          case MOLD_FIND_NEXT_MOM :
            status = process_find_next_mom(p_buf_idx, fd);
            break;

          case SERVER_STOPPED :
            server_state = SERVER_STOPPED;
            break;

          default :
            sprintf(msg,
                    MSG(sck_msg009, "S009 - Invalid function mold server"),
                    strerror(errno));
            socket_log(msg, LOG_ERR);

          } /* end-switch (function_to_call) */

          if (status != MAN_C_SUCCESS)
            {
            sprintf(msg,
                    MSG(sck_msg011, "S011 - mold_*() call failed (%s)"),
                    sck_get_man_status(status));
            socket_log(msg, LOG_ERR);
            status = MAN_C_SUCCESS;
            }

        } /* end-if (status == MAN_C_SUCCESS) */
        else
        {
        status = MAN_C_SUCCESS;
        }

      if ((fd != -1) && (!keepalive_this_fd))
        {
        if (close(fd) == -1)
          {
          sprintf(msg,
                  MSG(sck_msg007, "S007 - Error during close: '%s'\n"),
                  strerror(errno));
          socket_log(msg, LOG_ERR);
          }
        } 
      else
        {
        keepalive_this_fd = FALSE;
        }

      if (p_buf != NULL)
        {
        free(p_buf);
        p_buf = NULL;
        }

    } /* end-while (server_state = SERVER_STARTED) */

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

  if (pthread_mutex_destroy(&thread_fd_mutex) == -1) 
    {
    sprintf(msg,
            MSG(sck_msg017, "S017 - unable to destroy mutex: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    }

  *p_status = error_status_ok;

return;
}


static man_status process_register_mom(p_buf_idx, fd)
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
  man_binding_handle   mold_handle         = NULL;
  management_handle    man_handle;
  object_id            parent_id,
                       mom_id;
  unsigned int         registration_type   = 0,
                       supported_interface = 0,
                       pid                 = 0;
  man_status           status              = MAN_C_SUCCESS;
  char                 return_buf[4],
                       msg[LINEBUFSIZE]; 

  memset((void *)&parent_id, '\0', sizeof(object_id));
  memset((void *)&mom_id, '\0', sizeof(object_id));

  status = unmarshall_register_args(p_buf_idx, &mold_handle,
                                    &man_handle, &parent_id, 
                                    &mom_id, &registration_type,
                                    &supported_interface, &pid);

  if (status == MAN_C_SUCCESS)
    {
    status = mold_register_mom(mold_handle, &man_handle,
                               &parent_id, &mom_id, registration_type,
                               supported_interface, pid);
    }

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

  if (mold_handle != NULL) free(mold_handle);
  if (parent_id.value != NULL) free(parent_id.value);
  if (mom_id.value != NULL) free(mom_id.value);

return(status);
}



static man_status process_deregister_mom(p_buf_idx, fd)
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
  man_binding_handle   mold_handle         = NULL;
  management_handle    man_handle;
  object_id            obj_class;
  man_status           status              = MAN_C_SUCCESS;
  char                 return_buf[4],
                       msg[LINEBUFSIZE];

  memset((void *)&obj_class, '\0', sizeof(object_id));

  status = unmarshall_deregister_args(p_buf_idx, &mold_handle,
                                      &man_handle, &obj_class);

  if (status == MAN_C_SUCCESS)
    { 
    status = mold_deregister_mom(mold_handle, &man_handle, &obj_class);
    }

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

  if (mold_handle != NULL) free(mold_handle);
  if (obj_class.value != NULL) free(obj_class.value);

return(status);
}



static man_status process_find_mom(p_buf_idx, fd)
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
  man_binding_handle   mold_handle         = NULL;
  object_id            base_class;   
  unsigned int         containment_level   = 0;
  avl                  return_info,
                      *p_return_info       = &return_info;
  man_status           status              = MAN_C_SUCCESS;

  memset((void *)&base_class, '\0', sizeof(object_id));
  memset((void *)&return_info,'\0', sizeof(return_info));

  status = unmarshall_find_args(p_buf_idx, &mold_handle,
                                &base_class, &containment_level, 
                                p_return_info);

  if (status == MAN_C_SUCCESS)
    {
    status = mold_find_mom(mold_handle, &base_class,
                           containment_level, p_return_info);
    }

  status = marshall_and_send_return_args(fd, status, p_return_info);
   
  if (mold_handle != NULL) free(mold_handle);
  if (base_class.value != NULL) free(base_class.value);
  avl_free_inst(p_return_info);

return(status);
}



static man_status process_find_next_mom(p_buf_idx, fd)
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
  man_binding_handle   mold_handle         = NULL;
  object_id            current_class;
  avl                  return_info,
                      *p_return_info       = &return_info;
  man_status           status              = MAN_C_SUCCESS,
                       send_status         = MAN_C_SUCCESS;

  memset((void *)&current_class, '\0', sizeof(object_id));
  memset((void *)&return_info,   '\0', sizeof(return_info));

  status = unmarshall_find_next_args(p_buf_idx, &mold_handle,
                                     &current_class, p_return_info);

  if (status == MAN_C_SUCCESS)
    {
    status = mold_find_next_mom(mold_handle, current_class,
                                 p_return_info);
    }

  send_status = marshall_and_send_return_args(fd, status, p_return_info);

  if ((status == MAN_C_SUCCESS) && (send_status != MAN_C_SUCCESS))
    {
    status = send_status;
    }

  if (mold_handle != NULL) free(mold_handle);
  if (current_class.value != NULL) free(current_class.value);
  avl_free_inst(p_return_info);

return(status);
}



static man_status unmarshall_register_args(
                       p_buf_idx,
                       p_mold_handle, 
                       p_man_handle, 
                       p_parent_id,
                       p_mom_id, 
                       p_registration_type, 
                       p_supported_interface, 
                       p_pid
                            )
char               *p_buf_idx;
man_binding_handle *p_mold_handle;
management_handle  *p_man_handle;
object_id          *p_parent_id,
                   *p_mom_id;
unsigned int       *p_registration_type,
                   *p_supported_interface,
                   *p_pid;
/*
 * inputs:   p_buf_idx is a pointer into the byte stream the contains
 *                     the function's arguments. 
 *           p_mold_handle is the address of the mold_handle to be
 *                     extracted from the byte stream.
 *           p_man_handle is the address of the management handle to be
 *                     extracted from the byte stream.
 *           p_parent_id is the address of the parent object id to be
 *                     extracted from the byte stream.
 *           p_mom_id is the address of the mom object id to be
 *                     extracted from the byte stream.
 *           p_registration_type is the address of the registration type
 *                     to be extracted.
 *           p_supported_interface is the address of the
 *                     supported_interface to be extracted.
 *           p_pe is the address of the pid to be extracted.
 *           
 *
 * outputs: *p_mold_handle is the value of the mold_handle.
 *          *p_man_handle  is the value of the management handle.
 *          *p_parent_id   is the value of the parent id.
 *          *p_mom_id      is the value of the mod id. 
 *          *p_registration_type is the value of the registration type.
 *          *p_supported_interface is the value of the supported int.
 *          *p_pid         is the value of the pid.
 *
 * side effects: Memory is allocated in unmarshall_man_binding_handle()
 *               and unmarshall_object_id() THAT THE CALLER MUST FREE. 
 *
 * description:  This function copies from the buffer the value of
 *               mold_handle, man_handle, parent_id, mom_id,
 *               registration_type, supported_interface and pid.
 */
{
  man_status status = MAN_C_SUCCESS;

  /*
   * Memory is allocated in unmarshall_man_binding_handle(),
   * the caller must free(*p_mold_handle)
   */

  status = unmarshall_man_binding_handle(&p_buf_idx, p_mold_handle);

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_man_handle(&p_buf_idx, p_man_handle);
    }

  /*
   * Memory is allocated in unmarshall_object_id(),
   * the caller must free(p_parent_id->value) and (p_mom_id->value)
   */

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(&p_buf_idx, p_parent_id); 
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(&p_buf_idx, p_mom_id);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_unsigned_int(&p_buf_idx, p_registration_type);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_unsigned_int(&p_buf_idx, p_supported_interface);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_unsigned_int(&p_buf_idx, p_pid);
    }

return(status);
}



static man_status unmarshall_deregister_args(
               p_buf_idx,
               p_mold_handle,
               p_man_handle,
               p_obj_class
                              )
char               *p_buf_idx; 
man_binding_handle *p_mold_handle;
management_handle  *p_man_handle;
object_id          *p_obj_class; 
/*
 * inputs:   p_buf_idx is a pointer into the byte stream the contains
 *                     the function's arguments.
 *           p_mold_handle is the address of the mold_handle to be
 *                     extracted from the byte stream.
 *           p_man_handle is the address of the management handle to be
 *                     extracted from the byte stream.
 *           p_obj_class is the address of the object class id to be
 *                     extracted from the byte stream.
 *
 * outputs: *p_mold_handle is the value of the extracted mold_handle.
 *          *p_man_handle  is the value of the extracted management handle.
 *          *p_obj_class   is the value of the extracted object class.
 *
 * side effects: Memory is allocated in unmarshall_man_binding_handle()
 *               and unmarshall_object_id() THAT THE CALLER MUST FREE.
 *
 * description:  This function copies from the buffer the value of
 *               mold_handle, man_handle and obj_class.
 */
{
  man_status   status     = MAN_C_SUCCESS;

  /*
   * Memory is allocated in unmarshall_man_binding_handle(),
   * the caller must free(*p_mold_handle)
   */

  status = unmarshall_man_binding_handle(&p_buf_idx, p_mold_handle);

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_man_handle(&p_buf_idx, p_man_handle);
    }

  /*
   * Memory is allocated in unmarshall_object_id(),
   * the caller must free(p_obj_class->value) 
   */

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(&p_buf_idx, p_obj_class);
    }

return(status);
}



static man_status unmarshall_find_args(
               p_buf_idx,
               p_mold_handle,
               p_base_class,
               p_containment_level,
               p_return_info
                                 )
char               *p_buf_idx;
man_binding_handle *p_mold_handle;  
object_id          *p_base_class;  
unsigned int       *p_containment_level;
avl                *p_return_info;    
/*
 * inputs:   p_buf_idx is a pointer into the byte stream the contains
 *                     the function's arguments.
 *           p_mold_handle is the address of the mold_handle to be
 *                     extracted from the byte stream.
 *           p_base_class is the address of the base class object id to be 
 *                     extracted from the byte stream.
 *           p_containment_level is the address of the containment_level
 *                     to be extracted from the byte stream.
 *           p_return_info is the address of the return info avl to be
 *                     extracted from the byte stream.
 *
 * outputs: *p_mold_handle is the value of the mold_handle.
 *          *p_base_class  is the value of the base class object id.
 *          *p_containment_level is the value of the containment_level.
 *          *p_return_info is the value of the return info avl. 
 * 
 * side effects: Memory is allocated in unmarshall_man_binding_handle()
 *               unmarshall_object_id() and unmarshall_avl that
 *               THE CALLER MUST FREE.
 *
 * description:  This function copies from the buffer the value of 
 *               mold_handle, base_class, containment_level and
 *               return_info. 
 */
{
  man_status   status     = MAN_C_SUCCESS;

  /*
   * Memory is allocated in unmarshall_man_binding_handle(),
   * the caller must free(*p_mold_handle)
   */

  status = unmarshall_man_binding_handle(&p_buf_idx, p_mold_handle);

  /*
   * Memory is allocated in unmarshall_object_id(),
   * the caller must free(p_base_class->value)
   */

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(&p_buf_idx, p_base_class);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_unsigned_int(&p_buf_idx, p_containment_level);
    }

  /*
   * Memory is allocated in unmarshall_avl().  The caller
   * must free the memory, use avl_free_inst().
   */

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(&p_buf_idx, p_return_info);
    }

return(status);
}



static man_status unmarshall_find_next_args(
               p_buf_idx,
               p_mold_handle,
               p_current_class,
               p_return_info
                                      )
char               *p_buf_idx;
man_binding_handle *p_mold_handle;   
object_id          *p_current_class; 
avl                *p_return_info;         
/*
 * inputs:   p_buf_idx is a pointer into the byte stream the contains
 *                     the function's arguments.
 *           p_mold_handle is the address of the mold_handle to be
 *                     extracted from the byte stream.
 *           p_current_class is the address of the current class object id 
 *                     to be extracted from the byte stream.
 *           p_return_info is the address of the return info avl to be
 *                     extracted from the byte stream.
 *
 * outputs: *p_mold_handle is the value of the mold_handle.
 *          *p_current_class is the value of the current class object id.
 *          *p_return_info is the value of the return info avl.
 *
 * side effects: Memory is allocated in unmarshall_man_binding_handle()
 *               unmarshall_object_id() and unmarshall_avl that
 *               THE CALLER MUST FREE.
 *
 * description:  This function copies from the buffer the value of
 *               mold_handle, current_class and return_info.
 */
{
  man_status   status     = MAN_C_SUCCESS;

  /*
   * Memory is allocated in unmarshall_man_binding_handle(),
   * the caller must free(*p_mold_handle)
   */

  status = unmarshall_man_binding_handle(&p_buf_idx, p_mold_handle);

  /*
   * Memory is allocated in unmarshall_object_id(),
   * the caller must free(p_current_class->value)
   */

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(&p_buf_idx, p_current_class);
    }

  /*
   * Memory is allocated in unmarshall_avl().  The caller
   * must free the memory, use avl_free_inst().
   */

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(&p_buf_idx, p_return_info);
    }

return(status);
}



static man_status marshall_and_send_return_args(fd, mstatus, p_return_info)
int          fd;
man_status   mstatus;
avl         *p_return_info;
/*
 * inputs:  fd is the streams file descriptor used to send back to
 *             the client the return arguments.
 *         
 *          mstatus is the return status to be sent back to the client.
 *
 *          p_return_info is a pointer to the avl containing the return
 *             information. 
 *
 * outputs: none.
 *
 * description:  This function flattens and sends the return arguments
 *               to the client.
 *
 */
{
  avl_trans_t  *p_return_xmit_obj = NULL;
  size_t        return_info_size  = 0,
                mstatus_size      = 0,
                buf_size          = 0;
  man_status    status            = MAN_C_SUCCESS;
  char         *p_ret_buf         = NULL,
               *p_buf_idx         = NULL,
                msg[LINEBUFSIZE];

  /* Flatten the AVL into a transmittable format. */

  avl_to_xmit(p_return_info, &p_return_xmit_obj);

  if (p_return_xmit_obj != NULL)
    {
    return_info_size = p_return_xmit_obj->buf_len *
                       sizeof(p_return_xmit_obj->avl);
    }
  else
    {
    return_info_size = 0;
    }

  mstatus_size = sizeof(mstatus);

  buf_size = (2 * sizeof(size_t)) + mstatus_size + return_info_size;
  p_ret_buf = (char *) malloc(buf_size);
  p_buf_idx = p_ret_buf;

  status = marshall_mstatus(&p_buf_idx, &mstatus, mstatus_size);

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_avl(&p_buf_idx, p_return_xmit_obj, return_info_size);
    }

  if (write (fd, p_ret_buf, buf_size) == -1)
    {
    /* Write to SYSLOG. */
    sprintf(msg,
            MSG(sck_msg006, "S006 - Error during write: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    status = MAN_C_PROCESSING_FAILURE;
    }

  if (p_ret_buf != NULL)
    {
    free(p_ret_buf);
    }

return(status);
}



static void keepalive_connect(p_fd)
int *p_fd;

/*
 * inputs:  p_fd a pointer to the byte stream's file descriptor.
 *
 * outputs: none.
 *
 * description:  This function, started as a separate thread, will
 *               maintain a dedicated pipe between the pe and the mold.
 *
 */
{

  man_status           status              = MAN_C_SUCCESS;
  int                  fd                  = 0;
  unsigned int         arg_size            = 1,
                       function_to_call    = 0;
  char                *p_buf               = NULL,
                      *p_buf_idx           = NULL, 
                       msg[LINEBUFSIZE];

  if (pthread_mutex_lock(&thread_fd_mutex) == -1) 
    {
    sprintf(msg,
            MSG(sck_msg018, "S018 - unable to lock mutex: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    }

  fd = *p_fd;

  if (pthread_mutex_unlock(&thread_fd_mutex) == -1) 
    {
    sprintf(msg,
            MSG(sck_msg019, "S019 - unable to unlock mutex: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    }

  while (status != MAN_C_PROCESSING_FAILURE)
    {
    status = read_msg_from_client(fd, &p_buf);

    if (status == MAN_C_SUCCESS)
      {
      /* 
       * Set p_buf_idx to the beginning of buf.
       */ 
      p_buf_idx = p_buf;
      /*
       * Copy the first byte from the buffer, this indicates
       * which function to call.
       */
      memcpy((void *) &function_to_call, p_buf_idx, arg_size);
      p_buf_idx = p_buf_idx + arg_size;

      switch (function_to_call)
        {
        case MOLD_FIND_MOM :

          status = process_find_mom(p_buf_idx, fd);
          /*
           * Don't log MAN_C_NO_SUCH_CLASS errors, the PE
           * uses this status to roll to the next attrib or class.
           */
          if (status == MAN_C_NO_SUCH_CLASS)
            {
            status = MAN_C_SUCCESS;
            }
          break;

        default :
          sprintf(msg,
                  MSG(sck_msg009, "S009 - Invalid function mold server"),
                  strerror(errno));
          socket_log(msg, LOG_ERR);

        } /* end-switch (function_to_call) */

      if (status != MAN_C_SUCCESS)
        {
        sprintf(msg,
                MSG(sck_msg011, "S011 - mold_*() call failed (%s)"),
                    sck_get_man_status(status));
        socket_log(msg, LOG_ERR);
        }

      } /* end-if (status == MAN_C_SUCCESS) */

    if (p_buf != NULL)
      {
      free(p_buf);
      p_buf = NULL;
      }

    } /* end-while (status != MAN_C_PROCESSING_FAILURE) */

  if (close(fd) == -1)
    {
    sprintf(msg,
            MSG(sck_msg007, "S007 - Error during close: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    }

return;
}
