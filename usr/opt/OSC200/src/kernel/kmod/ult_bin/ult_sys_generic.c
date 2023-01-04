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
static char *rcsid = "@(#)$RCSfile: ult_sys_generic.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/07/09 16:16:17 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)ult_sys_generic.c	3.3	(ULTRIX/OSF)	8/8/91";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * ult_sys_generic.c
 *
 * Modification History:
 *
 *  7-Oct-91    Philip Cameron
 *      Created file.
 *
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <sys/termio.h>
#include <sys/termios.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include "ult_kmod.h"



/* Ultrix 4.2 defines for the following binary compatability ioctls
 *	All Ultrix 4.2 ioctl commands that require special processing
 *	are placed here at a common location. These commands are
 *	"converted" to an equivalent OSF/1 command which is 
 *	executed. The result is converted back to Ultrix.
 *
 *	Special note must be taken of the the size of the data structs
 *	that are processed. Frequently the data passed by the commands
 *	is different for OSF/1 and Ultrix.
 */
#define U_TCGETP	0x40247455	/* _IOR('t',85,struct termios) */
#define U_TCSANOW	0x80247454	/* _IOW('t',84,struct termios) */
#define U_TCSADRAIN	0x80247453	/* _IOW('t',83,struct termios) */
#define U_TCSAFLUSH	0x80247452	/* _IOW('t',82,struct termios) */
#define	U_TCSETAF	0x80147458	/* _IOW('t',88,struct termio)  */
#define	U_TCSETAW	0x80147459	/* _IOW('t',89,struct termio)  */
#define	U_TCSETA	0x8014745a	/* _IOW('t',80,struct termio)  */
#define	U_TCGETA	0x4014745b	/* _IOR('t',91,struct termio)  */
#define	U_TCFLSH	0x2000745c	/* _IO('t',92)	- termio	*/
#define	U_TCXONC	0x2000745d	/* _IO('t',93)	- termio	*/
#define	U_TCSBRK	0x2000745e	/* _IO('t',94)	- termio	*/
#define	U_TIOCCAR	0x2000745f	/* _IO('t',95)			*/
#define	U_TIOCNCAR	0x20007460	/* _IO('t',96)			*/
#define	U_TIOCWONLINE	0x20007461	/* _IO('t',97)			*/
#define	U_TIOCMODEM	0x80047462	/* _IO('t',98)			*/
#define	U_TIOCNMODEM	0x80047463	/* _IO('t',99)			*/
#define U_TIOCGETD      0x40047400      /* _IOR('t',0,int)		*/
#define U_TIOCSETD      0x80047401      /* _IOR('t',1,int)		*/

/* routines to convert back and forth between OSF/1 and Ultrix
 * termios structs */
extern	int	cvtsOtoU();
extern	int	cvtsUtoO();
extern	int	cvtOtoU();
extern	int	cvtUtoO();


#define U_NCCS  19
struct u_termios {
        tcflag_t        c_iflag;                /* Input Modes          */
        tcflag_t        c_oflag;                /* Output Modes         */
        tcflag_t        c_cflag;                /* Control Modes        */
        tcflag_t        c_lflag;                /* Local Modes          */
        cc_t            c_cc[U_NCCS];           /* Control Characters   */
        cc_t            c_line;                 /* line disc. -local ext*/
};




/*
 * Ioctl system call
 *	Replaces the OSF call for Ultrix compatability module 
 */
ult_ioctl(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	fdes;
		u_long	cmd;
		caddr_t	cmarg;
	} *uap = (struct args *) args;
	struct file *fp;
	register int error, flag;
	register u_int com;
	register u_int size, real_size;
	caddr_t memp = 0;
#define STK_PARMS	128
	char stkbuf[STK_PARMS];
	caddr_t data = stkbuf;
	struct ufile_state *ufp = &u.u_file_state;

	struct	termios	osf;
	struct  u_termios *ult = (struct u_termios *)data;
	char	ult_line;
	int	line;
	u_int	ult_cmd;


	if(ULT_TRACE) {
	    printf("(%s %d) ioctl {54} fdes %d, cmd 0x%x, cmarg 0x%x\n",
		u.u_comm, u.u_procp -> p_pid, uap->fdes, uap->cmd, uap->cmarg);
	}

	error = 0;
	if (error = getf(&fp, uap->fdes, FILE_FLAGS_NULL, ufp))
		return (error);
	BM(FP_LOCK(fp));
	flag = fp->f_flag;
	BM(FP_UNLOCK(fp));
	if ((flag & (FREAD|FWRITE)) == 0) {
		error = EBADF;
		goto out;
	}
	com = (u_int) uap->cmd;
	if (com == FIOCLEX) {
		U_FDTABLE_LOCK(ufp);
		U_POFILE_SET(uap->fdes, U_POFILE(uap->fdes, ufp) | UF_EXCLOSE,
			ufp);
		U_FDTABLE_UNLOCK(ufp);
		goto out;
	}
	if (com == FIONCLEX) {
		U_FDTABLE_LOCK(ufp);
		U_POFILE_SET(uap->fdes, U_POFILE(uap->fdes, ufp) & ~UF_EXCLOSE,
			ufp);
		U_FDTABLE_UNLOCK(ufp);
		goto out;
	}

	/*
	 * Interpret high order word to find
	 * amount of data to be copied to/from the
	 * user's address space.
	 */
	real_size = size = IOCPARM_LEN(com);
	if (size > IOCPARM_MAX) {
		error = ENOTTY;
		goto out;
	}
	if (size > sizeof (stkbuf)) {
		if ((memp = (caddr_t)kalloc(size)) == 0) {
			error = ENOMEM;
			goto out;
		}
		data = memp;
	}

	if (com&IOC_IN) {
		if (size) {
			if (error = 
			    copyin(uap->cmarg, (caddr_t)data, (u_int)real_size))
				goto out;
		} else
			*(caddr_t *)data = uap->cmarg;
	} else if ((com&IOC_OUT) && size)
		/*
		 * Zero the buffer on the stack so the user
		 * always gets back something deterministic.
		 */
		bzero((caddr_t)data, real_size);
	else if (com&IOC_VOID)
		*(caddr_t *)data = uap->cmarg;

	/* This assumes that the Ultrix 4.2 encoding of the command
	 * is the same as OSF/1. 
	 *
	 * The strategy is to convert Ultrix 4.2 commands to the
	 * equivalent OSF/1 command. This involves changing the command
	 * name to the new name and converting the passed data struct
	 * to the OSF/1 definition. After the command is processed, 
	 * the results are converted back to the Ultrix 4.2 definition.
	 * The conversions are done "in place" by data conversion
	 * functions.
	 *
	 * In this version it is expected that both the Ultrix 4.2 and 
	 * OSF/1 versions of the passed data are less than STK_PARMS
	 * which is currently 128 bytes.
	 */
	/* These are the supported Ultrix 4.2 commands */
	ult_cmd = com;
	switch (ult_cmd) {
	case  U_TIOCSETD:	/* Ultrix 4.2 and OSF/1 commands are
				 * the same however the line discipline
				 * is different and must be converted 
				 * when Ultrix 4.2 is calling */
		/* function expects a char */
		cvtLineUtoO(data);
		break;
	case  (u_int)U_TCGETP:		/* _IOR('t',85,struct termios) */
		com = TIOCGETA;
		size = sizeof(struct termios);
		break;
	case  (u_int)U_TCSANOW:	/* _IOW('t',84,struct termios) */
		com = TIOCSETA;
		size = sizeof(struct termios);
		ult_line = ult->c_line;
        	/* Read the current settings to get the current speed */
        	FOP_IOCTL(fp, TIOCGETA, &osf, error);
        	if(error)
			goto out;
		if(error = cvtsUtoO(data, &osf))
			goto out;
		break;
	case  (u_int)U_TCSADRAIN:	/* _IOW('t',83,struct termios) */
		com = TIOCSETAW;
		size = sizeof(struct termios);
		ult_line = ult->c_line;
        	/* Read the current settings to get the current speed */
        	FOP_IOCTL(fp, TIOCGETA, &osf, error);
        	if(error)
			goto out;
		if(error = cvtsUtoO(data, &osf))
			goto out;
		break;
	case  (u_int)U_TCSAFLUSH:	/* _IOW('t',82,struct termios) */
		com = TIOCSETAF;
		size = sizeof(struct termios);
		ult_line = ult->c_line;
        	/* Read the current settings to get the current speed */
        	FOP_IOCTL(fp, TIOCGETA, &osf, error);
        	if(error)
			goto out;
		if(error = cvtsUtoO(data, &osf))
			goto out;
		break;
	case  (u_int)U_TCSETAF:        /* _IOW('t',88,struct termio)  */
		com = TCSETAF;
		size = sizeof(struct termio);
		if(error = cvtUtoO(data))
			goto out;
		break;
	case  (u_int)U_TCSETAW:        /* _IOW('t',89,struct termio)  */
		com = TCSETAW;
		size = sizeof(struct termio);
		if(error = cvtUtoO(data))
			goto out;
		break;
	case  (u_int)U_TCSETA:         /* _IOW('t',80,struct termio)  */
		com = TCSETA;
		size = sizeof(struct termio);
		if(error = cvtUtoO(data))
			goto out;
		break;
	case  (u_int)U_TCGETA:         /* _IOR('t',91,struct termio)  */
		com = TCGETA;
		size = sizeof(struct termio);
		break;
	case  (u_int)U_TCFLSH:         /* _IO('t',92)    - termio      */
		com = TCFLSH;
		break;
	case  (u_int)U_TCXONC:         /* _IO('t',93)     - termio     */
		com = TCXONC;
		break;
	case  (u_int)U_TCSBRK:         /* _IO('t',94)     - termio     */
		com = TCSBREAK;
		break;
	case  (u_int)U_TIOCCAR:        /* _IO('t',95)                  */
	case  (u_int)U_TIOCNCAR:       /* _IO('t',96)                  */
	case  (u_int)U_TIOCWONLINE:    /* _IO('t',97)                  */
		error = ENOTTY;
		goto out;
		break;
	case  (u_int)U_TIOCMODEM:      /* _IO('t',98)                  */
		com = TIOCMODEM;
		break;
	case  (u_int)U_TIOCNMODEM:     /* _IO('t',99)                  */
		com = TIOCNMODEM;
		break;
	}

	switch (com) {
	case FIONBIO:
		/* was fset(fp, FNDELAY, *(int *)data); */
		error = fset(fp, FNONBLOCK, *(int *)data);
		break;

	case FIOASYNC:
		error = fset(fp, FASYNC, *(int *)data);
		break;

	case FIOSETOWN:
		error = fsetown(fp, *(int *)data);
		break;

	case FIOGETOWN:
		error = fgetown(fp, (int *)data);
		break;
	default:
		/*
		 * We expect the lower-level routine to lock and unlock
		 * the file structure as necessary.  We guarantee that the
		 * file structure won't disappear because we hold a reference
		 * on the structure courtesy of getf.
		 */

		/* XXX hack for STREAMS -- temporary */
		u.u_spare[0] = 0;
		/* end hack */

		FOP_IOCTL(fp, com, data, error);

		/* set line discipline as needed */
                if (error == 0 && !(ult_cmd&IOC_OUT)) {
                        switch (ult_cmd) {
                        case (u_int)U_TCSANOW:   /* _IOW('t',84,struct termios) */
                        case (u_int)U_TCSADRAIN: /* _IOW('t',83,struct termios) */
                        case (u_int)U_TCSAFLUSH: /* _IOW('t',82,struct termios) */
                                cvtLineUtoO(&ult_line);
				line = (int)ult_line;
                                FOP_IOCTL(fp, TIOCSETD, &line, error);
                                if(error)
                                        goto out;
                                break;
                        }
		}

		/*
		 * Copy any data to user, real_size was
		 * already set and checked above.
		 */
		if (error == 0 && (com&IOC_OUT) && real_size) {
			switch (ult_cmd) {
			case  U_TCGETP:	/* _IOR('t',85,struct termios) */
				cvtsOtoU(data);
				/* OSF termios doesn't include line 
				 * discipline so get it here */
				FOP_IOCTL(fp, TIOCGETD , &line, error);
				ult_line = (char)line;
				cvtLineOtoU(&ult_line);
				ult->c_line = ult_line;
				break;
			case U_TCGETA: /* _IOR('t',91,struct termio)  */
				cvtOtoU(data);
				break;
			case U_TIOCGETD:	/* Convert line desc */
				cvtLineOtoU(data);
				break;
			}
			error = copyout(data, uap->cmarg, (u_int)real_size);
		}

		/* XXX hack for STREAMS -- temporary */
		if (u.u_spare[0] != 0)
			*retval = u.u_spare[0];
		/* end hack */

		break;
	}
out:
	FP_UNREF(fp);
	if (memp)
		kfree(memp, size);
	return (error);
}



