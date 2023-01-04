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
static char *rcsid = "@(#)$RCSfile: sck_moi_sstb.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 15:46:18 $";
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
 * Common Agent MOI server stub.
 *
 * Module SCK_MOI_SSTB.C 
 *
 * WRITTEN BY:
 *   Enterprise Management Frameworks
 *   Pat Mulligan  December 1992 
 *
 * Overview:
 *   The Common Agent MOI server listens for MOI IPC procedure
 *   calls, unmarshalls the procedure's arguments and invokes the
 *   specified MOI procedure.
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
 * ----------------------  --------
 * rpc_server_register_if  Register a mom server interface.
 * rpc_server_listen       Bind the server to a socket and accept client
 *                         connections.  This function is named
 *                         rpc_server_listen to maintain the option of
 *                         linking the agent code againt the DCE/RPC
 *                         library.
 * 
 * INTERNAL FUNCTIONS:
 *
 * Function Name            Synopsis
 * -----------------------  --------
 * process_get_attributes   Unmarshall the moi_get_attributes() arguments
 *                          call the function and send the return status
 *                          back to the client.
 *
 * process_set_attributes   Unmarshall the moi_set_attributes() arguments
 *                          call the function and send the return status
 *                          back to the client.
 *
 * process_create_instance  Unmarshall the moi_create_instance() arguments
 *                          call the function and send the return status
 *                          back to the client. 
 *
 * process_delete_instance  Unmarshall the moi_delete_instance() arguments
 *                          call the function and send the return status
 *                          back to the client.
 *
 * process_invoke_action    Unmarshall the moi_invoke_action() arguments 
 *                          call the function and send the return status
 *                          back to the client.
 */



/*
 *  Header files
 */

#include "man_data.h"
#include "nbase.h"
#include "man.h"
#include "mo.h"
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

extern int cma_socket();
extern int cma_accept();
extern int cma_read();
extern int cma_write();
extern int cma_close();
extern void cma_lib_free();

extern void object_id_from_xmit();
extern void object_id_free_xmit();
extern void avl_from_xmit();
extern void avl_free_xmit();
extern void avl_to_xmit();
extern void avl_free_inst();

/* Define prototypes for local module functions. */

static man_status process_get_attributes (char **pp_buf_idx, int fd);
static man_status process_set_attributes (char **pp_buf_idx, int fd);
static man_status process_create_instance(char **pp_buf_idx, int fd);
static man_status process_delete_instance(char **pp_buf_idx, int fd);
static man_status process_invoke_action  (char **pp_buf_idx, int fd);

static char mo_ifspec[] = "mom";

/* 
 * External variable used by the caller of rpc_server_register_if() 
 * The caller expects mo_v1_0_s_ifspec to be defined externally.
 */

char *mo_v1_0_s_ifspec = mo_ifspec;



void rpc_server_register_if(p_if_spec, mgr_type_uuid, mgr_epv, p_status)
unsigned char *p_if_spec;
uuid_p_t       mgr_type_uuid;
rpc_mgr_epv_t  mgr_epv;
unsigned int  *p_status;

/*
 * inputs:   p_if_spec     - Pointer to a character array which will be used
 *                           to construct the server's name.
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
 * side effects:  The external variable sh will be set.  sh needs to
 *                be an external variable because it is used by both
 *                rpc_server_register_if and rpc_server_listen and I
 *                can't change the function's argument lists because we
 *                want to maintain compatibility with the DCE/RPC API.
 *                The typedef declaration of server_handle is in man_data.h.
 *                sh.s will be set to the descriptor of the socket created.
 *                sh.name.sun_family will be set to AF_UNIX. 
 *                sh.name.sun_path will be set to the name of the server.
 *                sh.name_length will contain the length of the server name.
 */
{
#define UNDERSCORE "_"

  extern server_handle  sh;
  /*
   * global_mom_name is set equal to the executable name of the calling MOM.
   */
  extern char          *global_mom_name;

  man_status   mstatus           = MAN_C_SUCCESS;
  size_t       image_name_length = 0,
               space_left_in_buf = 0;
  char         s_str[8],
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
    /* Build the socket path name; for example /dev/eca_mom_rfc1213_mom3. */

    sh.name.sun_family = AF_UNIX;
    memset((void *)sh.name.sun_path, '\0', sizeof(sh.name.sun_path));
    strncpy(sh.name.sun_path, SOCKET_PATH, strlen(SOCKET_PATH));
    strncat(sh.name.sun_path, (char *) p_if_spec, strlen((char *)p_if_spec));

    /*
     * The mom server name must be made unique.  We will do this by 
     * appending the image name and socket descriptor to the end of 
     * the socket name string.
     */

    strncat(sh.name.sun_path, UNDERSCORE, strlen(UNDERSCORE));
    image_name_length = strlen(global_mom_name);
    sprintf(s_str, "%d", sh.s);
    space_left_in_buf = (sizeof(sh.name.sun_path) - strlen(SOCKET_PATH) -
                         strlen((char *)p_if_spec) - strlen(UNDERSCORE) - 
                         strlen(s_str) - 1);
    if (image_name_length > space_left_in_buf)
      {
      strncat(sh.name.sun_path, global_mom_name, space_left_in_buf);
      }
    else
      {
      strncat(sh.name.sun_path, global_mom_name, image_name_length);
      }
    strcat(sh.name.sun_path, s_str);
    sh.name_length = sizeof(sh.name.sun_path);
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
 *                            The implementation is single threaded.
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
  unsigned char        function_to_call    = 0;
  int                  server_state        = SERVER_STARTED,
                       fd                  = 0;
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
        /* 
         * Memory is allocated in read_msg_from_client() that must be freed.
         */
        status = read_msg_from_client(fd, &p_buf);
        }

      if (status == MAN_C_SUCCESS)
        {
        /*
         * Set the pointer that will be used to index into the char array
         * to the beginning of the msg received from the client.
         */
        p_buf_idx = p_buf;

        /* 
         * Copy the first byte from the buffer, this indicates which
         * function to call and then increment index.
         */
        memcpy((void *) &function_to_call, p_buf_idx, sizeof(function_to_call));
        p_buf_idx = p_buf_idx + sizeof(function_to_call);

        /* Unmarshall the arguments, call f(), send back the return
         * status and clean up. 
         */
        switch (function_to_call)
          {
          case MOI_GET_ATTRIBUTES :
            status = process_get_attributes(&p_buf_idx, fd);
            break;

          case MOI_SET_ATTRIBUTES :
            status = process_set_attributes(&p_buf_idx, fd);
            break;

          case MOI_CREATE_INSTANCE :
            status = process_create_instance(&p_buf_idx, fd);
            break;

          case MOI_DELETE_INSTANCE :
            status = process_delete_instance(&p_buf_idx, fd);
            break;

          case MOI_INVOKE_ACTION :
            status = process_invoke_action(&p_buf_idx, fd);
            /*
             * Don't log MAN_C_NO_SUCH_ATTRIBUTE_ID errors, the PE 
             * uses this status to roll to the next attrib or class.
             */
            if (status == MAN_C_NO_SUCH_ATTRIBUTE_ID)
              {
              status = MAN_C_SUCCESS;
              }
            break;

          case SERVER_STOPPED :
            server_state = SERVER_STOPPED;
            break;

          default :
            sprintf(msg,
                    MSG(sck_msg008, "S008 - Invalid function moi server"),
                    strerror(errno));
            socket_log(msg, LOG_ERR);

          } /* end-switch (function_to_call) */
         
          if (status != MAN_C_SUCCESS)
            {
            /* Log the error and reset the status. */
            sprintf(msg,
                    MSG(sck_msg012, "S012 - moi_*() call failed (%s)"),
                    sck_get_man_status(status));
            socket_log(msg, LOG_ERR);
            status = MAN_C_SUCCESS;
            }

        } 
        else
        {
        /*
         * There was a problem reading the message from the client.  The
         * problem was logged in read_msg_from_client(), just reset status.
         */
        status = MAN_C_SUCCESS;

        } /* end-if (status == MAN_C_SUCCESS) */

      if (fd != -1)
        {
        /* 
         * If the accept() was successful then close the file descriptor. 
         */
        if (close(fd) == -1)
          {
          sprintf(msg,
                  MSG(sck_msg007, "S007 - Error during close: '%s'\n"),
                  strerror(errno));
          socket_log(msg, LOG_ERR);
          }
        } /* end-if (fd != -1) */

      /* Free allocated memory. */

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
      /* Write to SYSLOG. */
        sprintf(msg, 
                MSG(sck_msg007, "S007 - Error during close: '%s'\n"),
                strerror(errno));
        socket_log(msg, LOG_ERR);
      }

    *p_status = error_status_ok;

return;
}



static man_status process_get_attributes(pp_buf_idx, fd) 
char         **pp_buf_idx;
int            fd;
/*
 * inputs:  pp_buf_idx is the address of a pointer into the byte stream 
 *          that contains the function's arguments.
 *
 *          fd is the streams file descriptor used to send back to
 *          the client a return status.
 *
 * outputs: none.
 *
 * description:  This function unmarshalls the arguments, calls the
 *               requested function and sends a return status back to the
 *               client.
 *
 */
{
  man_status          status              = MAN_C_SUCCESS;
  man_binding_handle  mom_handle          = NULL;
  object_id           obj_class;
  avl                 object_instance,
                     *p_object_instance   = &object_instance,
                      filter,
                     *p_filter            = &filter,
                      access_control,
                     *p_access_control    = &access_control,
                      attribute_list,
                     *p_attribute_list    = &attribute_list;
  int                 synchronization     = 0,
                      invoke_identifier   = 0; 
  scope               iso_scope           = 0;
  management_handle   return_routine,
                     *p_return_routine    = &return_routine;
  char                return_buf[4],
                      msg[LINEBUFSIZE];

  memset((void *)&obj_class,       '\0', sizeof(obj_class));
  memset((void *)&object_instance, '\0', sizeof(object_instance));
  memset((void *)&filter,          '\0', sizeof(filter));
  memset((void *)&access_control,  '\0', sizeof(access_control));
  memset((void *)&attribute_list,  '\0', sizeof(attribute_list));

  status = unmarshall_man_binding_handle(pp_buf_idx, &mom_handle);

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(pp_buf_idx, &obj_class);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_object_instance);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_scope(pp_buf_idx, &iso_scope);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_filter);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_access_control);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_int(pp_buf_idx, &synchronization);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_attribute_list);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_int(pp_buf_idx, &invoke_identifier);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_man_handle(pp_buf_idx, p_return_routine);
    }
                                                                          
  if (status == MAN_C_SUCCESS)
    {
      status = moi_get_attributes(mom_handle, &obj_class, p_object_instance,
                   iso_scope, p_filter, p_access_control, synchronization,
                   p_attribute_list, invoke_identifier, p_return_routine);
    }

  /* Send back the return status. */

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

  /* Release all allocated memory. */

  if (mom_handle != NULL) free(mom_handle);

  if (obj_class.value != NULL) free(obj_class.value);

  /* 
   * Free the memory attached to the avl's.  Can't use moss_free_avl() 
   * because part of the avl is on the stack.
   */

  avl_free_inst(p_object_instance);
  avl_free_inst(p_filter);
  avl_free_inst(p_access_control);
  avl_free_inst(p_attribute_list);

return(status);
}
    


static man_status process_set_attributes(pp_buf_idx, fd)
char         **pp_buf_idx;
int            fd;
/*
 * inputs:  pp_buf_idx is the address of a pointer into the byte stream
 *          that contains the function's arguments.
 *
 *          fd is the streams file descriptor used to send back to
 *          the client a return status.
 *
 * outputs: none.
 *
 * description:  This function unmarshalls the arguments, calls the
 *               requested function and sends a return status back to the
 *               client.
 *
 */
{
  man_status          status              = MAN_C_SUCCESS;
  man_binding_handle  mom_handle          = NULL;
  object_id           obj_class;
  avl                 object_instance,
                     *p_object_instance   = &object_instance,
                      filter,
                     *p_filter            = &filter,
                      access_control, 
                     *p_access_control    = &access_control,
                      attribute_list,
                     *p_attribute_list    = &attribute_list;
  int                 synchronization     = 0,
                      invoke_identifier   = 0;
  scope               iso_scope           = 0;
  management_handle   return_routine,
                     *p_return_routine    = &return_routine; 
  char                return_buf[4],
                      msg[LINEBUFSIZE];

  memset((void *)&obj_class,       '\0', sizeof(obj_class));
  memset((void *)&object_instance, '\0', sizeof(object_instance));
  memset((void *)&filter,          '\0', sizeof(filter));
  memset((void *)&access_control,  '\0', sizeof(access_control));
  memset((void *)&attribute_list,  '\0', sizeof(attribute_list));
 
  status = unmarshall_man_binding_handle(pp_buf_idx, &mom_handle);

  if (status == MAN_C_SUCCESS)
    { 
    status = unmarshall_object_id(pp_buf_idx, &obj_class);
    }

  if (status == MAN_C_SUCCESS)
    { 
    status = unmarshall_avl(pp_buf_idx, p_object_instance);
    }

  if (status == MAN_C_SUCCESS)
    { 
    status = unmarshall_scope(pp_buf_idx, &iso_scope);
    }

  if (status == MAN_C_SUCCESS)
    { 
    status = unmarshall_avl(pp_buf_idx, p_filter);
    }

  if (status == MAN_C_SUCCESS)
    { 
    status = unmarshall_avl(pp_buf_idx, p_access_control);
    }

  if (status == MAN_C_SUCCESS)
    { 
    status = unmarshall_int(pp_buf_idx, &synchronization);
    }

  if (status == MAN_C_SUCCESS)
    { 
    status = unmarshall_avl(pp_buf_idx, p_attribute_list);
    }

  if (status == MAN_C_SUCCESS)
    { 
    status = unmarshall_int(pp_buf_idx, &invoke_identifier);
    }

  if (status == MAN_C_SUCCESS)
    { 
    status = unmarshall_man_handle(pp_buf_idx, p_return_routine);
    }

  if (status == MAN_C_SUCCESS)
    {
      status = moi_set_attributes(mom_handle, &obj_class, p_object_instance,
                  iso_scope, p_filter, p_access_control, synchronization,
                  p_attribute_list, invoke_identifier, p_return_routine);
    }

  /* Send back the return status. */

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

  /* Release all allocated memory. */

  if (mom_handle != NULL) free(mom_handle);

  if (obj_class.value != NULL) free(obj_class.value);

  /*
   * Free the memory attached to the avl's.  Can't use moss_free_avl()
   * because part of the avl is on the stack.
   */

  avl_free_inst(p_object_instance);
  avl_free_inst(p_filter);
  avl_free_inst(p_access_control);
  avl_free_inst(p_attribute_list);

return(status);
}
  


static man_status process_create_instance(pp_buf_idx, fd)
char         **pp_buf_idx;
int            fd;
/*
 * inputs:  pp_buf_idx is the address of a pointer into the byte stream
 *          that contains the function's arguments.
 *
 *          fd is the streams file descriptor used to send back to
 *          the client a return status.
 *
 * outputs: none.
 *
 * description:  This function unmarshalls the arguments, calls the
 *               requested function and sends a return status back to the
 *               client.
 *
 * warning: This function has not been tested because the SNMP PE can't
 *          generated a request of this type.
 *
 */
{
  man_status          status              = MAN_C_SUCCESS;
  man_binding_handle  mom_handle          = NULL;
  object_id           obj_class;
  avl                 object_instance,
                     *p_object_instance   = &object_instance,
                      access_control,
                     *p_access_control    = &access_control,
                      attribute_list,
                     *p_attribute_list    = &attribute_list,
                      superior_instance,
                     *p_superior_instance = &superior_instance,
                      reference_instance,
                     *p_reference_instance = &reference_instance;
  int                 invoke_identifier    = 0; 
  management_handle   return_routine,
                     *p_return_routine     = &return_routine; 
  char                return_buf[4],
                      msg[LINEBUFSIZE];

  memset((void *)&obj_class,         '\0', sizeof(obj_class));
  memset((void *)&object_instance,   '\0', sizeof(object_instance));
  memset((void *)&access_control,    '\0', sizeof(access_control));
  memset((void *)&attribute_list,    '\0', sizeof(attribute_list));
  memset((void *)&superior_instance, '\0', sizeof(superior_instance));
  memset((void *)&reference_instance,'\0', sizeof(reference_instance));

  status = unmarshall_man_binding_handle(pp_buf_idx, &mom_handle);

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(pp_buf_idx, &obj_class);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_object_instance);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_superior_instance);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_access_control);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_reference_instance);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_attribute_list);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_int(pp_buf_idx, &invoke_identifier);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_man_handle(pp_buf_idx, p_return_routine);
    }
                                                                         
  if (status == MAN_C_SUCCESS)
    {
    status = moi_create_instance(mom_handle, &obj_class, p_object_instance,
                p_superior_instance, p_access_control, p_reference_instance,
                p_attribute_list, invoke_identifier, p_return_routine);
    }

  /* Send back the return status. */

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

  /* Release all allocated memory. */

  if (mom_handle != NULL) free(mom_handle);

  if (obj_class.value != NULL) free(obj_class.value);

  /*
   * Free the memory attached to the avl's.  Can't use moss_free_avl()
   * because part of the avl is on the stack.
   */

  avl_free_inst(p_object_instance);
  avl_free_inst(p_access_control);
  avl_free_inst(p_attribute_list);
  avl_free_inst(p_superior_instance);
  avl_free_inst(p_reference_instance);

return(status);
}



static man_status process_delete_instance(pp_buf_idx, fd)
char         **pp_buf_idx;
int            fd;
/*
 * inputs:  pp_buf_idx is the address of a pointer into the byte stream
 *          that contains the function's arguments.
 *
 *          fd is the streams file descriptor used to send back to
 *          the client a return status.
 *
 * outputs: none.
 *
 * description:  This function unmarshalls the arguments, calls the
 *               requested function and sends a return status back to the
 *               client.
 *
 * warning: This function has not been tested because the SNMP PE can't
 *          generated a request of this type.
 */
{
  man_status          status              = MAN_C_SUCCESS;
  man_binding_handle  mom_handle          = NULL;
  object_id           obj_class;
  avl                 object_instance,
                     *p_object_instance   = &object_instance,
                      filter,
                     *p_filter            = &filter,
                      access_control,
                     *p_access_control    = &access_control;
  int                 synchronization     = 0,
                      invoke_identifier   = 0;
  scope               iso_scope           = 0;
  management_handle   return_routine,
                     *p_return_routine    = &return_routine; 
  char                return_buf[4],
                      msg[LINEBUFSIZE];

  memset((void *)&obj_class,      '\0', sizeof(obj_class));
  memset((void *)&object_instance,'\0', sizeof(object_instance));
  memset((void *)&filter,         '\0', sizeof(filter));
  memset((void *)&access_control, '\0', sizeof(access_control));

  status = unmarshall_man_binding_handle(pp_buf_idx, &mom_handle);

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(pp_buf_idx, &obj_class);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_object_instance);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_scope(pp_buf_idx, &iso_scope);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_filter);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_access_control);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_int(pp_buf_idx, &synchronization);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_int(pp_buf_idx, &invoke_identifier);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_man_handle(pp_buf_idx, p_return_routine);
    }

  if (status == MAN_C_SUCCESS)
    {
      status = moi_delete_instance(mom_handle, &obj_class, p_object_instance,
                  iso_scope, p_filter, p_access_control,
                  synchronization, invoke_identifier, p_return_routine);
    }

  /* Send back the return status. */

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

  /* Release all allocated memory. */

  if (mom_handle != NULL) free(mom_handle);

  if (obj_class.value != NULL) free(obj_class.value);

  /*
   * Free the memory attached to the avl's.  Can't use moss_free_avl()
   * because part of the avl is on the stack.
   */

  avl_free_inst(p_object_instance);
  avl_free_inst(p_filter);
  avl_free_inst(p_access_control);

return(status);
}



static man_status process_invoke_action(pp_buf_idx, fd)
char         **pp_buf_idx;
int            fd;
/*
 * inputs:  pp_buf_idx is the address of a pointer into the byte stream
 *          that contains the function's arguments.
 *
 *          fd is the streams file descriptor used to send back to
 *          the client a return status.
 *
 * outputs: none.
 *
 * description:  This function unmarshalls the arguments, calls the
 *               requested function and sends a return status back to the
 *               client.
 *
 */
{
  man_status          status               = MAN_C_SUCCESS;
  man_binding_handle  mom_handle           = NULL;
  object_id           obj_class,
                      action_type;
  avl                 object_instance,
                     *p_object_instance    = &object_instance,
                      filter,
                     *p_filter             = &filter,
                      access_control,
                     *p_access_control     = &access_control,
                      action_information,
                     *p_action_information = &action_information;
  int                 synchronization      = 0,
                      invoke_identifier    = 0;
  scope               iso_scope            = 0;
  management_handle   return_routine,
                     *p_return_routine     = &return_routine; 
  char                return_buf[4],
                      msg[LINEBUFSIZE];

  memset((void *)&obj_class,         '\0', sizeof(obj_class));
  memset((void *)&action_type,       '\0', sizeof(action_type));
  memset((void *)&object_instance,   '\0', sizeof(object_instance));
  memset((void *)&filter,            '\0', sizeof(filter));
  memset((void *)&access_control,    '\0', sizeof(access_control));
  memset((void *)&action_information,'\0', sizeof(action_information));

  status = unmarshall_man_binding_handle(pp_buf_idx, &mom_handle);

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(pp_buf_idx, &obj_class);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_object_instance);
    }
  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_scope(pp_buf_idx, &iso_scope);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_filter);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_access_control);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_int(pp_buf_idx, &synchronization);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_object_id(pp_buf_idx, &action_type);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_avl(pp_buf_idx, p_action_information);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_int(pp_buf_idx, &invoke_identifier);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = unmarshall_man_handle(pp_buf_idx, p_return_routine);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = moi_invoke_action(mom_handle, &obj_class, p_object_instance,
                iso_scope, p_filter, p_access_control, synchronization,
                &action_type, p_action_information, invoke_identifier,
                p_return_routine);
    }

  /* Send back the return status. */

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

  /* Release all allocated memory. */

  if (mom_handle != NULL) free(mom_handle);

  if (obj_class.value != NULL) free(obj_class.value);
  if (action_type.value != NULL) free(action_type.value);

  /*
   * Free the memory attached to the avl's.  Can't use moss_free_avl()
   * because part of the avl is on the stack.
   */

  avl_free_inst(p_object_instance);
  avl_free_inst(p_filter);
  avl_free_inst(p_access_control);
  avl_free_inst(p_action_information);

return(status);
}
