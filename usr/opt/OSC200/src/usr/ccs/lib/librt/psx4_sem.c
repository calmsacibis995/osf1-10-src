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
static char	*sccsid = "@(#)psx4_sem.c	3.2	(ULTRIX/OSF)	12/6/91";
#endif
/*
 * @DIGITALCOPYRIGHT@
 */

/*****************************************************************************
 *++
 *
 * Facility:
 *
 *	ULTRIX POSIX 1003.4/D11 Binary Semaphores run-time library routines
 *
 * Abstract:
 *
 *	This module contains the routines to manipulate
 *	the binary semaphores
 *
 *      Binary Semaphores Routines:
 *		sem_mksem()	sem_open()	sem_close()    sem_destroy()
 *		sem_getnsem()   sem_wait()	sem_ifwait()	sem_post()     
 *              
 *
 *		
 * Author:
 *      Lai-Wah Hui
 *	
 *
 * Modified by:
 *
 *   modifier's name,	dd-mmm-yyyy,  VERSION: svv.u-ep
 *   01 - modification description
 *
 * Notes --
 *--
 */
#include <binsem.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

/*
** Constants and Macros
*/
#ifdef  _THREAD_SAFE
#define SETERR(err)        seterrno(err)
#else
#define	SETERR(err)	   errno = err
#endif


#define  p4semdes_entry    p4semdes_table->des_entry


#define err_ret(e)          SETERR(e);                  \
                            return(FAILURE)

#define  check_sem_range(b, obj)                        \
  if((b < 0) || (b >= obj->p4obj_hd.cnt)){              \
     err_ret(EINVAL);                                   \
   }                                                     
   


#define semfile_size        sizeof(p4semid_set_t) 
#define sem_locked(sem)     (sem->semncnt != 0) || (_msem_tas(&sem->semstate) != 0)

/*
** External user mode supported functions
*/

extern p4_key_table_t      *p4_create_key_table();
extern void                *p4_find_object_by_key();
extern p4_key_entry_t      *p4_find_entry_by_key();
extern int                 p4_bind_key_to_name_entry();
extern int                 p4_get_key_by_name();

/*
** External syscalls
*/
extern int                 psx4_sem_sleep();
extern int                 psx4_sem_wakeup();


static  p4_key_table_t     *p4semdes_table = 0;


/*++
 *
 * sem_mksem()
 *
 * Functional description:
 *
 *	This function creates a new binary semaphore set.
 *
 * Inputs:
 *
 *	name  - The name of the binary semaphore set.
 *      nsems - Number of binary semaphores in the set.
 *      flag  - The initial state of the binary semaphore: SEM_LOCKED or SEM_UNLOCKED 
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *     None
 * Implicit outputs/side effects:
 *
 *     A new set of binary semaphores is created.
 *
 *--
 */


sem_mksem(const char *name, size_t nsems, int flags){
 
int            fd;
int            sav_err;
p4semid_set_t  *sempa;
int            state, i;
mode_t         mode = S_IRWXO|S_IRWXG|S_IRWXU;

  if (!name) {
     err_ret(EINVAL);
  }

  if((nsems <= 0) || (nsems > SEM_NSEMS_MAX)){
    err_ret(EAGAIN);
  }

  if((fd = open(name, O_RDWR|O_CREAT|O_TRUNC|O_EXCL, mode)) == FAILURE)
     return(FAILURE);

  

  /* lock the file to ensure an open error by other atempts to access the Binary 
     Semaphore set during mksem. */

  if(flock(fd,LOCK_EX) == FAILURE)
    {
     sav_err = errno;
     goto errout1;
    }

  if(ftruncate(fd, 2*semfile_size) == FAILURE)
     goto errout;



  if((sempa = (p4semid_set_t *)mmap(0, semfile_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, NULL)) == (void *)FAILURE)
      goto errout;

  strncpy(sempa->semname, name, strlen(name));
  sempa->p4obj_hd.cnt = nsems;
  sempa->p4obj_hd.state = P4_STATE_INUSE;
  

  state = (flags & SEM_LOCKED) ? SEM_LOCKED : SEM_UNLOCKED;

  /* init. the initial state of each binary semaphore in the set */
     
  for(i = 0; i < nsems; i++){
      sempa->sem[i].semstate = state;
      sempa->sem[i].semncnt  = 0;
    }

  msync(sempa, semfile_size, MS_SYNC);
  munmap(sempa, semfile_size);
  flock(fd, LOCK_UN); 
  close(fd);
  return(SUCCESS); 

     
errout:     sav_err = errno;
            flock(fd, LOCK_UN);
errout1:    close(fd);
            unlink(name);
            err_ret(sav_err);

}


 
/*++
 *
 * sem_open()
 *
 * Functional description:
 *
 *      The purpose of this function is to establish the connection between a set of
 *      binary semaphores created by the sem_mksem() function and a binary semaphore
 *      set descriptor.   This function allocats a binary semaphore set descriptor,
 *      which is used in subsequent calls to sem_getnsems(), sem_(if)wait(), sem_post(),
 *      and sem_close().
 *
 * Inputs:
 *
 *	name  - The name of the binary semaphore set. 
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *      *b    - A pointer to the Binary Semaphore set descriptor.
 *
 * Implicit outputs/side effects:
 *
 *     None
 *
 *--
 */



int sem_open(const char *name,  binsemset_t *b){

int           fd, i;
struct stat   statb;
p4semid_set_t *pa;
int           err;


  if ((!name) || (!b)){
    err_ret(EFAULT);
  }

  if((fd = open (name, O_RDWR)) == FAILURE)
      return(FAILURE);


  if(flock(fd, LOCK_SH) == FAILURE){
    close(fd);
    err_ret(ENOENT);
   }

  flock(fd, LOCK_UN);

  if((fstat(fd, &statb) == FAILURE) || (statb.st_size == 0)){
    close(fd);
    if(statb.st_size == 0)
       err_ret(ENOENT);
    return(FAILURE);
  }
  
   
                

  /*
  ** If this is the first time the process calls the sem_open(),  then the binary semaphore descriptor
  ** table will be created. 
  */

 
 if(p4semdes_table == 0){
     if((p4semdes_table = p4_create_key_table(SEM_NSETS_MAX, sizeof(p4_key_entry_t))) == P4_KEY_TABLE_NULL){
           close(fd);
           err_ret(ENOSPC);
       }
     }
 
 /* 
 ** If this is not the first time the process opens the binary semaphore set named by
 ** the argument name.  returns the binary semaphore descriptor as allocated at the first open 
 ** for the binary semaphore set 
 */
 

  if((p4_get_key_by_name(p4semdes_table, name, b)) != FAILURE){
      close(fd);
      return(SUCCESS);
    }
 

  if((pa = (p4semid_set_t *)mmap(0, semfile_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, NULL))
       == (void *)FAILURE){
      err = errno;
      close(fd);
      err_ret(err);
    }
    
  close(fd);


  /* check if this is a semaphore set */
  
  
  if(( strncmp(name, pa->semname, strlen(name))) != 0){
     munmap(pa, semfile_size);
     err_ret(ENOENT);
   }

  /* allocates a new binary semaphore descriptor */

  if((p4_bind_key_to_name_entry(p4semdes_table,pa,name, b)) == FAILURE){
      munmap(pa, semfile_size);
      err_ret(ENOSPC);
   }

  return(SUCCESS);
}
  

/*++
 *
 * sem_close()
 *
 * Functional description:
 *
 *	This function frees the binary semaphore set descriptor indicated by binsemset. 
 *
 * Inputs:
 *
 *	binsemset  - The binary semaphore set descriptor
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *     None
 *
 * Implicit outputs/side effects:
 *
 *    None
 *
 *--
 */



sem_close(binsemset_t binsemset){

register p4_key_entry_t *ke;
register p4semid_set_t *obj;

  
  if((ke = (void *)p4_find_entry_by_key(p4semdes_table, binsemset)) == (void *)EINVAL){
     err_ret(EINVAL);
   }

  munmap(ke->object, semfile_size);
  p4_return_key_entry(p4semdes_table,ke);
  return(SUCCESS);

}

 
/*++
 *
 * sem_destroy()
 *
 * Functional description:
 *
 *	This routine deallocate the binary semaphore set named by name.  
 *
 * Inputs:
 *
 *	name  - The name of the binary semaphore set.
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *     None
 *
 * Implicit outputs/side effects:
 *
 *    The file is removed.
 *
 *--
 */



sem_destroy(const char *name){

int           fd;
p4semid_set_t *pa;
struct stat   statb;
int           err;

  if (!name){
     err_ret(EFAULT);
  }

  if((fd = open (name, O_RDWR)) == FAILURE)
    return(FAILURE);


  if(flock(fd, LOCK_SH) == FAILURE){
    close(fd);
    err_ret(ENOENT);
  }

  flock(fd, LOCK_UN);

  /* Get information about the file, if not possible return.
   * If the file size is 0 then it is obviously not a semaphore file, so return.
   */

  if((fstat(fd, &statb) == FAILURE)){
    close(fd);
    return(FAILURE);
  } else if (statb.st_size == 0){
    close(fd);
    err_ret(ENOENT);
  }

  if((pa = (p4semid_set_t *)mmap(0, semfile_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, NULL))
       == (void *)FAILURE){
      err = errno;
      close(fd);
      err_ret(err);
    }

  close(fd);

  if((strncmp(name, pa->semname, strlen(name))) != 0){
     munmap(pa, semfile_size);
     err_ret(ENOENT);
   }

  pa->p4obj_hd.state = P4_KEY_STATE_DELETE;
  munmap(pa, semfile_size);
  unlink(name);
  return(SUCCESS);
}

 
/*++
 *
 * sem_getnsems()
 *
 * Functional description:
 *
 *	This function returns the number of binary semaphores in the binary semaphore set.
 *      
 *
 * Inputs:
 *
 *	binsemset  - The binary semaphore set descriptor.
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *     the number of binary semaphores in the set.
 *
 * Implicit outputs/side effects:
 *
 *    None
 *
 *--
 */



sem_getnsems(binsemset_t binsemset){

register p4semid_set_t *obj;

   
  if((obj = (void *)p4_find_object_by_key(p4semdes_table, binsemset)) == (void *)EINVAL){
     err_ret(EINVAL);
   }

  return(obj->p4obj_hd.cnt);
}


 
/*++
 *
 * sem_wait()
 *
 * Functional description:
 *
 *	This function performs the P operation on the binary semaphore referenced by the combination of binsemset 
 *      and binsemnum.   If the binary semaphore is already locked, the calling process will be blocked and added to 
 *      the list of awaiting the semaphore.
 *      
 *
 * Inputs:
 *
 *	binsemset  - The binary semaphore set descriptor.
 *      binsemnum  - The number of a binary semaphore in the set referenced by binsemset.
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *     None
 *
 * Implicit outputs/side effects:
 *
 *     None
 *
 *--
 */



sem_wait(binsemset_t binsemset, int binsemnum){

  
register p4semid_set_t  *obj;
register p4sem_t        *sem;
register p4_key_entry_t *ke;
int sem_ifwait();
  
  if((obj = (void *)p4_find_object_by_key(p4semdes_table, binsemset)) == (void *)EINVAL){
     err_ret(EINVAL);    
   }

   check_sem_range(binsemnum, obj);

   sem = &obj->sem[binsemnum];
  
   if(sem_locked(sem)){
      if((ke = p4_find_entry_by_key(p4semdes_table, binsemset)) == (void *)EINVAL){
          err_ret(EINVAL);
      }

   if(psx4_sem_sleep(sem,ke,obj) != 0){
        return(FAILURE);
      }
 



  }

  return(SUCCESS);

}


 
/*++
 *
 * sem_ifwait()
 *
 * Functional description:
 *
 *	This function performs the P operation on the binary semaphore referenced by the combination of binsemset 
 *      and binsemnum only if it can do so without waiting.
 *      
 *
 * Inputs:
 *
 *	binsemset  - The binary semaphore set descriptor.
 *      binsemnum  - The number of a binary semaphore in the set referenced by binsemset.
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *     None
 *
 * Implicit outputs/side effects:
 *
 *    None
 *
 *--
 */



sem_ifwait(binsemset_t binsemset, int binsemnum){

  
register p4semid_set_t *obj;
register p4sem_t       *sem;

                                   
  
  if((obj = (void *)p4_find_object_by_key(p4semdes_table, binsemset)) == (void *)EINVAL){
     err_ret(EINVAL);    
   }


  check_sem_range(binsemnum, obj);

   
  sem = &obj->sem[binsemnum];

  if(sem_locked(sem)){
        err_ret(EAGAIN);    
  }

  return(SUCCESS);

}

 
/*++
 *
 * sem_post()
 *
 * Functional description:
 *
 *	This function performs the V operation on  the binary semaphore referenced by the combination of 
 *      binsemset  and binsemnum.  If one or more processes are blocked waiting for the binary semaphore to 
 *      become unlocked,  the highest priority waiting process will be unblocked. 
 *      
 *
 * Inputs:
 *
 *	binsemset  - The binary semaphore set descriptor.
 *      binsemnum  - The number of a binary semaphore in the set referenced by binsemset.
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *     None
 *
 * Implicit outputs/side effects:
 *
 *    None
 *
 *--
 */


sem_post(binsemset_t binsemset, int binsemnum){
register p4semid_set_t  *obj;
register p4sem_t        *sem;
register p4_key_entry_t *ke;



                                     
  
  if((obj =(void *) p4_find_object_by_key(p4semdes_table, binsemset)) == (void *)EINVAL){
    err_ret(EINVAL);      
   }

  check_sem_range(binsemnum, obj);

  sem = &obj->sem[binsemnum];

  if(sem->semstate == SEM_UNLOCKED){
    err_ret(EAGAIN);    
  }
  sem->semstate = SEM_UNLOCKED;      
  if(sem->semncnt){
     
    return(psx4_sem_wakeup(sem));
  }
  return(SUCCESS);

}














