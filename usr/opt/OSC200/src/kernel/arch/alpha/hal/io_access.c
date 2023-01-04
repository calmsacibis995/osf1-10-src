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
static char *rcsid = "@(#)$RCSfile: io_access.c,v $ $Revision: 1.1.4.7 $ (DEC) $Date: 1993/11/19 14:11:50 $";
#endif

#include <io/common/devdriver.h>

/*
 * Forward prototype decl's for no_* routines 
 */
long no_read_io_port();
void no_write_io_port();
u_long no_iohandle_to_phys();
int no_io_copyin();
int no_io_copyout();
int no_io_zero();
int no_io_copyio();
vm_offset_t no_get_io_handle();
int no_io_bcopy();

/*
 * The ioaccess_callsw structure contains pointers to the platform-dependent
 * functions that support accesses by drivers to io devices. The structure
 * is initialized by the platform-specific init function during bootstrap
 * (e.g., kn121_init() on JENSEN).
 */

struct  ioaccess_callsw      {
   long    (*hal_read_io_port)();       /* ptr to HAL's read_io_port	 */
   void    (*hal_write_io_port)();      /* ptr to HAL's write_io_port	 */
   u_long  (*hal_iohandle_to_phys)();	/* ptr to HAL's iohandle_to_phys */
   int     (*hal_io_copyin)();  	/* ptr to HAL's io_copyin	 */
   int     (*hal_io_copyout)();    	/* ptr to HAL's io_copyout	 */
   int     (*hal_io_copyio)();    	/* ptr to HAL's io_copyio	 */
   int     (*hal_io_zero)();		/* ptr to HAL's io_zero		 */
   vm_offset_t (*hal_get_io_handle)();  /* ptr to HAL's get_io_handle    */
   int     (*hal_io_bcopy)();     	/* ptr to HAL's io_bcopy	 */
};

/*
 * define and initialize the call switch table
 */
struct  ioaccess_callsw ioaccess_callsw = {
					no_read_io_port, no_write_io_port,
					no_iohandle_to_phys, no_io_copyin,
					no_io_copyout, no_io_copyio,
				        no_io_zero,
					no_get_io_handle, no_io_bcopy
			};

/*
 *  This routine will fill in the ioaccess_callsw table,
 *  which is platform-specific.
 */
ioaccess_callsw_init(rd_ioport_fcn, wrt_ioport_fcn,
		     iohandle_to_phys_fcn, io_copyin_fcn,
		     io_copyout_fcn, io_copyio_fcn,
		     io_zero_fcn,
		     get_io_handle_fcn, io_bcopy_fcn) 
   long    (*rd_ioport_fcn)();         /* ptr to HAL's read_io_port	*/
   void    (*wrt_ioport_fcn)();        /* ptr to HAL's write_io_port	*/
   u_long  (*iohandle_to_phys_fcn)();  /* ptr to HAL's iohandle_to_phys */
   int     (*io_copyin_fcn)();  	  /* ptr to HAL's io_copyin	*/
   int     (*io_copyout_fcn)();  	  /* ptr to HAL's io_copyout	*/
   int     (*io_copyio_fcn)();  	  /* ptr to HAL's io_copyio	*/
   int     (*io_zero_fcn)();		  /* ptr to HAL's io_zero       */
   vm_offset_t (*get_io_handle_fcn)(); 	  /* ptr to HAL's get_io_handle */
   int     (*io_bcopy_fcn)();   	  /* ptr to HAL's io_bcopy	*/
{
        ioaccess_callsw.hal_read_io_port = rd_ioport_fcn;
        ioaccess_callsw.hal_write_io_port = wrt_ioport_fcn;
        ioaccess_callsw.hal_iohandle_to_phys = iohandle_to_phys_fcn;
        ioaccess_callsw.hal_io_copyin = io_copyin_fcn;
        ioaccess_callsw.hal_io_copyout = io_copyout_fcn;
        ioaccess_callsw.hal_io_copyio = io_copyio_fcn;
        ioaccess_callsw.hal_io_zero = io_zero_fcn;
        ioaccess_callsw.hal_get_io_handle = get_io_handle_fcn;
        ioaccess_callsw.hal_io_bcopy = io_bcopy_fcn;
}

long
read_io_port(dev_addr, width, type)
   io_handle_t	dev_addr;
   int		width;
   int		type;
{
	return((*ioaccess_callsw.hal_read_io_port)(dev_addr, width, type));
}

void
write_io_port(dev_addr, width, type, data)
   io_handle_t	dev_addr;
   int		width;
   int		type;
   long		data;
{
	(*ioaccess_callsw.hal_write_io_port)(dev_addr, width, type, data);
}

u_long
iohandle_to_phys(io_handle, flags)
   io_handle_t	io_handle;
   long		flags;
{
	return((*ioaccess_callsw.hal_iohandle_to_phys)(io_handle, flags));
}

int
io_copyin(io_handle, addr, len)
   io_handle_t  io_handle;
   vm_offset_t	addr;
   u_long	len;
{
	return((*ioaccess_callsw.hal_io_copyin)(io_handle, addr, len));
}

int
io_copyout(addr, io_handle, len)
   vm_offset_t	addr;
   io_handle_t  io_handle;
   u_long	len;
{
	return((*ioaccess_callsw.hal_io_copyout)(addr, io_handle, len));
}

int
io_copyio(io_handle1, io_handle2, len)
   io_handle_t  io_handle1;
   io_handle_t  io_handle2;
   u_long	len;
{
	return((*ioaccess_callsw.hal_io_copyio)(io_handle1, io_handle2, len));
}
int
io_zero(io_handle1, len)
   io_handle_t  io_handle1;
   u_long	len;
{
	return((*ioaccess_callsw.hal_io_zero)(io_handle1, len));
}

int
io_bcopy(srcaddr, dstaddr, byte_count)
   caddr_t srcaddr;
   caddr_t dstaddr;
   int     byte_count;
{
	return((*ioaccess_callsw.hal_io_bcopy)(srcaddr, dstaddr, byte_count));
}

vm_offset_t
get_io_handle(dev_addr, width, type)
   io_handle_t  dev_addr;
   int          width;
   int          type;
{
	return((*ioaccess_callsw.hal_get_io_handle)(dev_addr, width, type));
}

/*
 * the no/null functions loaded/init'd into io access switch table
 */
long
no_read_io_port()
{
	panic("read_io_port unimplemented on this platform");
}

void
no_write_io_port()
{
	panic("write_io_port unimplemented on this platform");
}

u_long
no_iohandle_to_phys(io_handle, flags)
   io_handle_t	io_handle;
   long		flags;
{
	panic("iohandle_to_phys unimplemented on this platform");
}

int
no_io_copyin()
{
	panic("io_copyin unimplemented on this platform");
}

int
no_io_copyout()
{
	panic("io_copyout unimplemented on this platform");
}

int
no_io_copyio()
{
	panic("io_copyio unimplemented on this platform");
}
int
no_io_zero()
{
	panic("io_zero unimplemented on this platform");
}

vm_offset_t
no_get_io_handle()
{
	panic("get_io_handle unimplemented on this platform");
}

int
no_io_bcopy()
{
	panic("io_bcopy unimplemented on this platform");
}

