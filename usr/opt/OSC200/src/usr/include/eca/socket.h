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
/*
 * @(#)$RCSfile: socket.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:49:08 $
 */
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
 * Common Agent socket IPC implementation include file.
 *
 * Module SOCKET.H
 *      Contains data structure definitions required by the 
 *      socket IPC implementation.
 *
 * WRITTEN BY:
 *    Enterprise Management Frameworks
 *    V1.0  December 1992  Pat Mulligan
 *
 * Context:
 *    This module is included into the compilations of modules that
 *    comprise the socket IPC implementation.
 *
 * Purpose:
 *    This module contains the internal data structure definitions
 *    required by the socket IPC modules.
 *
 * History
 *      V1.0    December 1992  Pat Mulligan
 */

#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include "evd_defs.h"

/* Define the prototypes for the socket IPC support routines. */

man_status bind_socket                  (server_handle *p_sh);
man_status connect_to_server            (server_handle *p_sh);
man_status connect_to_server_keepalive  (server_handle *p_sh);
man_status send_msg_to_server_get_status(unsigned int sd,
                                         size_t       buf_size,
                                         char        *p_buf);
man_status send_msg_to_server           (unsigned int sd,
                                         size_t       buf_size,
                                         char        *p_buf);
man_status read_msg_from_client         (unsigned int   fd,
                                         char         **pp_buf);
man_status unmarshall_man_binding_handle(char                **pp_buf_idx,
                                         man_binding_handle   *p_handle);
man_status unmarshall_object_id         (char         **pp_buf_idx,
                                         object_id     *p_object_id);
man_status unmarshall_avl               (char         **pp_buf_idx,
                                         avl           *p_avl);
man_status unmarshall_scope             (char         **pp_buf_idx,
                                         scope         *p_scope);
man_status unmarshall_int               (char         **pp_buf_idx,
                                         int           *p_int);
man_status unmarshall_unsigned_int      (char         **pp_buf_idx,
                                         unsigned int  *p_int);
man_status unmarshall_man_handle        (char             **pp_buf_idx,
                                         management_handle *p_man_handle);
man_status unmarshall_reply_type        (char         **pp_buf_idx,
                                         reply_type    *p_reply);
man_status unmarshall_uid               (char         **pp_buf_idx,
                                         uid           *p_object_uid);
man_status unmarshall_mo_time           (char         **pp_buf_idx,
                                         mo_time       *p_time);
man_status unmarshall_mstatus           (char         **pp_buf_idx,
                                         man_status    *p_mstatus);
man_status unmarshall_evd_queue_handle  (char              **pp_buf_idx,
                                         evd_queue_handle   *p_handle);
man_status unmarshall_access_mode       (char                  **pp_buf_idx,
                                         evd_queue_access_mode  *p_access_mode);
man_status marshall_man_binding_handle  (char               **pp_buf_idx,
                                         man_binding_handle   p_handle,
                                         size_t               handle_size);
man_status marshall_object_id           (char           **pp_buf_idx,
                                         obj_id_trans_t  *p_xmit_obj,
                                         size_t           object_id_size);
man_status marshall_avl                 (char         **pp_buf_idx,
                                         avl_trans_t   *p_avl_xmit_obj,
                                         size_t         avl_size);
man_status marshall_scope               (char         **pp_buf_idx,
                                         scope          scope_data,
                                         size_t         scope_size);
man_status marshall_int                 (char         **pp_buf_idx,
                                         int            int_data,
                                         size_t         int_size);
man_status marshall_unsigned_int        (char         **pp_buf_idx,
                                         unsigned int   int_data,
                                         size_t         int_size);
man_status marshall_man_handle          (char              **pp_buf_idx,
                                         management_handle  *p_man_handle,
                                         size_t              man_handle_size);
man_status marshall_reply_type          (char              **pp_buf_idx,
                                         reply_type          reply,
                                         size_t              reply_size);
man_status marshall_uid                 (char              **pp_buf_idx,
                                         uid                *p_uid,
                                         size_t              uid_size);
man_status marshall_mo_time             (char              **pp_buf_idx,
                                         mo_time            *p_time,
                                         size_t              time_size);
man_status marshall_mstatus             (char              **pp_buf_idx,
                                         man_status         *p_mstatus,
                                         size_t              mstatus_size);
man_status marshall_evd_queue_handle    (char              **pp_buf_idx,
                                         evd_queue_handle  **pp_handle,
                                         size_t              handle_size);
man_status marshall_access_mode         (char                 **pp_buf_idx,
                                         evd_queue_access_mode *p_access_mode,
                                         size_t               access_mode_size);
char *sck_get_man_status(int code);
void socket_log(char *p_msg, int syslog_code);

/* Functions that print translatable text. */

extern char *sck_msg001();
extern char *sck_msg002();
extern char *sck_msg003();
extern char *sck_msg004();
extern char *sck_msg005();
extern char *sck_msg006();
extern char *sck_msg007();
extern char *sck_msg008();
extern char *sck_msg009();
extern char *sck_msg010();
extern char *sck_msg011();
extern char *sck_msg012();
extern char *sck_msg013();
extern char *sck_msg014();
extern char *sck_msg015();
extern char *sck_msg016();
extern char *sck_msg017();
extern char *sck_msg018();
extern char *sck_msg019();

/* 
 * This symbol is used to specify the path name of the unix domain sockets.
 */

#define SOCKET_PATH "/dev/eca_"

/* 
 * These symbols are used to declare the size of character arrays into which
 * error messages are built.  
 */
#define LINEBUFSIZE 250
#define MAXERRMSG 300

/* Define a MACRO to be used for displaying translatable text */

#define MSG(msg_name, string)  msg_name()

/* 
 * Define MACROs to be used to marshall and unmarshall arguments.
 */

/*
 * Copy into the buffer the size of the parameter.
 */
#define PUT_SIZE_T_ARG(p_buf_idx, arg_size, size_t_size)               \
  memcpy(p_buf_idx, (void *) &arg_size, size_t_size);                  \
  p_buf_idx = p_buf_idx + size_t_size; 

/* 
 * Copy from the buffer the size of the parameter. 
 */
#define GET_SIZE_T_ARG(arg_size, p_buf_idx, size_t_size)               \
  arg_size = 0;                                                        \
  memcpy((void *) &arg_size, p_buf_idx, size_t_size);                  \
  p_buf_idx = p_buf_idx + size_t_size;

/*
 * Copy into the buffer a man_binding_handle.
 */
#define PUT_MAN_BINDING_HANDLE_ARG(p_buf_idx, handle, handle_size)     \
  memcpy(p_buf_idx, handle, handle_size);                              \
  p_buf_idx = p_buf_idx + handle_size;

/* 
 * Copy from the buffer a man_binding_handle.
 * GET_MAN_BINDING_HANDLE_ARG() allocates memory that must be freed by 
 * the caller.
 */
#define GET_MAN_BINDING_HANDLE_ARG(handle, p_buf_idx,                  \
                                   handle_size, status)                \
  handle = NULL;                                                       \
  handle = (man_binding_handle) malloc(sizeof(struct _handle_t));      \
  if (handle != NULL)                                                  \
    {                                                                  \
    memcpy(handle, p_buf_idx, handle_size);                            \
    p_buf_idx = p_buf_idx + handle_size;                               \
    }                                                                  \
  else                                                                 \
    {                                                                  \
    status = MAN_C_INSUFFICIENT_RESOURCES;                             \
    }                                                                  \

/*
 * Copy into the buffer a man_handle.
 */
#define PUT_MAN_HANDLE_ARG(p_buf_idx, p_man_handle, man_handle_size)   \
  memcpy(p_buf_idx, p_man_handle, man_handle_size);                    \
  p_buf_idx = p_buf_idx + man_handle_size;

/*
 * Copy from the buffer a man_handle.
 */
#define GET_MAN_HANDLE_ARG(p_man_handle, p_buf_idx, man_handle_size)   \
  memcpy(p_man_handle, p_buf_idx, man_handle_size);                    \
  p_buf_idx = p_buf_idx + man_handle_size;

/*
 * Copy into the buffer the flattened value of an object id. 
 * PUT_OBJECT_ID_ARG() frees p_xmit_obj.
 */
#define PUT_OBJECT_ID_ARG(p_buf_idx, p_xmit_obj, obj_id_size)          \
  memcpy(p_buf_idx, (void *) p_xmit_obj, obj_id_size);                 \
  if (p_xmit_obj != NULL) object_id_free_xmit(p_xmit_obj);             \
  p_buf_idx = p_buf_idx + obj_id_size;

/*
 * Copy from the buffer the flattened value of an object id. 
 * GET_OBJECT_ID_ARG() allocates memory that must be freed by the caller.
 */
#define GET_OBJECT_ID_ARG(p_xmit_obj, p_buf_idx, obj_id_size,          \
                          status)                                      \
  p_xmit_obj = NULL;                                                   \
  p_xmit_obj = (obj_id_trans_t *) malloc(obj_id_size);                 \
  if (p_xmit_obj != NULL)                                              \
    {                                                                  \
    memcpy((void *) p_xmit_obj, p_buf_idx, obj_id_size);               \
    p_buf_idx = p_buf_idx + obj_id_size;                               \
    }                                                                  \
  else                                                                 \
    {                                                                  \
    status = MAN_C_INSUFFICIENT_RESOURCES;                             \
    }                                                                  \

/*
 * Copy into the buffer an int.
 */
#define PUT_INT_ARG(p_buf_idx, int_arg, int_size)                      \
  memcpy(p_buf_idx, (void *) &int_arg, int_size);                      \
  p_buf_idx = p_buf_idx + int_size;          

/* 
 * Copy from the buffer an int.
 */
#define GET_INT_ARG(arg, p_buf_idx, int_size)                          \
  memcpy((void *) &arg, p_buf_idx, int_size);                          \
  p_buf_idx = p_buf_idx + int_size;

/*
 * Copy into the buffer the flattened value of the avl. 
 * PUT_AVL_ARG() frees p_xmit_obj. 
 */
#define PUT_AVL_ARG(p_buf_idx, p_xmit_obj, obj_id_size)                \
  memcpy(p_buf_idx, (void *) p_xmit_obj, obj_id_size);                 \
  if (p_xmit_obj != NULL) avl_free_xmit(p_xmit_obj);                   \
  p_buf_idx = p_buf_idx + obj_id_size;

/*
 * Copy from the buffer the flattened value of the avl. 
 * GET_AVL_ARG allocates memory that must be freed by the caller.
 */
#define GET_AVL_ARG(p_xmit_obj, p_buf_idx, obj_id_size, status)        \
  p_xmit_obj = NULL;                                                   \
  if (obj_id_size != 0)                                                \
    {                                                                  \
    p_xmit_obj = (avl_trans_t *) malloc(obj_id_size);                  \
    if (p_xmit_obj != NULL)                                            \
      {                                                                \
      memcpy((void *) p_xmit_obj, p_buf_idx, obj_id_size);             \
      }                                                                \
    else                                                               \
      {                                                                \
      status = MAN_C_INSUFFICIENT_RESOURCES;                           \
      }                                                                \
    }                                                                  \
  else                                                                 \
    {                                                                  \
    p_xmit_obj = (avl_trans_t *) malloc(sizeof(avl_trans_t));          \
    if (p_xmit_obj != NULL)                                            \
      {                                                                \
      memset((void *) p_xmit_obj, '\0', sizeof(avl_trans_t)) ;         \
      }                                                                \
    else                                                               \
      {                                                                \
      status = MAN_C_INSUFFICIENT_RESOURCES;                           \
      }                                                                \
    }                                                                  \
  p_buf_idx = p_buf_idx + obj_id_size;

/*
 * Copy into the buffer a uid.
 */
#define PUT_UID_ARG(p_buf_idx, p_arg, uid_size)                        \
  memcpy(p_buf_idx, (void *) p_arg, uid_size);                         \
  p_buf_idx = p_buf_idx + uid_size;          

/*
 * Copy from the buffer a uid.
 */
#define GET_UID_ARG(p_arg, p_buf_idx, uid_size)                        \
  memcpy((void *) p_arg, p_buf_idx, uid_size);                         \
  p_buf_idx = p_buf_idx + uid_size;

/*
 * Copy into the buffer a mo_time.
 */
#define PUT_TIME_ARG(p_buf_idx, p_arg, time_size)                      \
  memcpy(p_buf_idx, (void *) p_arg, time_size);                        \
  p_buf_idx = p_buf_idx + time_size;          

/*
 * Copy from the buffer a mo_time
 */
#define GET_TIME_ARG(p_arg, p_buf_idx, time_size)                      \
  memcpy((void *) p_arg, p_buf_idx, time_size);                        \
  p_buf_idx = p_buf_idx + time_size;

/* 
 * Copy into the buffer a man_status.
 */
#define PUT_MSTATUS_ARG(p_buf_idx, p_arg, mstatus_size)                \
  memcpy(p_buf_idx, (void *) p_arg, mstatus_size);                     \
  p_buf_idx = p_buf_idx + mstatus_size;

/*
 * Copy from the buffer a man_status.
 */
#define GET_MSTATUS_ARG(p_arg, p_buf_idx, mstatus_size)                \
  memcpy((void *) p_arg, p_buf_idx, mstatus_size);                     \
  p_buf_idx = p_buf_idx + mstatus_size;

/*
 * Copy into the buffer a pointer to an evd_queue_handle.
 */
#define PUT_QUEUE_HANDLE_ARG(p_buf_idx, p_handle, handle_size)         \
  memcpy(p_buf_idx, p_handle, handle_size);                            \
  p_buf_idx = p_buf_idx + handle_size;

/* 
 * Copy from the buffer a pointer to an evd_queue_handle.
 * We have sent the value of the pointer to an evd_queue_handle
 * not the value of an evd_queue_handle as one might expect.
 * If we every decide to pass an evd_queue_handle instead of the 
 * pointer to an evd_queue_handle as we do now then we must use the 
 * currently commented out version of the GET_QUEUE_HANDLE_ARG macro.
 */
#define GET_QUEUE_HANDLE_ARG(p_handle, p_buf_idx, handle_size, status) \
  memcpy((void *) p_handle, p_buf_idx, handle_size);                   \
  p_buf_idx = p_buf_idx + handle_size;

/* 
 * GET_QUEUE_HANDLE_ARG() allocates memory that must be freed by 
 * the caller.
 *
 * p_handle = NULL;                                                     \
 * p_handle = malloc(sizeof(evd_queue_handle));                         \
 * if (p_handle != NULL)                                                \
 *   {                                                                  \
 *   memcpy((void *) p_handle, p_buf_idx, handle_size);                 \
 *   p_buf_idx = p_buf_idx + handle_size;                               \
 *   }                                                                  \
 * else                                                                 \
 *   {                                                                  \
 *   status = MAN_C_INSUFFICIENT_RESOURCES;                             \
 *   }                                                                  \
 */

/*
 * Copy into the buffer an access_mode.
 */
#define PUT_ACCESS_MODE_ARG(p_buf_idx, p_access_mode, mode_size)       \
  memcpy(p_buf_idx, p_access_mode, mode_size);                         \
  p_buf_idx = p_buf_idx + mode_size;

/* 
 * Copy from the buffer an access_mode.
 */
#define GET_ACCESS_MODE_ARG(p_access_mode, p_buf_idx, mode_size)       \
  memcpy((void *) p_access_mode, p_buf_idx, mode_size);                \
  p_buf_idx = p_buf_idx + mode_size;  

#endif
