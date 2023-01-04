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
static char *rcsid = "@(#)$RCSfile: sck_evd_cstb.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 16:00:15 $";
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
 * Common Agent EVD client stub.
 *
 * Module SCK_EVD_CSTB.C
 *
 * WRITTEN BY:
 *   Enterprise Management Frameworks
 *   Pat Mulligan  February 1993 
 *
 * Overview:
 *  The Common Agent EVD client stub provides stubs for the three
 *  evd functions; evd_create_queue_handle, evd_delete_queue_handle 
 *  and evd_post_event.  These stubs are liked into MOMs to 
 *  provide transparent IPC function calls to EVD.
 *  Each stub marshalls the functions arguments, connects to the evd  
 *  server and transmits the function name and argumets and receives
 *  a return response.  This response is then return to the caller.
 *
 * History
 *      V1.0    February 1993  Pat Mulligan
 *
 * MODULE CONTENTS:
 *
 * USER-LEVEL INTERFACE (Functions defined in this module for
 *                       reference elsewhere):
 *
 * Function Name            Synopsis
 * --------------------     --------
 * evd_create_queue_handle  Client stub for evd_create_queue_handle.
 * evd_delete_queue_handle  Client stub for evd_delete_queue_handle.
 * evd_post_event           Client stub for evd_post_event.
 * 
 * INTERNAL FUNCTIONS:
 *
 * Function Name            Synopsis
 * -----------------------  --------
 * None. 
 */



/*
 *  Header files
 */

#include "man_data.h"
#include "man.h"             /* Management Return Codes */
#include "socket.h"
#include "evd_defs.h"
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
extern int   cma_lib_free();
extern void *cma_lib_malloc();

extern void object_id_to_xmit();
extern void object_id_free_xmit();
extern void avl_to_xmit();
extern void avl_free_xmit();
extern void avl_from_xmit();

static char evdc_ifspec[] = "snmp_pe";
char *evdc_v11_0_s_ifspec = evdc_ifspec;



man_status evd_create_queue_handle(pp_evd_handle, 
                                   p_queue_name, 
                                   access_mode)
evd_queue_handle      **pp_evd_handle;
avl                    *p_queue_name;
evd_queue_access_mode   access_mode;

/*
 * inputs:  pp_evd_handle is the address of a pointer to the queue handle
 *          that will be created.
 *          p_queue_name is a pointer to the queue_name should be NULL
 *          for now.
 *          access_mode is EVD_POST for now.
 *
 * outputs: *pp_evd_handle is a pointer to the queue handle created.
 *
 * description:   This function marshalls the evd function's arguments,
 *                connects to the evd server and sends the name of the
 *                function to be invoked along with its arguments and 
 *                receives a return status and return arguments back.
 */
{
#define NUM_CREATE_ARGS     3

  man_status        status            = MAN_C_SUCCESS,
                    evd_status        = MAN_C_SUCCESS;
  server_handle     evd_sh;
  size_t            handle_size       = 0,
                    queue_name_size   = 0,
                    access_mode_size  = sizeof(access_mode),
                    size_t_size       = sizeof(size_t),
                    buf_size          = 0;
  avl_trans_t      *p_avl_xmit        = NULL;
  evd_queue_handle  tmp_evd_handle    = 0,
                   *p_tmp_evd_handle  = &tmp_evd_handle;
  char             *p_buf             = NULL,
                   *p_buf_idx         = NULL,
                    return_buf[(2 * sizeof(size_t)) + sizeof(man_status) + 
                               sizeof(evd_queue_handle)],
                    msg[LINEBUFSIZE];

  /* 
   * Determine the number of bytes to send.  First flatten
   * the AVL into a transmittable format and find its size. 
   */
  if (p_queue_name != NULL)
    {
    avl_to_xmit(p_queue_name, &p_avl_xmit);
    if (p_avl_xmit != NULL)
      {
      queue_name_size = p_avl_xmit->buf_len * sizeof(p_avl_xmit->avl);
      }
    }

  /*
   * Determine the total size of the buffer to send. 
   * Because queue_handle is an output argument it has no value
   * to marshall so its size will not be included in the count.
   */
  buf_size = 1 + (NUM_CREATE_ARGS * size_t_size) + queue_name_size + 
             access_mode_size;

  p_buf = (char *) malloc(buf_size);
  p_buf_idx = p_buf;

  /* Copy into the buffer the code of the function to be invoked. */

  *p_buf_idx = EVD_CREATE_QUEUE_HANDLE;
  p_buf_idx++;

  /*
   * Copy into the buffer the arguments. 
   * Because evd_queue_handle is an output argument it has no value
   * to marshall.  Just copy into the buffer a size of zero which acts 
   * as a place holder.
   */

  PUT_SIZE_T_ARG(p_buf_idx, handle_size, size_t_size);

  status = marshall_avl(&p_buf_idx, p_avl_xmit, queue_name_size);

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_access_mode(&p_buf_idx, &access_mode, access_mode_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    /* 
     * Build the socket name and connect to the evd server. 
     */
    memset((void *)evd_sh.name.sun_path, '\0', sizeof(evd_sh.name.sun_path));
    strncpy(evd_sh.name.sun_path, SOCKET_PATH, sizeof(SOCKET_PATH));
    strcat(evd_sh.name.sun_path, (char *) evdc_ifspec);
    status = connect_to_server(&evd_sh);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = send_msg_to_server(evd_sh.s, buf_size, p_buf);
    }

  if (status == MAN_C_SUCCESS)
    {
    /* Read the return value. */

    if (read (evd_sh.s, return_buf, sizeof(return_buf)) == -1)
      {
      sprintf(msg,
              MSG(sck_msg004, "S004 - Error during read: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      status = MAN_C_PROCESSING_FAILURE;
      }
    else
      {
      /* Copy from the buffer the return status and handle value. */

      p_buf_idx = (void *) return_buf;      
      status = unmarshall_mstatus(&p_buf_idx, &evd_status);
      if (status == MAN_C_SUCCESS)
        {
        status = unmarshall_evd_queue_handle(&p_buf_idx, p_tmp_evd_handle);
        *pp_evd_handle = (evd_queue_handle *) *p_tmp_evd_handle;
        }
      }
    }

  close(evd_sh.s);

  /* Release allocated memory. */

  if (p_buf != NULL) free(p_buf);

  if (status == MAN_C_SUCCESS)
    {
    status = evd_status;
    }

return(status);
}



man_status evd_delete_queue_handle(pp_evd_handle)
evd_queue_handle **pp_evd_handle;

/*
 * inputs:  pp_evd_handle is the address of a pointer to the queue handle
 *          that will be delete.
 *
 * outputs: None.
 *
 * description:   This function marshalls the evd function's arguments,
 *                connects to the evd server and sends the name of the
 *                function to be invoked along with its arguments and 
 *                receives a return status and return arguments back.
 */
{
#define NUM_DELETE_ARGS 1

  man_status      status           = MAN_C_SUCCESS;
  server_handle   evd_sh;
  size_t          handle_size      = 0,
                  size_t_size      = sizeof(size_t),
                  buf_size         = 0;
  char           *p_buf            = NULL,
                 *p_buf_idx        = NULL,
                  return_buf[sizeof(size_t) + sizeof(man_status)],
                  msg[LINEBUFSIZE];

  /* Determine the total size of the buffer to send. */
 
  if (pp_evd_handle != NULL) handle_size = sizeof(*pp_evd_handle);

  buf_size = 1 + (NUM_DELETE_ARGS * size_t_size) + handle_size;

  p_buf = (char *) malloc(buf_size);
  p_buf_idx = p_buf;

  /* Copy into the buffer the function code and arguments. */

  *p_buf_idx = EVD_DELETE_QUEUE_HANDLE;
  p_buf_idx++;

  /*
   * WARNING
   * We marshall and send the value of the pointer to evd_queue_handle
   * not the value of evd_queue_handle as one might expect.
   */

  status = marshall_evd_queue_handle(&p_buf_idx, pp_evd_handle, handle_size);

  if (status == MAN_C_SUCCESS)
    {
    /* 
     * Build the socket name and connect to the evd server. 
     */
    memset((void *)evd_sh.name.sun_path, '\0', sizeof(evd_sh.name.sun_path));
    strncpy(evd_sh.name.sun_path, SOCKET_PATH, sizeof(SOCKET_PATH));
    strcat(evd_sh.name.sun_path, (char *) evdc_ifspec);
    status = connect_to_server(&evd_sh);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = send_msg_to_server(evd_sh.s, buf_size, p_buf);
    }

  if (status == MAN_C_SUCCESS)
    {
    /* Read the return value. */

    if (read (evd_sh.s, return_buf, sizeof(return_buf)) == -1)
      {
      sprintf(msg,
              MSG(sck_msg004, "S004 - Error during read: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      status = MAN_C_PROCESSING_FAILURE;
      }
    else
      {
      /* Copy from the buffer the return status. */

      p_buf_idx = (void *) return_buf;      
      status = unmarshall_mstatus(&p_buf_idx, &status);
      }
    }

  close(evd_sh.s);

  if (p_buf != NULL) free(p_buf);

return(status);
}


man_status evd_post_event(p_evd_handle,
                          p_object_class, 
                          p_instance_name,
                          p_event_time, 
                          p_event_type, 
                          p_event_parameters,
                          p_event_uid, 
                          p_mo_uid)
evd_queue_handle *p_evd_handle;
object_id        *p_object_class,
                 *p_event_type;
avl              *p_instance_name,
                 *p_event_parameters;
mo_time          *p_event_time;
uid              *p_event_uid,
                 *p_mo_uid;

/*
 * inputs:  p_evd_handle is a pointer to the evd queue handle.
 *          p_object_class is a pointer to the class of the poster.
 *          p_event_type is a pointer to the id of the event type.
 *          p_instance_name is a pointer to the instance of the poster.
 *          p_event_parameters is a pointer to the parameters of the event.
 *          p_event_time is a pointer to the timestamp of the evnet.
 *          p_event_uid is a pointer to the unique identifier of the event.
 *          p_mo_uid is a pointer to the unique identifier of the poster.
 *          
 *
 * outputs: *p_event_uid is an unique identifier for the event set by EVD. 
 *
 * description:   This function marshalls the evd function's arguments,
 *                connects to the evd server and sends the name of the
 *                function to be invoked along with its arguments and 
 *                receives a return status and return arguments back.
 */
{
#define NUM_POST_ARGS      8

  man_status      status           = MAN_C_SUCCESS,
                  evd_status       = MAN_C_SUCCESS;
  server_handle   evd_sh;
  size_t          handle_size      = 0,
                  class_size       = 0,
                  instance_size    = 0,
                  time_size        = 0,
                  type_size        = 0,
                  param_size       = 0,
                  uid_size         = 0,
                  mo_uid_size      = 0,
                  size_t_size      = sizeof(size_t),
                  buf_size         = 0;
  avl_trans_t    *p_avl_name_xmit  = NULL,
                 *p_avl_param_xmit = NULL;
  obj_id_trans_t *p_obj_class_xmit = NULL,
                 *p_obj_type_xmit  = NULL;
  char           *p_buf            = NULL,
                 *p_buf_idx        = NULL,
                  return_buf[sizeof(size_t) + sizeof(man_status) +
                             sizeof(size_t) + sizeof(uid)],
                  msg[LINEBUFSIZE];

  /* 
   * Determine the number of bytes to send.  
   */
  if (p_evd_handle != NULL) handle_size = sizeof(*p_evd_handle);
  if (p_event_time != NULL) time_size   = sizeof(*p_event_time);
  if (p_event_uid  != NULL) uid_size    = sizeof(*p_event_uid);
  if (p_mo_uid     != NULL) mo_uid_size = sizeof(*p_mo_uid);

  /*
   * First flatten the AVLs into a transmittable format and find their sizes. 
   */
  if (p_instance_name != NULL)
    {
    avl_to_xmit(p_instance_name, &p_avl_name_xmit);
    if (p_avl_name_xmit != NULL)
      {
      instance_size = p_avl_name_xmit->buf_len * sizeof(p_avl_name_xmit->avl);
      }
    }
  if (p_event_parameters != NULL)
    {
    avl_to_xmit(p_event_parameters, &p_avl_param_xmit);
    if (p_avl_param_xmit != NULL)
      {
      param_size = p_avl_param_xmit->buf_len * sizeof(p_avl_param_xmit->avl);
      }
    }

  /* 
   * Flatten the object_ids into a transmittable format and find 
   * their sizes.
   */
  if (p_object_class != NULL)
    {
    object_id_to_xmit(p_object_class, &p_obj_class_xmit);
    if (p_obj_class_xmit != NULL)
      {
      class_size = p_obj_class_xmit->count * sizeof(p_obj_class_xmit->value) +
                   sizeof(p_obj_class_xmit->count);
      }
    }
  if (p_event_type != NULL)
    {
    object_id_to_xmit(p_event_type, &p_obj_type_xmit);
    if (p_obj_type_xmit != NULL)
      {
      type_size = p_obj_type_xmit->count * sizeof(p_obj_type_xmit->value) +
                  sizeof(p_obj_type_xmit->count);
      }
    }

  /* Determine the total size of the buffer to send. */
 
  buf_size = 1 + (NUM_POST_ARGS * size_t_size) + handle_size +
             class_size + instance_size + time_size + type_size +
             param_size + uid_size + mo_uid_size;

  p_buf = (char *) malloc(buf_size);
  p_buf_idx = p_buf;

  /* 
   * Copy into the buffer the function code and arguments. 
   */

  *p_buf_idx = EVD_POST_EVENT;
  p_buf_idx++;

  /*
   * WARNING 
   * We marshall and send the value of the pointer to evd_queue_handle
   * not the value of evd_queue_handle as one might expect.
   */
  status = marshall_evd_queue_handle(&p_buf_idx, &p_evd_handle, handle_size);

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_object_id(&p_buf_idx, p_obj_class_xmit, class_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_avl(&p_buf_idx, p_avl_name_xmit, instance_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_mo_time(&p_buf_idx, p_event_time, time_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_object_id(&p_buf_idx, p_obj_type_xmit, type_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_avl(&p_buf_idx, p_avl_param_xmit, param_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_uid(&p_buf_idx, p_event_uid, uid_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_uid(&p_buf_idx, p_mo_uid, mo_uid_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    /* 
     * Build the socket name and connect to the evd server. 
     */
    memset((void *)evd_sh.name.sun_path, '\0', sizeof(evd_sh.name.sun_path));
    strncpy(evd_sh.name.sun_path, SOCKET_PATH, sizeof(SOCKET_PATH));
    strcat(evd_sh.name.sun_path, (char *) evdc_ifspec);
    status = connect_to_server(&evd_sh);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = send_msg_to_server(evd_sh.s, buf_size, p_buf);
    }

  if (status == MAN_C_SUCCESS)
    {
    /* Read the return value. */

    if (read (evd_sh.s, return_buf, sizeof(return_buf)) == -1)
      {
      sprintf(msg,
              MSG(sck_msg004, "S004 - Error during read: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      status = MAN_C_PROCESSING_FAILURE;
      }
    else
      {
      /* Copy from the buffer the return status and output argument. */

      p_buf_idx = (void *) return_buf;      
      status = unmarshall_mstatus(&p_buf_idx, &evd_status);
      if (status == MAN_C_SUCCESS)
        {
        status = unmarshall_uid(&p_buf_idx, p_event_uid);
        }
      }
    }

  close(evd_sh.s);

  /* Release allocated memory. */

  if (p_buf != NULL) free(p_buf);

  if (status == MAN_C_SUCCESS)
    {
    status = evd_status;
    }

return(status);
}
