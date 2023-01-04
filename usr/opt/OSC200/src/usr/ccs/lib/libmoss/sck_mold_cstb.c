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
static char *rcsid = "@(#)$RCSfile: sck_mold_cstb.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 16:00:45 $";
#endif
/*
 **  Copyright (c) Digital Equipment Corporation, 1993  
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
 * Common Agent MOLD client stub.
 *
 * Module SCK_MOLD_CSTB.C
 *
 * WRITTEN BY:
 *   Enterprise Management Frameworks
 *   Pat Mulligan  December 1992
 *
 * Overview:
 *  The Common Agent MOLD client stub provides stubs for the four
 *  mold functions; mold_register_mom, mold_deregister_mom,
 *  mold_find_mom and mold_find_next_mom.  These stubs are liked into
 *  the PEs and MOMs to provide transparent IPC function calls.
 *  Each stub marshalls the functions arguments, connects to the mold
 *  server and transmits the function name and argumets and receives
 *  a return response.  This response is then return to the caller.
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
 * mold_register_mom       Client stub for mold_register_mom.
 * 
 * mold_deregister_mom     Client stub for mold_deregister_mom.
 *
 * mold_find_mom           Client stub for mold_find_mom.
 *
 * mold_find_next_mom      Client stub for mold_find_next_mom.
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
extern void avl_free_inst();

extern man_status moss_avl_copy_all();



man_status mold_register_mom(
                  mold_handle,
                  p_man_handle,
                  p_parent_id,
                  p_mom_id,
                  registration_type,
                  supported_interface,
                  pid
                  )
man_binding_handle mold_handle;          
management_handle *p_man_handle;         
object_id         *p_parent_id;          
object_id         *p_mom_id;             
unsigned int       registration_type;    
unsigned int       supported_interface; 
unsigned int       pid;                 
/*
 * inputs:  mold_handle  is a MOLD binding handle.
 *          p_man_handle is a pointer to the management handle.
 *          p_parent_id  is a pointer to the parent class object id.
 *          p_mom_id     is a pointer to the object id of the object being
 *                       registered.
 *          registration_type is a bit-mask that represents the type(s)
 *                       of insertion required.
 *          supported_interface is a bit-mask that represents the
 *                       type(s) of interfaces supported.
 *          pid          is the process identification of the managed
 *                       object module.
 *
 * outputs: none.
 *
 * description:   This function marshalls the mold function's arguments,
 *                connects to the mold server and sends the name of the
 *                function to be invoked along with its arguments and 
 *                receive a return status.
 */
{
#define NUM_REGISTER_ARGS 7

  man_status      status           = MAN_C_SUCCESS;
  server_handle   mold_sh;
  size_t          mold_handle_size = 0,
                  man_handle_size  = 0,
                  reg_type_size    = 0,
                  supp_int_size    = 0,
                  pid_size         = 0,
                  par_id_size      = 0,
                  mom_id_size      = 0,
                  size_t_size      = sizeof(size_t),
                  buf_size         = 0;
  obj_id_trans_t *p_par_xmit_obj   = NULL,
                 *p_mom_xmit_obj   = NULL;
  char           *p_buf            = NULL,
                 *p_buf_idx        = NULL;

  /* Flatten the object_id into a transmittable format and find its size.*/

  if (p_parent_id != NULL)
    {
    object_id_to_xmit(p_parent_id, &p_par_xmit_obj);
    if (p_par_xmit_obj != NULL)
      {
      par_id_size = p_par_xmit_obj->count * sizeof(p_par_xmit_obj->value) +
                    sizeof(p_par_xmit_obj->count);
      }
    }

  if (p_mom_id != NULL)
    {
    object_id_to_xmit(p_mom_id, &p_mom_xmit_obj);
    if (p_mom_xmit_obj != NULL)
      {
      mom_id_size = p_mom_xmit_obj->count * sizeof(p_mom_xmit_obj->value) +
                    sizeof(p_mom_xmit_obj->count);
      }
    }

  /* Determine the number of bytes to send. */

  if (mold_handle != NULL)  mold_handle_size = sizeof(*mold_handle);
  if (p_man_handle != NULL) man_handle_size  = sizeof(*p_man_handle);
  reg_type_size = sizeof(registration_type);
  supp_int_size = sizeof(supported_interface);
  pid_size = sizeof(pid);

  /* Determine the total size of the buffer to send. */
 
  buf_size = 1 + (NUM_REGISTER_ARGS * size_t_size) + mold_handle_size +
             man_handle_size + par_id_size + mom_id_size +
             reg_type_size + supp_int_size + pid_size;

  p_buf = (char *) malloc(buf_size);
  p_buf_idx = p_buf;

  /* Copy into the buffer the code of the function to be invoked. */

  *p_buf_idx = MOLD_REGISTER_MOM;
  p_buf_idx++;

  /* Copy into the buffer the flattened arguments. */

  status = marshall_man_binding_handle(&p_buf_idx, mold_handle,
                                     mold_handle_size);

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_man_handle(&p_buf_idx, p_man_handle,
                                 man_handle_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_object_id(&p_buf_idx, p_par_xmit_obj, par_id_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_object_id(&p_buf_idx, p_mom_xmit_obj, mom_id_size);
    }
 
  if (status == MAN_C_SUCCESS)
    {
    status = marshall_unsigned_int(&p_buf_idx, registration_type, 
                                   reg_type_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_unsigned_int(&p_buf_idx, supported_interface, 
                                   supp_int_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_unsigned_int(&p_buf_idx, pid, pid_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    /* Connect to mold server. */
    memset((void *)mold_sh.name.sun_path, '\0', sizeof(mold_sh.name.sun_path));
    strcpy(mold_sh.name.sun_path, mold_handle->socket_address);
    status = connect_to_server(&mold_sh);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = send_msg_to_server_get_status(mold_sh.s, buf_size, p_buf);
    }

  close(mold_sh.s);

  if (p_buf != NULL)
    {
    free(p_buf);
    }

return(status);
}



man_status mold_deregister_mom(
                    mold_handle,
                    p_man_handle,
                    p_obj_class
                    )
man_binding_handle  mold_handle;    
management_handle  *p_man_handle;  
object_id          *p_obj_class;    
/*
 * inputs:  mold_handle  is a MOLD binding handle.
 *          p_man_handle is a pointer to the management handle.
 *          p_obj_class  is a pointer to the object class.
 *
 * outputs: none.
 *
 * description:   This function marshalls the mold function's arguments,
 *                connects to the mold server and sends the name of the
 *                function to be invoked along with its arguments and
 *                receive a return status.
 */
{
#define NUM_DEREGISTER_ARGS 3

  man_status      status           = MAN_C_SUCCESS;
  server_handle   mold_sh;
  size_t          mold_handle_size = 0,
                  man_handle_size  = 0,
                  obj_class_size   = 0,
                  size_t_size      = sizeof(size_t),
                  buf_size         = 0;
  obj_id_trans_t *p_xmit_obj       = NULL;
  char           *p_buf            = NULL,
                 *p_buf_idx        = NULL;

  /* Determine the number of bytes to send. */

  if (mold_handle != NULL)  mold_handle_size = sizeof(*mold_handle);
  if (p_man_handle != NULL) man_handle_size  = sizeof(*p_man_handle);

  /* Flatten the object_id into transmittable format and find its size.*/

  if (p_obj_class != NULL)
    {
    object_id_to_xmit(p_obj_class, &p_xmit_obj);
    if (p_xmit_obj != NULL)
      {
      obj_class_size = p_xmit_obj->count * sizeof(p_xmit_obj->value) +
                       sizeof(p_xmit_obj->count);
      }
    }

  /* Determine the total size of the buffer to send. */

  buf_size = 1 + (NUM_DEREGISTER_ARGS * size_t_size) + mold_handle_size +
             man_handle_size + obj_class_size;

  p_buf = (char *) malloc(buf_size);
  p_buf_idx = p_buf;

  /* Copy into the buffer the code of the function that will be invoked. */

  *p_buf_idx = MOLD_DEREGISTER_MOM;
  p_buf_idx++;

  /* Copy into the buffer the flattened arguments. */

  status = marshall_man_binding_handle(&p_buf_idx, mold_handle,
                                       mold_handle_size);
 
  if (status == MAN_C_SUCCESS)
    {
    status = marshall_man_handle(&p_buf_idx, p_man_handle, man_handle_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_object_id(&p_buf_idx, p_xmit_obj, obj_class_size);
    }
 
  if (status == MAN_C_SUCCESS)
    {
    /* Connect to mold server. */
    memset((void *)mold_sh.name.sun_path, '\0', sizeof(mold_sh.name.sun_path));
    strcpy(mold_sh.name.sun_path, mold_handle->socket_address);
    status = connect_to_server(&mold_sh);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = send_msg_to_server_get_status(mold_sh.s, buf_size, p_buf);
    }

  close(mold_sh.s);

  if (p_buf != NULL)
    {
    free(p_buf);
    }

return(status);
}



man_status mold_find_mom(
              mold_handle,
              p_base_class,
              containment_level,
              p_return_info
              )
man_binding_handle  mold_handle;        
object_id          *p_base_class;       
unsigned int        containment_level; 
avl                *p_return_info;     
/*
 * inputs:  mold_handle   is a MOLD binding handle.
 *          p_base_class  is a pointer to the base class.
 *          containment_level is an integer specifying the level of the
 *                        tree to search (relative to base class).]
 *          p_return_info is a pointer to a return AVL handle.
 *
 * outputs: *p_return_info is the return AVL value.
 *
 * description:   This function marshalls the mold function's arguments,
 *                connects to the mold server and sends the name of the
 *                function to be invoked along with its arguments and
 *                receive a return status.
 */
{
#define MAX_RET_BUF_SIZE 1024
#define NUM_FIND_ARGS 4

  static int           pe_mold_connected = FALSE;
  static server_handle pe_mold_sh;

  man_status      status                     = MAN_C_SUCCESS,
                  find_mom_status            = MAN_C_SUCCESS;
  int             stat                       = 0;
  size_t          mold_handle_size           = 0,
                  base_class_size            = 0,
                  containment_size           = 0,
                  return_info_size           = 0,
                  size_t_size                = sizeof(size_t),
                  buf_size                   = 0;
  obj_id_trans_t *p_base_xmit_obj            = NULL;
  avl_trans_t    *p_return_xmit_obj          = NULL;
  avl             tmp_return,
                 *p_tmp_return               = &tmp_return;
  char           *p_buf                      = NULL,
                 *p_buf_idx                  = NULL,
                  ret_buf[MAX_RET_BUF_SIZE],
                  msg[LINEBUFSIZE];

  memset((void *)&tmp_return, '\0', sizeof(tmp_return));
 
  /* Determine the number of bytes to send. */

  if (mold_handle != NULL) mold_handle_size = sizeof(*mold_handle);
  containment_size = sizeof(containment_level);

  /* Flatten the object_id into a transmittable format and find its size. */

  if (p_base_class != NULL)
    {  
    object_id_to_xmit(p_base_class, &p_base_xmit_obj);
    if (p_base_xmit_obj != NULL)
      {
      base_class_size = p_base_xmit_obj->count * 
                        sizeof(p_base_xmit_obj->value) +
                        sizeof(p_base_xmit_obj->count);
      }
    }

  /* Flatten the AVL into a transmittable format and find its size. */

  if (p_return_info != NULL)
    {
    avl_to_xmit(p_return_info, &p_return_xmit_obj);
    if (p_return_xmit_obj != NULL)
      {
      return_info_size = p_return_xmit_obj->buf_len *
                         sizeof(p_return_xmit_obj->avl); 
      }
    }

  /* Determine the total size of the buffer to send. */

  buf_size = 1 + (NUM_FIND_ARGS * size_t_size) + mold_handle_size +
             base_class_size + containment_size + return_info_size;

  p_buf = (char *) malloc(buf_size);
  p_buf_idx = p_buf;

  /* Copy into the buffer the function that the server will invoke. */

  *p_buf_idx = MOLD_FIND_MOM;
  p_buf_idx++;

  /* Copy into the buffer the flattened arguments. */

  status = marshall_man_binding_handle(&p_buf_idx, mold_handle,
                                       mold_handle_size);

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_object_id(&p_buf_idx, p_base_xmit_obj, base_class_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_unsigned_int(&p_buf_idx, containment_level, 
                                   containment_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_avl(&p_buf_idx, p_return_xmit_obj, return_info_size);
    }

  if ((status == MAN_C_SUCCESS) && (!pe_mold_connected))
    {
    /* 
     * Connect to the mold server. 
     */
    memset((void *)pe_mold_sh.name.sun_path, '\0', 
           sizeof(pe_mold_sh.name.sun_path));
    strcpy(pe_mold_sh.name.sun_path, mold_handle->socket_address);
    status = connect_to_server_keepalive(&pe_mold_sh);
    if (status == MAN_C_SUCCESS)
      {
      pe_mold_connected = TRUE;
      }
    }

  if (status == MAN_C_SUCCESS)
    {
    status = send_msg_to_server(pe_mold_sh.s, buf_size, p_buf);
    }

  if (status == MAN_C_SUCCESS)
    {
    /* Read the return value. */
    if (read (pe_mold_sh.s, ret_buf, sizeof(ret_buf)) == -1)
      {
      sprintf(msg,
              MSG(sck_msg004, "S004 - Error during read: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      stat = -1;
      status = MAN_C_PROCESSING_FAILURE;
      }
    else
      {
      /* Copy from the buffer the status and tmp_return. */

      p_buf_idx = (void *) ret_buf;
      status = unmarshall_mstatus(&p_buf_idx, &find_mom_status);
      if (status == MAN_C_SUCCESS)
        {
        status = unmarshall_avl(&p_buf_idx, p_tmp_return);
        }
      }
    }

  if (stat == -1)
    {
    close(pe_mold_sh.s);
    pe_mold_connected = FALSE;
    }

  /*
   * Copy p_tmp_return to p_return_info.  Could not use p_return_info
   * directly because avl_from_xmit() assumes that the avl header is on
   * the stack.
   */
  if (status == MAN_C_SUCCESS)
    {
    status = moss_avl_copy_all(p_return_info, p_tmp_return, TRUE);
    }

  if (p_buf != NULL)
    {
    free(p_buf);
    }

  /*
   * Free the memory attached to the tmp_return.  Can't use moss_free_avl()
   * because part of the avl is on the stack.
   */
  avl_free_inst(p_tmp_return);


  if (status == MAN_C_SUCCESS)
    {
    status = find_mom_status;
    }

return(status); 
}



man_status mold_find_next_mom(
                   mold_handle,
                   p_current_class,
                   p_return_info
                  )
man_binding_handle  mold_handle;         
object_id          *p_current_class;      
avl                *p_return_info;     
/*
 * inputs:  mold_handle     is a MOLD binding handle.
 *          p_current_class is a pointer to the current class.
 *          p_return_info   is a pointer to a return AVL handle.
 *
 * outputs: *p_return_info  is the return AVL value.
 *
 * description:   This function marshalls the mold function's arguments,
 *                connects to the mold server and sends the name of the
 *                function to be invoked along with its arguments and
 *                receive a return status.
 */
{
#define NUM_FIND_NEXT_ARGS 3

  man_status      status             = MAN_C_SUCCESS,
                  find_mom_status    = MAN_C_SUCCESS;
  server_handle   mold_sh;
  size_t          mold_handle_size   = 0,
                  current_class_size = 0,
                  return_info_size   = 0,
                  size_t_size        = sizeof(size_t),
                  buf_size           = 0;
  obj_id_trans_t *p_current_xmit_obj = NULL;
  avl_trans_t    *p_return_xmit_obj  = NULL;
  avl             tmp_return,
                 *p_tmp_return       = &tmp_return;
  char           *p_buf              = NULL,
                 *p_buf_idx          = NULL,
                  return_buf[4],
                  msg[LINEBUFSIZE];
  memset((void *)&tmp_return, '\0', sizeof(tmp_return));

  /* Determine the number of bytes to send. */

  if (mold_handle != NULL) mold_handle_size = sizeof(*mold_handle);

  /* Flatten the object_id into a transmittable format and find size. */

  if (p_current_class != NULL)
    {
    object_id_to_xmit(p_current_class, &p_current_xmit_obj);
    if (p_current_xmit_obj != NULL)
      {
      current_class_size = p_current_xmit_obj->count * 
                           sizeof(p_current_xmit_obj->value) +
                           sizeof(p_current_xmit_obj->count);
      }
    }

  /* Flatten the AVL into a transmittable format and find its size. */

  if (p_return_info != NULL)
    {
    avl_to_xmit(p_return_info, &p_return_xmit_obj);
    if (p_return_xmit_obj != NULL)
      {
      return_info_size = p_return_xmit_obj->buf_len *
                         sizeof(p_return_xmit_obj->avl); 
      }
    }

  /* Determine the total size of the buffer to send. */

  buf_size = 1 + (NUM_FIND_NEXT_ARGS * size_t_size) + mold_handle_size +
             current_class_size + return_info_size;

  p_buf = (char *) malloc(buf_size);
  p_buf_idx = p_buf;

  /* Copy into the buffer the code for the function that will be invoked. */

  *p_buf_idx = MOLD_FIND_NEXT_MOM;
  p_buf_idx++;

  /* Copy into the buffer the flattened arguments. */

  status = marshall_man_binding_handle(&p_buf_idx, mold_handle,
                                       mold_handle_size);

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_object_id(&p_buf_idx, p_current_xmit_obj,
                                current_class_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = marshall_avl(&p_buf_idx, p_return_xmit_obj, return_info_size);
    }

  if (status == MAN_C_SUCCESS)
    {
    /* Connect to mold server. */
    memset((void *)mold_sh.name.sun_path, '\0', sizeof(mold_sh.name.sun_path));
    strcpy(mold_sh.name.sun_path, mold_handle->socket_address);
    status = connect_to_server(&mold_sh);
    }

  if (status == MAN_C_SUCCESS)
    {
    status = send_msg_to_server(mold_sh.s, buf_size, p_buf);
    }

  if (status == MAN_C_SUCCESS)
    {
    /* Read the return value. */
    if (read (mold_sh.s, return_buf, sizeof(return_buf)) == -1)
      {
      /* Write to SYSLOG. */
      sprintf(msg,
              MSG(sck_msg004, "S004 - Error during read: '%s'\n"),
              strerror(errno));
      socket_log(msg, LOG_ERR);
      status = MAN_C_PROCESSING_FAILURE;
      }
    else
      {
      /* Copy from the buffer status and return_info. */

      p_buf_idx = (void *) return_buf;
      status = unmarshall_mstatus(&p_buf_idx, &find_mom_status);
      if (status == MAN_C_SUCCESS)
        {
        status = unmarshall_avl(&p_buf_idx, p_tmp_return);
        }
      }
    }

  close(mold_sh.s);

  /*
   * Copy p_tmp_return to p_return_info.  Could not use p_return_info
   * directly because avl_from_xmit() assumes that the avl header is on
   * the stack.
   */
  if (status == MAN_C_SUCCESS)
    {
    status = moss_avl_copy_all(p_return_info, p_tmp_return, TRUE);
    }

  if (p_buf != NULL)
    {
    free(p_buf);
    }

  /*
   * Free the memory attached to the tmp_return.  Can't use moss_free_avl()
   * because part of the avl is on the stack.
   */
  avl_free_inst(p_tmp_return);

  if (status == MAN_C_SUCCESS)
    {
    status = find_mom_status;
    }

return(status);
}
