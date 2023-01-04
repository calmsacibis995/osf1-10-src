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
 * @(#)$RCSfile: man_data.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:48:22 $
 */
#ifndef _MAN_DATA_H 
#define _MAN_DATA_H  

#if defined(__osf__)
/* KLUDGE. 
 * On OSF <sys/un.h> defines "u_short sun_family" however
 * on Ultrix it's defined as  "short sun_family". We 
 * include <sys/types.h> here, which defines the "u_" typedefs.
 * <sys/types.h> only defines the "u_" typedefs if _OSF_SOURCE  
 * is defined.  Warning, if <sys/types.h> is included before
 * man_data.h they will not be defined.
 */
# if !defined(_OSF_SOURCE)
#  define _OSF_SOURCE
#  include <sys/types.h>
#  undef _OSF_SOURCE
# else
#  include <sys/types.h>
# endif 
#endif

#if defined(__osf__)
# if !defined(_POSIX_4SOURCE)
#  define _POSIX_4SOURCE
#  include <signal.h>
#  include <sys/time.h>
#  undef _POSIX_4SOURCE
# endif
#endif

#include <sys/un.h>

extern int socket();
extern int bind();
extern int listen();
extern int accept();
extern int connect();
extern int close();

#define globaldef
#define globalref extern

#define rpc_c_protseq_max_calls_default (0)

#define MOLD_REGISTER_MOM        1
#define MOLD_DEREGISTER_MOM      2
#define MOLD_FIND_MOM            3
#define MOLD_FIND_NEXT_MOM       4
#define MOI_GET_ATTRIBUTES       5
#define MOI_SET_ATTRIBUTES       6
#define MOI_CREATE_INSTANCE      7
#define MOI_DELETE_INSTANCE      8
#define MOI_INVOKE_ACTION        9
#define PEI_SEND_GET_REPLY      10
#define PEI_SEND_SET_REPLY      11
#define PEI_SEND_CREATE_REPLY   12
#define PEI_SEND_DELETE_REPLY   13
#define PEI_SEND_ACTION_REPLY   14
#define EVD_CREATE_QUEUE_HANDLE 15
#define EVD_DELETE_QUEUE_HANDLE 16
#define EVD_POST_EVENT          17
#define STOP_SERVER             99

#define SERVER_STARTED           1
#define SERVER_STOPPED           0

typedef struct
  {
    struct sockaddr_un  name;         /* UNIX domain socket address */
    int                 s,            /* socket descriptor */
                        name_length;
  } server_handle;

typedef struct _handle_t
{
  char socket_address[128];
} *handle_t;

typedef handle_t man_binding_handle;

typedef struct _management_handle
{ 
  unsigned int length;
  char socket_address[128];
} management_handle;

typedef struct _queue_handle
{
  char socket_address[128];
} queue_handle;

typedef struct object_id 
{
  int count;
  unsigned int *value;
} object_id;

typedef struct
{
#if defined(__osf__)
  char opaque_data[40];
#else
  char opaque_data[24];
#endif
} avl;

typedef int scope;

typedef int reply_type;

typedef struct
{ 
  char utc[16];
} mo_time;

typedef struct
{ 
  char uid[16];
} uid;

typedef int process_id;

typedef struct obj_id_trans_t
{
  int count;
  unsigned int value[1];
} obj_id_trans_t;

typedef struct avl_trans_t 
{
  int buf_len;
  char avl[1];
} avl_trans_t;

typedef struct free_avl_trans_t
{
  int buf_len;
  char avl[1];
} free_avl_trans_t;

typedef struct
{
#if defined(__osf__)
  char opaque_data[40];
#else
  char opaque_data[24];
#endif 
} free_avl;

typedef struct man_handle_trans_t
{
  int buf_len;
  char management_handle[1];
} man_handle_trans_t;

typedef struct {
    unsigned char process[12];
  } uuid_t;  
typedef uuid_t *uuid_p_t;

typedef struct  {
    unsigned int count;
    uuid_p_t uuid[1];
  } uuid_vector_t;
typedef uuid_vector_t *uuid_vector_p_t;

typedef void (*rpc_mgr_proc_t)(void);
typedef rpc_mgr_proc_t *rpc_mgr_epv_t;

typedef handle_t rpc_binding_handle_t;

typedef struct  {
    unsigned int count;
    rpc_binding_handle_t binding_h[1];
  } rpc_binding_vector_t;

typedef rpc_binding_vector_t *rpc_binding_vector_p_t;

#endif

