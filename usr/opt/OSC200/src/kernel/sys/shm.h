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
 *	@(#)$RCSfile: shm.h,v $ $Revision: 4.2.12.5 $ (DEC) $Date: 1993/09/09 19:32:56 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: (SYSIPC) IPC Shared Memory Facility
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 3
 *
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _SYS_SHM_H_
#define _SYS_SHM_H_

#include <standards.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <mach/machine/vm_types.h>
#ifdef _KERNEL
#include <vm/vm_object.h>
#include <vm/vm_anon.h>
#endif

#ifdef _XOPEN_SOURCE

/*
 *	Implementation Constants.
 */

#ifdef _KERNEL
#define SHMLBA  PAGE_SIZE
#else
#define SHMLBA  getpagesize()
#endif  /* _KERNEL */
/*
 *	Operation Flags.
 */

#define	SHM_RDONLY	010000	/* attach read-only (else read-write) */
#define	SHM_RND		020000	/* round attach address to SHMLBA */

/*
 *	Structure Definitions.
 */

/*
 *      There is a shared mem id data structure for each shared memory
 *      and mapped file segment in the system.
 */

struct shmid_ds {
	struct ipc_perm	shm_perm;	/* operation permission struct */
	int		shm_segsz;	/* size of segment in bytes */
	pid_t		shm_lpid;	/* pid of last shmop */
	pid_t		shm_cpid;	/* pid of creator */
	shmatt_t	shm_nattch;	/* current # attached */
	time_t		shm_atime;	/* last shmat time */
	time_t		shm_dtime;	/* last shmdt time */
	time_t		shm_ctime;	/* last change time */
};

#ifdef _KERNEL
struct shmid_internal {
	struct shmid_ds	s;		/* shared mem id data structure */
	struct vm_shm_object *shm_object;	/* Our object */
};
#endif /* _KERNEL */

#ifndef _KERNEL
#ifdef _NO_PROTO
extern void *	shmat();
extern int	shmdt();
extern int	shmctl();
extern int	shmget();
#else
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
extern void *	shmat(int, const void*, int);
extern int 	shmdt(const void*);
extern int 	shmctl(int, int, struct shmid_ds *);
extern int 	shmget(key_t, size_t, int);
#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */
#endif /* _KERNEL */

#endif /* _XOPEN_SOURCE */

#ifdef _OSF_SOURCE

/*
 *	ipc_perm Mode Definitions.
 */
#define	SHM_R		IPC_R	/* read permission */
#define	SHM_W		IPC_W	/* write permission */

struct	shminfo {
	int	shmmax,		/* max shared memory segment size */
		shmmin,		/* min shared memory segment size */
		shmmni,		/* number of shared memory identifiers */
		shmseg;		/* max attached shared memory segments per process */
};

#endif /* _OSF_SOURCE */


#ifdef _KERNEL
#if defined(__cplusplus)
extern struct shmid_internal *shmem;
#else
extern struct shmid_internal shmem[];
#endif
extern struct shminfo shminfo;
#endif

#endif /* _SYS_SHM_H_ */




