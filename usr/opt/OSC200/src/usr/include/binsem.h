

#ifndef _BINSEM_H
#define _BINSEM_H 1

#include  <sys/limits.h>
#include <sys/psx4_nspace.h>
  
typedef  int            binsemset_t;
#define  SEM_LOCKED     1
#define  SEM_UNLOCKED   0
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
 * @(#)$RCSfile: binsem.h,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/12/15 22:13:03 $
 */




/*
 * One binary seamphore data structure per binary semaphore.
 */

typedef struct p4sem{
  int	        semstate;	                             /* semaphore state                  */
  int   	semncnt;	                             /* # awaiting semaphore             */
}p4sem_t;

/*
 * One binary semaphore set data structure (p4semid_set) for each set of semaphores 
 * in the system.
 */

typedef struct p4semid_set{ 
     struct obj_header       p4obj_hd;                  
     char                    semname[SEM_NAME_MAX];         /* semaphore set name                  */
     p4sem_t                 sem[SEM_NSEMS_MAX];            /* binary semaphore array              */
}p4semid_set_t;



#ifndef _KERNEL

/*
 * POSIX 1003.4 Binary Semaphores functions
 */
#ifdef _NO_PROTO

int sem_mksem();
int sem_destroy();
int sem_getnsem();
int sem_open();
int sem_close();
int sem_wait();
int sem_ifwait();
int sem_post();
#else                         

/* 
 * routine definitions
 */

_BEGIN_CPLUSPLUS
int sem_mksem(const char *name, size_t nsems, int flags);  
int sem_destroy(const char *name);
int sem_open(const char *name, binsemset_t *b);  
int sem_close(binsemset_t binsemset); 
int sem_getnsem(binsemset_t binsemset); 
int sem_wait(binsemset_t binsemdes, int binsemnum);
int sem_ifwait(binsemset_t binsemdes, int binsemnum);
int sem_post(binsemset_t binsemdes, int binsemnum);
_END_CPLUSPLUS
#endif  /* _NO_PROTO_ */
#endif  /* _KERNEL */
#endif	/* _BINSEM_H */



