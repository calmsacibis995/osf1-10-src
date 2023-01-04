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
static char *rcsid = "@(#)$RCSfile: memlog.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/10/29 20:33:14 $";
#endif
/*
 * Name:
 *	memlog.c  (/dev/mlog)
 *
 * Description:
 * 	This is a pseudo-driver to log all calls to kernel wired memory
 *	allocation and free request. It supports driver interface (read,
 *	open, ioctl etc.) to read the log entries and set options for
 * 	the logging. 
 */

#include <sys/param.h>
#include <sys/uio.h>
#include <sys/ucred.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/memlog.h>
#include <sys/ioctl.h>
#include <sys/secdefines.h>
#include <sys/security.h>
#include <sys/user.h>


#define MEM_BUFF_SIZE 	1024*64

char mem_buff[MEM_BUFF_SIZE];
long mem_count = 0;
char * mem_rptr = mem_buff;
char * mem_wptr = mem_buff;
char * mem_lptr = &mem_buff[MEM_BUFF_SIZE];
int mem_init = 0;
int mem_wait = 0;
int mem_open = 0;
simple_lock_data_t      mem_lock;

/*
 * OPEN
 */
int
mlogopen(dev_t dev)
{
	int error;

#if SEC_BASE
        if (!privileged(SEC_DEBUG, EPERM))      /* XXX which priv needed? */
                return(EPERM);
#else
        if (error = suser(u.u_cred, &u.u_acflag))
                return(error);
#endif
	if(mem_open == 0) {
		mem_open = 1;
		return(ESUCCESS);
	}
	else
		return(EBUSY);
}

/*
 * CLOSE
 */
int
mlogclose()
{
	int error;

	if(mem_open) 
		mem_open = 0;

	memlog = 0;

	return(ESUCCESS);
}




void
memory_log(int type, void *caller, int size)
{

	if(mem_init == 0) {
		simple_lock_init(&mem_lock);
		mem_init = 1;
	}

	mem_count++;

	if( size < memlog )
		return;

	if(*mem_wptr != 0) 
		return;

	simple_lock(&mem_lock);
	*mem_wptr++ = 1;
	sprintf(mem_wptr,"%18d %2d %16lx %10d\n", mem_count, type, caller, size);
	mem_wptr += MEM_RECORD_SIZE;
	if ((mem_wptr + MEM_RECORD_SIZE +1) > mem_lptr)
		mem_wptr = mem_buff;

	if(mem_wait) {
		mem_wait = 0;
		thread_wakeup((vm_offset_t)&mem_wait);
	}
	simple_unlock(&mem_lock);

	return;
}


int
mlogread(dev_t dev, struct uio *uio)
{
	caddr_t		base;
	int 		len;
	register	struct iovec *iov;
	register	struct uio *uiop;
	int 		error = 0;

	uiop = uio;

	iov = uiop->uio_iov;

	if(uiop->uio_rw != UIO_READ)
		return(EIO);

	if(iov->iov_len < MEM_RECORD_SIZE)
		return(EIO);

	base = iov->iov_base;

	simple_lock(&mem_lock);

	if(*mem_rptr != 1) {
		assert_wait((vm_offset_t)&mem_wait, TRUE);
                mem_wait = 1;
                simple_unlock(&mem_lock);
                thread_block();
	}
	
	*mem_rptr++ = 0;
	bcopy(mem_rptr, base, MEM_RECORD_SIZE);
	mem_rptr += MEM_RECORD_SIZE;

	if ((mem_rptr + MEM_RECORD_SIZE +1) > mem_lptr)
		mem_rptr = mem_buff;

        simple_unlock(&mem_lock);

	iov->iov_base += MEM_RECORD_SIZE;
	iov->iov_len -= MEM_RECORD_SIZE;
        uiop->uio_offset += MEM_RECORD_SIZE;
        uiop->uio_resid -= MEM_RECORD_SIZE;

	return(error);
}


/*
 * Function: mlogioctl(dev, com, data, flag)
 *
 * Function description: Binary event logger control and status interface
 *
 * Arguments: 
 *	dev	device (not used)
 *	com 	command type specified by user
 *	data	argument passed by user, or loc to return value to user
 *	f_flag	not used
 *
 * Return value: SUCCESS or ERROR
 *
 * Side effects: None
 */

mlogioctl(dev, com, data, flag)
dev_t dev;
unsigned int com;
long *data; 
long flag;
{

	switch (com) {
	case MEM_LOG_ENABLE:
		mlogflush();
		memlog = *data;
		break;
	case MEM_LOG_DISABLE:
		memlog = 0;
		break;
	default:
		return(EINVAL);
	}
	return(0);
}


mlogflush()
{
	long save;

	simple_lock(&mem_lock);
	save = memlog;
	memlog = 0;	/* no one should be able to log while we are flushing */
	while(*mem_rptr == 1) {
		*mem_rptr++ = 0;
		mem_rptr += MEM_RECORD_SIZE;

		if ((mem_rptr + MEM_RECORD_SIZE +1) > mem_lptr)
			mem_rptr = mem_buff;
	}

	memlog = save;
        simple_unlock(&mem_lock);

	return(0);
}

