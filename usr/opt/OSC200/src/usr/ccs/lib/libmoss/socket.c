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
static char *rcsid = "@(#)$RCSfile: socket.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/08/02 17:52:05 $";
#endif
/*
 * MODULE DESCRIPTION:
 *
 * Common Agent socket IPC implementation.
 *
 * Module SOCKET.C 
 *
 * WRITTEN BY:
 *   Enterprise Management Frameworks
 *   Pat Mulligan  December 1992
 *
 * Overview:
 *   This module contains the functions that support the
 *   UNIX domain socket IPC implementation.  
 *
 * History
 *      V1.0    December 1992  Pat Mulligan
 *
 * MODULE CONTENTS:
 *
 * USER-LEVEL INTERFACE (Functions defined in this module for
 *                       reference elsewhere):
 *
 * Function Name                    Synopsis
 * -------------------------------  --------
 * rpc_server_unregister_if         Unregister a server interface.
 * rpc_server_use_protseq           A noop in the socket implementation
 *                                  included to maintain compatibility
 *                                  with DCE/RPC.
 * rpc_server_inq_bindings          Return binding handle for
 *                                  communication with a server.
 * rpc_ep_register                  A noop in the socket implementation
 *                                  included to maintain compatibility
 *                                  with DCE/RPC.
 * rpc_ep_unregister                A noop in the socket implementation
 *                                  included to maintain compatibility
 *                                  with DCE/RPC.
 * rpc_binding_vector_free          Free the memory used to store a
 *                                  vector and binding handle.
 * rpc_binding_from_string_binding  Return a binding handle from a
 *                                  string representation.
 * rpc_binding_free                 Releases binding handle resources.
 * rpc_binding_to_string_binding    Return a string representation of a
 *                                  binding handle.
 * rpc_string_free                  Free a string allocated at runtime.
 * rpc_binding_set_object           Set the oobject UUID value into a
 *                                  server binding handle.
 * rpc_string_binding_compose       A noop in the socket implementation
 *                                  included to maintain compatibility
 *                                  with DCE/RPC.
 * rpc_mgmt_stop_server_listening   Tells a server to stop listening for
 *                                  IPC calls.
 * 
 * bind_socket                      Binds a name to a socket and prepare
 *                                  to accept connections.
 * connect_to_server                Create a UNIX domain stream socket
 *                                  and connect to a server.
 * connect_to_server_keepalive      Create a UNIX domain stream socket
 *                                  and connect to a server with KEEPALIVE.
 * unmarshall_man_binding_handle    Extract a man_binding_handle from a 
 *                                  byte stream.
 * unmarshall_object_id             Extract an object_id from a byte stream.
 * unmarshall_avl                   Extract an avl from a byte stream.
 * unmarshall_scope                 Extract a scope from a byte stream.
 * unmarshall_int                   Extract an int from a byte stream.
 * unmarshall_man_handle            Extract a management_handle from a 
 *                                  byte stream.
 * unmarshall_reply_type            Extract a reply_type from a byte stream.
 * unmarshall_uid                   Extract an uid from a byte stream.
 * unmarshall_mo_time               Extract a mo_time from a byte stream.
 * unmarshall_mstatus               Extract a mstatus from a byte stream.
 * unmarshall_queue_handle          Extract a queue_handle.
 * unmarshall_access_mode           Extract an evd access_mode.
 * marshall_man_binding_handle      Insert a man_binding_handle into a
 *                                  byte stream.
 * marshall_object_id               Insert an object_id into a byte stream.
 * marshall_avl                     Insert an avl into a byte stream.
 * marshall_scope                   Insert a scope into a byte stream.
 * marshall_int                     Insert an int into a byte stream.
 * marshall_man_handle              Insert a management_handle into a
 *                                  byte stream.
 * marshall_reply_type              Insert a reply_type into a byte stream.
 * marshall_uid                     Insert a uid into a byte stream.
 * marshall_mo_time                 Insert a mo_time into a byte stream.
 * marshall_mstatus                 Insert a mstatus into a byte stream.
 * sck_get_man_status               Interprets status for logging.
 * socket_log                       Write a message to syslog().
 *
 * MODULE INTERNAL FUNCTIONS:
 *
 * Function Name              Synopsis
 * --------------------       --------
 * 
 * None.
 */



/*
 *  Header files
 */

#include "man_data.h"
#include "man.h"
#include "nbase.h"
#include "socket.h"
#include "evd_defs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <pthread.h>
#include <exc_handling.h>

#if defined(__osf__)
/* KLUDGE.
 * On Ultrix pthread.h includes cma.h which includes dce/cma_errno.h
 * which includes errno.h.  On OSF dce/cma_errno.h is not present so we 
 * include errno.h here.
 */
#include <errno.h>
#else
/*
 * On OSF syslog.h contains the extern func declartion of syslog()
 * but Ultrix does not.
 */
extern void syslog();
#endif

extern int   cma_socket();
extern int   cma_connect();
extern int   cma_close();
extern void *cma_lib_malloc();
extern       cma_lib_free();
extern int   cma_write();
extern int   cma_read();
extern int   unlink();
extern int   setsockopt();

extern void object_id_from_xmit();
extern void avl_from_xmit();
extern void object_id_free_xmit();
extern void avl_free_xmit();

void       socket_log();
man_status connect_to_server();

/* 
 * The external variable sh is needed to share server 
 * information between rpc_*() functions because
 * I can't change the function's argument lists. 
 */
server_handle sh;

EXCEPTION rpc_x_comm_failure;



void rpc_server_unregister_if(p_if_spec, mgr_type_uuid, p_status)
unsigned int *p_if_spec,
             *p_status;
uuid_p_t      mgr_type_uuid;
/*
 * inputs:   p_if_spec     - Pointer to a character array which contains
 *                           the server's name.
 *
 *           mgr_type_uuid - Not used in the socket implementation.
 *
 *           p_status      - Pointer to the return status.
 *
 * outputs: *p_status      - The return status.
 *
 * description:   Unregister a server interface.  This function 
 *                closes a socket.
 */
{

  if (unlink(sh.name.sun_path) == -1)
    {
    *p_status = error_status_not_ok;
    }
  else
    {
    *p_status = error_status_ok;
    }

return;
}


void rpc_server_use_protseq(p_protseq, max_call_requests, p_status)
unsigned char *p_protseq;
unsigned int   max_call_requests,
              *p_status;
/*
 * This is a noop function in the socket implementation.
 */
{

  *p_status = error_status_ok;

return;
}



void rpc_server_inq_bindings(p_binding_vector, p_status)
rpc_binding_vector_p_t *p_binding_vector;
unsigned int           *p_status;

/*
 * inputs:  none.
 *
 * outputs: *p_binding_vector - Contains the address of the allocated memory.
 *          *p_status         - Contians the return status.
 *
 * description: This function allocates memory for a rpc_binding_vector_t
 *
 * side effects: A rpc_binding_vector_t struct is allocated, this memory
 *               MUST BE FREED by the caller. 
 */
{
  *p_binding_vector = NULL;
  
  *p_binding_vector =
       (rpc_binding_vector_t *) malloc(sizeof(rpc_binding_vector_t));

  if (*p_binding_vector != NULL)
    {
    (*p_binding_vector)->count = NULL;
    (*p_binding_vector)->binding_h[0] = NULL;
    *p_status = error_status_ok;
    }
  else
    {
    *p_status = error_status_not_ok;
    }

return;
}


void rpc_ep_register(p_if_spec, binding_vec, object_uuid_vec, p_annotation,
                     p_status)
unsigned int           *p_if_spec;
rpc_binding_vector_p_t  binding_vec;
uuid_vector_p_t         object_uuid_vec;
unsigned char          *p_annotation;
unsigned int           *p_status;
/*
 * This is a noop function in the socket implementation.
 */
{

  *p_status = error_status_ok;

return;
}



void rpc_ep_unregister(p_if_spec, binding_vec, object_uuid_vec, p_status)
unsigned int           *p_if_spec;
rpc_binding_vector_p_t  binding_vec;
uuid_vector_p_t         object_uuid_vec;
unsigned int           *p_status;

/*
 * This is a noop function in the socket implementation.
 */
{

  *p_status = error_status_ok;

return;
}


void rpc_binding_vector_free(p_binding_vector, p_status)
rpc_binding_vector_p_t *p_binding_vector;
unsigned int           *p_status;

/*
 * inputs:  p_binding_vector  - Address of a pointer to a 
 *                              rpc_binding_vector_t struct.
 *
 * outputs: *p_status         - Contains the return status.
 *
 * description: This function frees a rpc_binding_vector_t struct.
 *
 */
{

  if (*p_binding_vector != NULL)
  {
    free(*p_binding_vector);
  }

  *p_status = error_status_ok;

return;
}




void rpc_binding_from_string_binding(p_string_binding, p_binding_handle,
                                     p_status)
unsigned char        *p_string_binding;
rpc_binding_handle_t *p_binding_handle;
unsigned int         *p_status;

/*
 * inputs:  p_string_binding  - Pointer to the handle name string.
 *
 * outputs: *p_binding_handle - Contains the address of the allocated memory.
 *          *p_status         - Contians the return status.
 *
 * description: Return a binding handle from a string representation.
 *              This function allocates memory for a _handle_t struct
 *              and copies the value of p_string_binding into it.
 *
 * side effects: A _handle_t struct is allocated, this memory
 *               MUST BE FREED by the caller. 
 */
{
  *p_status = error_status_ok; 
  *p_binding_handle = NULL;
 
  *p_binding_handle =(handle_t) malloc(sizeof(struct _handle_t));
  memset((void *)*p_binding_handle, '\0', sizeof(struct _handle_t));

  if (*p_binding_handle != NULL)
    {
    if (p_string_binding != NULL)
      {
      strcpy((*p_binding_handle)->socket_address, 
             (char *) p_string_binding);
      }
    }
  else
    {
    *p_status = error_status_not_ok;
    } 
 
return;
}



void rpc_binding_free(p_binding_handle, p_status)
rpc_binding_handle_t *p_binding_handle;
unsigned int         *p_status;

/*
 * inputs:  p_binding_handle  - Address of a pointer to a _handle_t struct.
 *
 * outputs: *p_status         - Contains the return status.
 *
 * description: This function frees a rpc_binding_handle_t struct.
 *
 */
{
 
  if (*p_binding_handle != NULL)
    { 
    free(*p_binding_handle);
    }

  *p_status = error_status_ok;

return;
}


void rpc_binding_to_string_binding(binding_handle, pp_string_binding, 
                                   p_status)
rpc_binding_handle_t   binding_handle;
unsigned char        **pp_string_binding;
unsigned int          *p_status;
/*
 * inputs:  binding_handle     - Not used.
 *
 * outputs: *pp_string_binding - Contains the address of the allocated memory.
 *          *p_status          - Contains the return status.
 *
 * description: Return a string representation of a binding handle.
 *              This function allocates memory for a string binding and
 *              sets the string binding to the value contained in 
 *              external variable sh.
 *
 * side effects: Memory is allocated that MUST BE FREED by the caller. 
 */
{
  *pp_string_binding = NULL;

  *pp_string_binding = (unsigned char *) malloc(sizeof(sh.name.sun_path));

  if (*pp_string_binding != NULL)
    {
    memset((void *)*pp_string_binding, '\0', sizeof(sh.name.sun_path));

    strncpy((char *) *pp_string_binding, sh.name.sun_path, 
            sizeof(sh.name.sun_path));
    *p_status = error_status_ok;
    }
  else
    {
    *p_status = error_status_not_ok;
    }
  
return;
}



void rpc_string_free(pp_string, p_status)
unsigned char **pp_string;
unsigned int   *p_status;

/*
 * inputs:  pp_string  - Address of a pointer to a char array.
 *
 * outputs: *p_status  - Contains the return status.
 *
 * description: This function frees a char array.
 *
 */
{

  if (*pp_string != NULL)
  {
    free(*pp_string);
  }
  *p_status = error_status_ok;

return;
}


void rpc_binding_set_object(binding_handle, object_uuid, p_status)
rpc_binding_handle_t  binding_handle;
uuid_p_t              object_uuid;
unsigned int         *p_status;
/*
 * inputs:  binding_handle - Pointer to the server binding handle to be set.
 * 
 *          object_uuid    - UUID value to which the server binding handle 
 *                           will be set.
 *
 * outputs: *p_status  - Contains the return status.
 *
 * description: This function sets the object UUID value in a server
 *              binding handle. 
 *
 */
{
  strncpy(binding_handle->socket_address, SOCKET_PATH, sizeof(SOCKET_PATH));

  strcat(binding_handle->socket_address, (char *) object_uuid); 
  
  *p_status = error_status_ok;

return;
}



void rpc_string_binding_compose(p_string_object_uuid, p_string_protseq,
                                p_string_netaddr, p_string_endpoint,
                                p_string_options, pp_string_binding,
                                p_status)
unsigned char  *p_string_object_uuid;
unsigned char  *p_string_protseq;
unsigned char  *p_string_netaddr;
unsigned char  *p_string_endpoint;
unsigned char  *p_string_options;
unsigned char **pp_string_binding;
unsigned int   *p_status;
/*
 * This is a noop function in the socket implementation.
 */
{
 
  *pp_string_binding = NULL;
 
  *p_status = error_status_ok;

return;
}




void rpc_mgmt_stop_server_listening(binding_handle, p_status)
rpc_binding_handle_t  binding_handle;
unsigned int         *p_status;
/*
 * inputs:   binding_handle - Pointer to the handle of the server to stop.
 *
 * outputs: *p_status  - Contains the return status.
 *
 * description: Tell a server to stop listening for IPC calls.
 */
{
  man_status     status            = MAN_C_SUCCESS;
  signed int     stat              = 0;
  server_handle  stop_sh;
  char           buf               = SERVER_STOPPED,
                 msg[LINEBUFSIZE];
  size_t         buf_size          = sizeof(buf);

  *p_status = error_status_not_ok;
  stop_sh = sh;

  if ((stop_sh.s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
    sprintf(msg,
            MSG(sck_msg001, "S001 - Error during socket: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    status = MAN_C_PROCESSING_FAILURE;
   }

  if (status == MAN_C_SUCCESS)
    {
    /*
     * Conenct to the server.
     */
    stat = connect(stop_sh.s, (struct sockaddr *)&stop_sh.name,
                   sizeof(stop_sh.name));
    if (stat == -1)
      {
      sprintf(msg,
              MSG(sck_msg005, "S005 - Error during connect: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      status = MAN_C_PROCESSING_FAILURE;
      }
    }

  if (status == MAN_C_SUCCESS)
    {
    status = send_msg_to_server(stop_sh.s, buf_size, &buf);
    }

  close(stop_sh.s);

return;
}




void dce_error_inq_text(status_to_convert, p_error_text, p_status)
unsigned int  status_to_convert;
unsigned char *p_error_text;
int           *p_status;
/*
 * This is a noop function in the socket implementation.
 */
{

  *p_status = error_status_ok;

return;
}



man_status bind_socket(p_sh)
server_handle *p_sh;
/*
 * inputs:   p_sh   - Pointer to a server_handle.
 * 
 * outputs:  none 
 *
 * description:     This function binds a name to a socket and 
 *                  prepares to accept connections.
 */
{
  man_status  mstatus = MAN_C_SUCCESS;
  char        msg[LINEBUFSIZE]; 
  int	l_status;
  int	l_s;

  /*
   * Bind the name to the server end of socket.
   *	if the bind fails because the socket file was left behind by a departed server
   *	then remove the socket file and retry binding the socket and the address.
   *	This recovery logic eliminates the need for the user to rm /dev/eca_<server>
   *	before rerunning their mom after it crashes.
   */
  if ((l_status = bind (p_sh->s, &p_sh->name, p_sh->name_length)) == -1) {
    if (errno == EADDRINUSE) {
      if ((l_s = socket(AF_UNIX, SOCK_STREAM, 0)) != -1) {
	if ((connect(l_s, (struct sockaddr *)&p_sh->name, sizeof(p_sh->name))) == -1) {
	  close(l_s);
	  if (errno == ECONNREFUSED) {
	    unlink(p_sh->name.sun_path);
	    l_status = bind (p_sh->s, &p_sh->name, p_sh->name_length);
	  }
	}
	else close(l_s);
      }
    }

    if (l_status == -1) { /* Write to SYSLOG. */
      sprintf(msg,
	      MSG(sck_msg002, "S002 - Error during bind: '%s'\n"),
	      strerror(errno));
      socket_log(msg, LOG_ERR);
      mstatus = MAN_C_PROCESSING_FAILURE;
    }
  }

  /*
   * Prepare to accept connections, specify a backlog of 5.
   */
  if (mstatus == MAN_C_SUCCESS)
    listen (p_sh->s, 5); 
  
return(mstatus);
}



man_status connect_to_server(p_sh)
server_handle *p_sh;
/*
 * inputs:   p_sh->name.sun_path contains the name of the server to
 *           connect.
 *
 * outputs:  p_sh->s will contain the descriptor of the socket
 *           that was created.
 *            
 *           p_sh->name.sun_family will contain AF_UNIX.
 *
 * description:     This function creates a UNIX domain stream socket 
 *                  and connects to a server. 
 */
{
  man_status  mstatus = MAN_C_SUCCESS;
  signed int  stat = 0;
  char        msg[LINEBUFSIZE];

  /*
   * Create a UNIX domain stream socket.
   */

  if ((p_sh->s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
    /* Write to SYSLOG. */
    sprintf(msg,
            MSG(sck_msg001, "S001 - Error during socket: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    mstatus = MAN_C_PROCESSING_FAILURE;
    }

  if (mstatus == MAN_C_SUCCESS)
    {
    /*
     * Conenct to the server.
     */
    p_sh->name.sun_family = AF_UNIX;
    stat = connect(p_sh->s, (struct sockaddr *)&p_sh->name,
                   sizeof(p_sh->name));
    if (stat == -1)
      {
      sprintf(msg,
              MSG(sck_msg005, "S005 - Error during connect: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      mstatus = MAN_C_PROCESSING_FAILURE;
      }
    }

return(mstatus);
}



man_status connect_to_server_keepalive(p_sh)
server_handle *p_sh;
/*
 * inputs:   p_sh->name.sun_path contains the name of the server to
 *           connect.
 *
 * outputs:  p_sh->s will contain the descriptor of the socket
 *           that was created.
 *            
 *           p_sh->name.sun_family will contain AF_UNIX.
 *
 * description:     This function creates a UNIX domain stream socket 
 *                  and connects to a server. 
 */
{
#define SCK_TRUE 1

  man_status  mstatus   = MAN_C_SUCCESS;
  signed int  stat      = 0;
  int         optlen    = 4,            /* If < 4 alpha returns errno 22 */
              optname   = SO_KEEPALIVE;
  char        optval[4],
              msg[LINEBUFSIZE];

  /*
   * Create a UNIX domain stream socket.
   */

  if ((p_sh->s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
    sprintf(msg,
            MSG(sck_msg001, "S001 - Error during socket: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    mstatus = MAN_C_PROCESSING_FAILURE;
    }

  if (mstatus == MAN_C_SUCCESS)
    {
    optval[3] = SCK_TRUE;
    if (setsockopt(p_sh->s, SOL_SOCKET, optname, optval, optlen) == -1)
      {
      sprintf(msg,
              MSG(sck_msg001, "S001 - Error during socket: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      mstatus = MAN_C_PROCESSING_FAILURE;
      }
    }

  if (mstatus == MAN_C_SUCCESS)
    {
    /*
     * Conenct to the server.
     */
    p_sh->name.sun_family = AF_UNIX;
    stat = connect(p_sh->s, (struct sockaddr *)&p_sh->name,
                   sizeof(p_sh->name));
    if (stat == -1)
      {
      sprintf(msg,
              MSG(sck_msg005, "S005 - Error during connect: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      mstatus = MAN_C_PROCESSING_FAILURE;
      }
    }

return(mstatus);
}



man_status send_msg_to_server_get_status(sd, buf_size, p_buf)
unsigned int sd;
size_t       buf_size;
char        *p_buf;

/*
 * inputs:   sd       - socket descriptor of the socket we will write to.
 *           buf_size - size of the char buffer that will be sent.
 *           p_buf    - pointer to the char buffer that will be sent.
 *
 * outputs:  none.
 *
 * description:  This function writes two messages to the server.  First
 *               it writes a message containing the size of the following
 *               message, it then writes the message containing the routine
 *               to invoke and its arguments.  It then reads from the
 *               server the return status.
 */
{
  man_status  status    = MAN_C_SUCCESS;
  char        return_buf[sizeof(status)],
              msg[LINEBUFSIZE];
  /* 
   * First send a message containing the size of the following message.
   * This is done so that the server can allocate a buffer of the 
   * appropriate size.
   */

  if (write (sd, &buf_size, sizeof(size_t)) == -1)
    {
    sprintf(msg,
            MSG(sck_msg006, "S006 - Error during write: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    status = MAN_C_PROCESSING_FAILURE;
    }

  if (status == MAN_C_SUCCESS)
    {
    if (write (sd, p_buf, buf_size) == -1)
      {
      sprintf(msg,
              MSG(sck_msg006, "S006 - Error during write: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      status = MAN_C_PROCESSING_FAILURE;
      }
    }
  
  if (status == MAN_C_SUCCESS)
    {
    /* Read the return value. */

    if (read (sd, return_buf, sizeof(return_buf)) == -1)
      {
      sprintf(msg,
              MSG(sck_msg004, "S004 - Error during read: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      status = MAN_C_PROCESSING_FAILURE;
      }
    else
      {
      memcpy((void *) &status, return_buf, sizeof(status));
      }
    }

return(status);
}



man_status send_msg_to_server(sd, buf_size, p_buf)
unsigned int sd;
size_t       buf_size;
char        *p_buf;

/*
 * inputs:   sd       - socket descriptor of the socket we will write to.
 *           buf_size - size of the char buffer that will be sent.
 *           p_buf    - pointer to the char buffer that will be sent.
 *
 * outputs:  none.
 *
 * description:  This function writes two messages to the server.  First
 *               it writes a message containing the size of the following
 *               message, it then writes the message containing the routine
 *               to invoke and its arguments.  
 */
{
  man_status  status    = MAN_C_SUCCESS;
  char        msg[LINEBUFSIZE];
  
  /* 
   * First send a message containing the size of the following message.
   * This is done so that the server can allocate a buffer of the 
   * appropriate size.
   */

  if (write (sd, &buf_size, sizeof(size_t)) == -1)
    {
    sprintf(msg,
            MSG(sck_msg006, "S006 - Error during write: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    status = MAN_C_PROCESSING_FAILURE;
    }

  if (status == MAN_C_SUCCESS)
    {
    if (write (sd, p_buf, buf_size) == -1)
      {
      sprintf(msg,
              MSG(sck_msg006, "S006 - Error during write: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      status = MAN_C_PROCESSING_FAILURE;
      }
    }
  
return(status);
}



man_status read_msg_from_client(fd, pp_buf)
unsigned int   fd;
char         **pp_buf;

/*
 * inputs:   fd - file descriptor for the socket we are reading.
 *           pp_buf - Address of a pointer to a char buffer that will
 *                    be allocated and filled.
 *
 * outputs:  *pp_buf - Pointer to the char buffer that has been allocated.
 *
 * description:  This function reads two messages from the client.  First
 *               it reads a message containing the size of the following
 *               message, it then allocates memory to hold the following 
 *               message and then reads the message containing the routine
 *               to invoke and its arguments.  THIS FUNCTION ALLOCATES 
 *               MEMORY THAT MUST BE FREED BY THE CALLER.
 */
{
  man_status  status         = MAN_C_SUCCESS;
  int         stat           = 0;
  size_t      msg_size       = 0;
  char        size_buf[sizeof(size_t)],
              msg[LINEBUFSIZE];

  memset((void *)size_buf, '\0', sizeof(size_t));

  stat = read(fd, size_buf, sizeof(size_t));
  if (stat == -1)
    {
    sprintf(msg,
            MSG(sck_msg004, "S004 - Error during read: '%s'\n"),
            strerror(errno));
    socket_log(msg, LOG_ERR);
    status = MAN_C_PROCESSING_FAILURE;
    }

  if (status == MAN_C_SUCCESS)
    {
    /*
     * Copy from the buffer the size of the message that will be sent.
     * If no process has the pipe open for writing, read returns zero to
     * indicate end-of-file.
     */
    memcpy((void *) &msg_size, size_buf, sizeof(size_t));
    if (msg_size > 0)
      {
      *pp_buf = (char *) malloc(msg_size);
      }
    else
      {
      status = MAN_C_PROCESSING_FAILURE;
      }
    }

  if (status == MAN_C_SUCCESS)
    {
    stat = read(fd, *pp_buf, msg_size);
    if (stat == -1)
      {
      sprintf(msg,
              MSG(sck_msg004, "S004 - Error during read: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      status = MAN_C_PROCESSING_FAILURE;
      }
    }

return(status);
}



man_status unmarshall_man_binding_handle(pp_buf_idx, p_handle)
char               **pp_buf_idx;
man_binding_handle  *p_handle;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_handle is the address of a pointer to the socket address.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_handle is allocated and needs to be freed by the 
 *          caller.
 *
 * description:  This function copies the handle from the byte stream. 
 *               
 * side effects: A handle_t struct is allocated, its address returned
 *               in *p_handle.  THIS MUST BE FREED by the caller. 
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;


  /* Copy from the buffer the size of the handle. */

  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of the handle, a handle_t struct
   * is allocated that the caller needs to free. 
   */

  GET_MAN_BINDING_HANDLE_ARG(*p_handle, *pp_buf_idx, arg_size, status);

return(status);
}



man_status unmarshall_object_id(pp_buf_idx, p_object_id)
char         **pp_buf_idx;
object_id     *p_object_id;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_object_id is the address of an object_id.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          p_object_id, memory is allocated and attached to p_object_id.
 *
 * description:  This function copies the object id from the byte stream.
 *
 * side effects: memory is allocated, its address returned in
 *               p_handle->value.  THIS MUST BE FREED by the caller.
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  obj_id_trans_t *p_xmit_obj     = NULL;
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of obj_class. */

  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the flattened value of obj_class. */

  GET_OBJECT_ID_ARG(p_xmit_obj, *pp_buf_idx, arg_size, status);

  /* 
   * Memory is allocated by object_id_from_xmit and attached to p_object_id.
   * p_object_id->value needs to be freed by the caller.
   */

  if (status == MAN_C_SUCCESS)
    {
    object_id_from_xmit(p_xmit_obj, p_object_id);

    if (p_xmit_obj != NULL)
      {
      free(p_xmit_obj);
      }
    }

return(status);
}



man_status unmarshall_avl(pp_buf_idx, p_avl)
char         **pp_buf_idx;
avl           *p_avl;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_avl is the address of an avl.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_avl, memory is allocated and attached to *p_avl.
 *
 * description:  This function copies the avl from the byte stream.
 *
 * side effects: Memory is allocated, its address returned 
 *               in *p_avl.  THIS MUST BE FREED by the caller.
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  avl_trans_t    *p_avl_xmit_obj = NULL;
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of the avl. */

  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the flattened value of the avl. */

  GET_AVL_ARG(p_avl_xmit_obj, *pp_buf_idx, arg_size, status);

  /* 
   * Memory is allocated by avl_from_xmit() and attached to 
   * p_avl that needs to be freed by the caller.
   */

  if (status == MAN_C_SUCCESS)
    {
    avl_from_xmit(p_avl_xmit_obj, p_avl);

    if (p_avl_xmit_obj != NULL)
      {
      free(p_avl_xmit_obj);
      }
    }

return(status);
}



man_status unmarshall_scope(pp_buf_idx, p_scope)
char         **pp_buf_idx;
scope         *p_scope;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_scope is the address of a scope.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_scope, the value of the unmarshalled arg is put here.
 *
 * description:  This function copies the scope from the byte stream.
 *
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of scope. */

  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of scope. */

  GET_INT_ARG(*p_scope, *pp_buf_idx, arg_size);

return(status);
}



man_status unmarshall_int(pp_buf_idx, p_int)
char     **pp_buf_idx;
int       *p_int;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_int is the address of an int.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_int, the value of the unmarshalled arg is put here.
 *
 * description:  This function copies the int from the byte stream.
 *
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of int. */
  
  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of int. */

  GET_INT_ARG(*p_int, *pp_buf_idx, arg_size);

return(status);
}



man_status unmarshall_unsigned_int(pp_buf_idx, p_int)
char              **pp_buf_idx;
unsigned int       *p_int;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_int is the address of an int.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_int, the value of the unmarshalled arg is put here.
 *
 * description:  This function copies the int from the byte stream.
 *
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of int. */
  
  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of int. */

  GET_INT_ARG(*p_int, *pp_buf_idx, arg_size);

return(status);
}



man_status unmarshall_man_handle(pp_buf_idx, p_man_handle)
char              **pp_buf_idx;
management_handle  *p_man_handle;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_man_handle is the address of a management_handle. 
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_man_handle, the value of the unmarshalled arg is put
 *          here. 
 *
 * description:  This function copies the management_handle from the 
 *               byte stream.
 *
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of the management handle. */

  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of the management handle. */

  GET_MAN_HANDLE_ARG(p_man_handle, *pp_buf_idx, arg_size);

return(status);
}



man_status unmarshall_reply_type(pp_buf_idx, p_reply)
char              **pp_buf_idx;
reply_type         *p_reply;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_reply is the address of a reply type.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_reply, the value of the unmarshalled arg is put here.
 *
 * description:  This function copies the reply From the byte stream.
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of the reply. */
  
  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of the reply. */

  GET_INT_ARG(*p_reply, *pp_buf_idx, arg_size);

return(status);
}



man_status unmarshall_uid(pp_buf_idx, p_object_uid)
char              **pp_buf_idx;
uid                *p_object_uid;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_object_uid is the address of a uid.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_object_uid, the value of the unmarshalled arg is put here.
 *
 * description:  This function copies the uid from the byte stream.
 *
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of the uid. */
  
  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of the uid. */

  GET_UID_ARG(p_object_uid, *pp_buf_idx, arg_size);

return(status);
}



man_status unmarshall_mo_time(pp_buf_idx, p_time)
char          **pp_buf_idx;
mo_time        *p_time;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_time is the address of a mo_time.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_time, the value of the unmarshalled arg is put here.
 *
 * description:  This function copies the mo_time from the byte stream.
 *
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of the reply. */
  
  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of the reply. */

  GET_TIME_ARG(p_time, *pp_buf_idx, arg_size);

return(status);
}



man_status unmarshall_mstatus(pp_buf_idx, p_mstatus)
char              **pp_buf_idx;
man_status         *p_mstatus;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_mstatus is the address of a mstatus.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_mstatus, the value of the unmarshalled arg is put here.
 *
 * description:  This function copies the mstatus from the byte stream.
 *
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of the mstatus. */

  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of the mstatus. */

  GET_MSTATUS_ARG(p_mstatus, *pp_buf_idx, arg_size);

return(status);
}



man_status unmarshall_evd_queue_handle(pp_buf_idx, p_handle)
char              **pp_buf_idx;
evd_queue_handle   *p_handle;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_handle is the address of a evd_queue_handle.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_handle, the value of the unmarshalled arg is put here.
 *
 * description:  This function copies the evd_queue_handle from the 
 *               byte stream.
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of evd_queue_handle. */
  
  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of evd_queue_handle. */

  GET_QUEUE_HANDLE_ARG(p_handle, *pp_buf_idx, arg_size, status);

return(status);
}



man_status unmarshall_access_mode(pp_buf_idx, p_access_mode)
char                  **pp_buf_idx;
evd_queue_access_mode  *p_access_mode;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream that is to be unmarshalled.
 *
 *          p_access_mode is the address of an evd_queue_access_mode.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 *          *p_access_mode, the value of the unmarshalled arg is put here.
 *
 * description:  This function copies the evd_queue_access_mode from the 
 *               byte stream.
 *
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy from the buffer the size of access_mode. */
  
  GET_SIZE_T_ARG(arg_size, *pp_buf_idx, size_t_size);

  /* Copy from the buffer the value of access_mode. */

  GET_ACCESS_MODE_ARG(p_access_mode, *pp_buf_idx, arg_size);

return(status);
}



man_status marshall_man_binding_handle(pp_buf_idx, p_handle, handle_size)
char               **pp_buf_idx;
man_binding_handle   p_handle;
size_t               handle_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream.
 *
 *          p_handle is a pointer to the socket address.
 *
 *          handle_size is the size of the handle;
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the handle into the byte stream. 
 */               
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of the handle. */

  PUT_SIZE_T_ARG(*pp_buf_idx, handle_size, size_t_size);

  /* Copy into the buffer the value of the handle. */

  PUT_MAN_BINDING_HANDLE_ARG(*pp_buf_idx, p_handle, handle_size);
                                                              
return(status);
}



man_status marshall_object_id(pp_buf_idx, p_xmit_obj, object_id_size)
char           **pp_buf_idx;
obj_id_trans_t  *p_xmit_obj;
size_t           object_id_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream.
 *
 *          p_xmit_obj is a pointer to the flattened object.
 *
 *          object_id_size is the size of the flattened object.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the flalttend object id into the 
 *               byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of the flattened object. */

  PUT_SIZE_T_ARG(*pp_buf_idx, object_id_size, size_t_size);

  /* Copy into the buffer the value of the object id. */

  PUT_OBJECT_ID_ARG(*pp_buf_idx, p_xmit_obj, object_id_size);


return(status);
}



man_status marshall_avl(pp_buf_idx, p_avl_xmit_obj, avl_size)
char         **pp_buf_idx;
avl_trans_t   *p_avl_xmit_obj;
size_t         avl_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream.
 *
 *          p_avl_xmit_obj is a pointer to a flattened avl.
 *
 *          avl_size is the size of the avl.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the flattened avl into the byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of the avl. */

  PUT_SIZE_T_ARG(*pp_buf_idx, avl_size, size_t_size);

  /* Copy into the buffer the value of the avl. */

  PUT_AVL_ARG(*pp_buf_idx, p_avl_xmit_obj, avl_size);

return(status);
}



man_status marshall_scope(pp_buf_idx, scope_data, scope_size)
char         **pp_buf_idx;
scope          scope_data;
size_t         scope_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream.
 *
 *          scope_data is the value of the scope.
 *
 *          scope_size is the size of the scope.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the scope into the byte stream.
 */
{
  size_t          arg_size       = 0,
                  size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of the int. */

  PUT_SIZE_T_ARG(*pp_buf_idx, scope_size, size_t_size);

  /* Copy into the buffer the value of the int. */

  PUT_INT_ARG(*pp_buf_idx, scope_data, scope_size);

return(status);
}



man_status marshall_int(pp_buf_idx, int_data, int_size)
char     **pp_buf_idx;
int        int_data;
size_t     int_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream.
 *
 *          int_data is the address of an int.
 *
 *          int_size is the size of the int.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the int into the byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of the int. */

  PUT_SIZE_T_ARG(*pp_buf_idx, int_size, size_t_size);

  /* Copy into the buffer the value of the int. */

  PUT_INT_ARG(*pp_buf_idx, int_data, int_size);

return(status);
}



man_status marshall_unsigned_int(pp_buf_idx, int_data, int_size)
char              **pp_buf_idx;
unsigned int        int_data;
size_t              int_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream.
 *
 *          int_data is the address of an int.
 *
 *          int_size is the size of the int.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the int into the byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of the int. */

  PUT_SIZE_T_ARG(*pp_buf_idx, int_size, size_t_size);

  /* Copy into the buffer the value of the int. */

  PUT_INT_ARG(*pp_buf_idx, int_data, int_size);

return(status);
}



man_status marshall_man_handle(pp_buf_idx, p_man_handle, man_handle_size)
char              **pp_buf_idx;
management_handle  *p_man_handle;
size_t              man_handle_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location
 *          in the byte stream..
 *
 *          p_man_handle is the address of a management_handle. 
 *
 *          man_handle_size is the size of *p_man_handle
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the management_handle from the 
 *               byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of man_handle. */

  PUT_SIZE_T_ARG(*pp_buf_idx, man_handle_size, size_t_size);

  /* Copy into the buffer the value of man_handle. */

  PUT_MAN_HANDLE_ARG(*pp_buf_idx, p_man_handle, man_handle_size);

return(status);
}



man_status marshall_reply_type(pp_buf_idx, reply, reply_size)
char              **pp_buf_idx;
reply_type          reply;
size_t              reply_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream..
 *
 *          reply is the value of the reply.
 *
 *          reply_size is the size of the reply type.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the reply into the byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of reply. */

  PUT_SIZE_T_ARG(*pp_buf_idx, reply_size, size_t_size);

  /* Copy into the buffer the value of reply. */

  PUT_INT_ARG(*pp_buf_idx, reply, reply_size);

return(status);
}



man_status marshall_uid(pp_buf_idx, p_uid, uid_size)
char              **pp_buf_idx;
uid                *p_uid;
size_t              uid_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream.
 *
 *          p_uid is a pointer to a uid.
 *
 *          uid_size is the size of the uid.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the uid into the byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of the uid. */

  PUT_SIZE_T_ARG(*pp_buf_idx, uid_size, size_t_size);

  /* Copy into the buffer the value of the uid. */

  PUT_UID_ARG(*pp_buf_idx, p_uid, uid_size);

return(status);
}



man_status marshall_mo_time(pp_buf_idx, p_time, time_size)
char          **pp_buf_idx;
mo_time        *p_time;
size_t          time_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location 
 *          in the byte stream.
 *
 *          p_time is the address of mo_time.
 *
 *          time_size is the size of mo_time.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the mo_time into the byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of mo_time. */

  PUT_SIZE_T_ARG(*pp_buf_idx, time_size, size_t_size);

  /* Copy into the buffer the value of mo_time. */

  if (p_time != NULL)
    {
    PUT_TIME_ARG(*pp_buf_idx, p_time, time_size);
    }

return(status);
}



man_status marshall_mstatus(pp_buf_idx, p_mstatus, mstatus_size)
char        **pp_buf_idx;
man_status   *p_mstatus;
size_t        mstatus_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location
 *          in the byte stream.
 *
 *          p_mstatus is the address of a mstatus.
 *
 *          mstatus_size is the size of the mstatus.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the int into the byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of the mstatus. */

  PUT_SIZE_T_ARG(*pp_buf_idx, mstatus_size, size_t_size);

  /* Copy into the buffer the value of the mstatus. */

  PUT_MSTATUS_ARG(*pp_buf_idx, p_mstatus, mstatus_size);

return(status);
}



man_status marshall_evd_queue_handle(pp_buf_idx, pp_handle, handle_size)
char              **pp_buf_idx;
evd_queue_handle  **pp_handle;
size_t              handle_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location
 *          in the byte stream.
 *
 *          pp_handle is the address of a pointer to an evd queue handle. 
 * 
 *          handle_size is the size of *p_handle.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the queue handle from the 
 *               byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of evd queue handle. */

  PUT_SIZE_T_ARG(*pp_buf_idx, handle_size, size_t_size);

  /* Copy into the buffer the value of the queue handle. */

  PUT_QUEUE_HANDLE_ARG(*pp_buf_idx, pp_handle, handle_size);

return(status);
}



man_status marshall_access_mode(pp_buf_idx, p_access_mode, access_mode_size)
char                  **pp_buf_idx;
evd_queue_access_mode  *p_access_mode;
size_t                  access_mode_size;

/*
 * inputs:  pp_buf_idx is the address of a pointer to the location
 *          in the byte stream.
 *
 *          p_access_mode is a pointer to an evd_queue_access_mode.
 *
 *          access_mode_size is the size of access_mode.
 *
 * outputs: *pp_buf_idx is incremented by the size of the argument.
 *
 * description:  This function copies the queue handle from the 
 *               byte stream.
 */
{
  size_t          size_t_size    = sizeof(size_t);
  man_status      status         = MAN_C_SUCCESS;

  /* Copy into the buffer the size of the evd_queue_access_mode. */

  PUT_SIZE_T_ARG(*pp_buf_idx, access_mode_size, size_t_size);

  /* Copy into the buffer the value of the evd_queue_access_mode. */

  PUT_QUEUE_HANDLE_ARG(*pp_buf_idx, p_access_mode, access_mode_size);

return(status);
}



char *sck_get_man_status(code)
int code;
/*
 * Function description:
 *
 *    Gets the ASCII-string representation of a man_status
 *
 * Arguments:
 *
 *    code          man_status to display
 *
 * Return values:
 *
 *    None.
 *
 * Side effects:
 *
 *    None.
 */
{
static  char value[100];

/*
 *  Local
 */
static
char *man_status_table_1[] = {
    "MAN_C_NO_SUCH_CLASS" ,
    "MAN_C_NO_SUCH_OBJECT_INSTANCE" ,
    "MAN_C_ACCESS_DENIED" ,
    "MAN_C_SYNC_NOT_SUPPORTED" ,
    "MAN_C_INVALID_FILTER" ,
    "MAN_C_NO_SUCH_ATTRIBUTE_ID" ,
    "MAN_C_INVALID_ATTRIBUTE_VALUE" ,
    "MAN_C_GET_LIST_ERROR" ,
    "MAN_C_SET_LIST_ERROR" ,
    "MAN_C_NO_SUCH_ACTION" ,
    "MAN_C_PROCESSING_FAILURE" ,
    "MAN_C_DUPLICATE_M_O_INSTANCE" ,
    "MAN_C_NO_SUCH_REFERENCE_OBJECT" ,
    "MAN_C_NO_SUCH_EVENT_TYPE" ,
    "MAN_C_NO_SUCH_ARGUMENT" ,
    "MAN_C_INVALID_ARGUMENT_VALUE" ,
    "MAN_C_INVALID_SCOPE" ,
    "MAN_C_INVALID_OBJECT_INSTANCE" ,
    "MAN_C_MISSING_ATTRIBUTE_VALUE" ,
    "MAN_C_CLASS_INSTANCE_CONFLICT" ,
    "MAN_C_COMPLEXITY_LIMITATION" ,
    "MAN_C_MISTYPED_OPERATION" ,
    "MAN_C_NO_SUCH_INVOKE_ID" ,
    "MAN_C_OPERATION_CANCELLED" ,
    "MAN_C_INVALID_OPERATION" ,
    "MAN_C_INVALID_OPERATOR" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "MAN_C_DIRECTIVE_NOT_SUPPORTED" ,
    "MAN_C_ENTITY_CLASS_NOT_SUPPORTED" ,
    "MAN_C_INVALID_USE_OF_WILDCARD" ,
    "UNKNOWN reply code" ,
    "MAN_C_CONSTRAINT_VIOLATION" ,
    "MAN_C_WRITE_ONLY_ATTRIBUTE" ,
    "MAN_C_READ_ONLY_ATTRIBUTE" ,
    "MAN_C_DUPLICATE_ATTRIBUTE" ,
    "MAN_C_DUPLICATE_ARGUMENT" ,
    "UNKNOWN" ,
    "MAN_C_REQUIRED_ARGUMENT_OMITTED" ,
    "MAN_C_FILTER_INVALID_FOR_ACTION" ,
    "MAN_C_INSUFFICIENT_RESOURCES" ,
    "MAN_C_NO_SUCH_ATTRIBUTE_GROUP" ,
    "MAN_C_FILTER_USED_WITH_CREATE"
} ;

static
char *man_status_table_2[] = {
    "MAN_C_WILD_NOT_AT_LOWEST_LEVEL" ,
    "MAN_C_WILD_CLASS_WITH_FILTER" ,
    "MAN_C_WILD_INVALID_DIRECTIVE" ,
    "MAN_C_WILD_WITH_CREATE" ,
    "MAN_C_WILD_INVALID_GROUP" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "MAN_C_SCOPE_TOO_COMPLEX" ,
    "MAN_C_SYNC_TOO_COMPLEX" ,
    "MAN_C_FILTER_TOO_COMPLEX"
} ;

static
char *man_status_table_3[] = {
     "MAN_C_ALREADY_INITIALIZED" ,
     "MAN_C_BAD_PARAMETER" ,
     "MAN_C_FAILURE" ,
     "MAN_C_HANDLE_NOT_BOUND" ,
     "MAN_C_HAS_ACTIVE_CHILDREN" ,
     "MAN_C_MO_TIMEOUT" ,
     "MAN_C_MOLD_TIMEOUT" ,
     "MAN_C_NO_ELEMENT" ,
     "MAN_C_NO_MOLD" ,
     "MAN_C_NO_REPLY" ,
     "MAN_C_NO_SUCH_PARENT_CLASS" ,
     "MAN_C_NOT_CONSTRUCTED" ,
     "MAN_C_NOT_INITIALIZED" ,
     "MAN_C_OBJECT_ALREADY_EXISTS" ,
     "MAN_C_PE_TIMEOUT" ,
     "MAN_C_READ_ONLY"
 } ;

static
char *man_status_table_4[] = {
    "MAN_C_EQUAL" ,
    "MAN_C_TRUE" ,
    "MAN_C_NOT_EQUAL" ,
    "MAN_C_FALSE"
} ;

static
char *man_status_table_5[] = {
    "MAN_C_NOT_SUPPORTED" ,
    "MAN_C_END_OF_MIB"
} ;

  if (code == (int)MAN_C_SUCCESS)
      return( "MAN_C_SUCCESS" ) ;
  else
  if ( (code > (int)MAN_C_SUCCESS) &&
      (code <= (int)MAN_C_FILTER_USED_WITH_CREATE ) )
      return( man_status_table_1[ code ] ) ;
  else
  if ( (code >= (int)MAN_C_WILD_NOT_AT_LOWEST_LEVEL) &&
      (code <= (int)MAN_C_FILTER_TOO_COMPLEX) )
      return( man_status_table_2[ code - 1000 ] ) ;
  else
  if ( (code >= (int)MAN_C_ALREADY_INITIALIZED) &&
      (code <= (int)MAN_C_READ_ONLY) )
      return( man_status_table_3[ code - 1200 ] ) ;
  else
  if ( (code >= (int)MAN_C_EQUAL) &&
      (code <= (int)MAN_C_FALSE) )
      return( man_status_table_4[ code - 1300 ] ) ;
  else
  if ( (code >= (int)MAN_C_NOT_SUPPORTED) &&
      (code <= (int)MAN_C_END_OF_MIB) )
      return( man_status_table_5[ code - 2000 ] ) ;
  else {
      sprintf(value, "<UKNOWN man_status code %d>", code );
      return( value );
      }
}



void socket_log(p_msg, syslog_code)
char *p_msg;         
int   syslog_code;  
/*
 * inputs:   p_msg is a pointer to the message to be written to the log. 
 *
 *           syslog_code is the code needed for syslog call.
 *
 * outputs:  None.
 *
 * description:  Write a message to syslog().
 */
{
char    bigbuf[MAXERRMSG+1];/* Where we copy a message to add a newline*/
                            /* (+1 is for the NULL byte) */
char    *outmsg;            /* Pointer to final message to send */
int     in_msg_len;         /* Computed length of inbound message */

  /* if (it doesn't end in a newline and there is room) */

  if ( ( (in_msg_len = strlen(p_msg)) <  MAXERRMSG ) &&
         ( *(p_msg + in_msg_len - 1)    != '\n'     )     )
    {
    /* add a newline */

    strcpy(bigbuf, p_msg);            /* copy inbound message */
    bigbuf[in_msg_len] = '\n';      /* add '\n' over null byte */
    bigbuf[in_msg_len+1] = '\0';    /* add the '\0' */
    outmsg = bigbuf;                /* New outbound message is here */
    } 
  else 
    {
    outmsg = p_msg;   /* Just use the original message */
    }

  /* write message to syslog() */

  syslog(syslog_code, outmsg);

return;
}
