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
 *	@(#)$RCSfile: lpio.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:58:38 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: (sysxprnt) Include file for use with 'lp' special file
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _SYS_LPIO_H_
#define _SYS_LPIO_H_


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* include file for use with the 'lp' special file                         */  
/* For more detailed information, see the AIX Technical Reference          */  
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <sys/termio.h>
#include <sys/ioctl.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/* extended open parms used for the extend operand in the open.           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */

#define LPDIAGMOD 1             /* Request diagnostic mode on open        */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* ioctl constants                                                         */  
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define LPR     ('l'<<8)
#define LPRGET  (LPR|01)	/* Get margin parameters                   */
#define LPRSET  (LPR|02)	/* Set margin parameters                   */
#define LPRGETA (LPR|17)	/* Get RS232 parameters                    */
#define LPRSETA (LPR|18)	/* Set RS232 parameters                    */
#define LPRGTOV (LPR|19)	/* Get the time-out value, seconds         */
#define LPRSTOV (LPR|20)	/* Set the time-out value, seconds         */
#define LPQUERY	(LPR|23)	/* Query Command		           */
#define LPDIAG	(LPR|26)	/* Parallel printer diagnostics            */
#define LPRMODG	(LPR|27)	/* Get printer modes  		           */
#define LPRMODS	(LPR|28)	/* Set printer modes  		           */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*	Declarations of structures used with lp ioctls                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* margin parameters structure used with LPRGET and LPRSET                 */
struct lprio {
	int	ind;		/* indent size			           */
	int	col;		/* columns per line		           */
	int	line;		/* lines per page		           */
};


/* printer mode structure used with LPRMODG and LPRMODS                    */
/* Note: The default value for all modes is off.                           */
struct lprmod {
       int  modes;         /* line printer modes                           */
#define PLOT      0x0001   /* if on, bypass all formatting/interpretation  */
                           /* modes.  NOFF, NONL, NOCL, NOTAB, NOBS, NOCR  */
                           /* CAPS and WRAP are ignored.                   */
#define NOFF      0x0002   /* if on, simulate form feed function with      */
                           /* line feeds, based on line value above.       */
#define NONL      0x0004   /* if on, substitute carriage returns for any   */
                           /* line feeds                                   */
#define NOCL      0x0008   /* if on, do not insert a carriage return after */
			   /* each line feed                               */
#define NOTB      0x0010   /* if on, do not expand tabs. if off, simulate  */
                           /* 8 position tabs with spaces.                 */
#define NOBS      0x0020   /* if on, ignore backspace characters           */
#define NOCR      0x0040   /* if on, substitute line feeds for any         */
                           /* carriage returns                             */
#define CAPS      0x0080   /* if on, map lower-case alphabetics to upper   */
                           /* case                                         */
#define WRAP      0x0100   /* if on, print characters beyond the page      */
                           /* width on the next line. If off, truncate     */
                           /* characters extending beyond page width.      */
#define FONTINIT  0x0200   /* if on, fonts have been initialized           */
#define RPTERR    0x0400   /* if on, errors are reported back to the       */
                           /* application.  If off, the device driver will */
                           /* not return until the error is cleared or     */
                           /* a cancel signal is received.                 */
/* Temporary Definitions and Declarations */
#define DOALLWRITES	01000000
};


/* timeout structure used with LPRGTOV and LPRSTOV                         */
struct lptimer {  
	int      v_timout;      /* time out value in seconds               */
};


struct lpquery {
	int	status;		/* device status	                   */
#define LPST_POUT	0x01	/* paper out condition		           */
#define LPST_TOUT	0x02	/* printer timeout - intervention required */
#define LPST_ERROR	0x04	/* unspec. error, intervention required	   */
#define LPST_BUSY	0x08	/* printer busy, may not be timeout yet	   */
#define LPST_NOSLCT	0x020	/* printer is not selected		   */
#define LPST_SOFT	0x040	/* software error			   */
	int	tadapt;		/* adapter type		                   */
	int	reccnt;		/* number of bytes in rec buffer           */
	char	rsvd[12];	/* reserved, set to binary value of 0      */
};


/* diagnostics structure used with LPDIAG, parallel printers only            */
struct lpdiag {
	int	cmd;		/* command to perform	                     */
#define LP_R_STAT	0x01	/* put contents of status reg in value field */
#define LP_R_CNTL	0x02	/* put contents of contrl reg in value field */
#define LP_W_CNTL	0x03	/* put contents of value field in contrl reg */
#define LP_R_DATA	0x04	/* put contents of data reg in value field   */
#define LP_W_DATA	0x05	/* put contents of value field in data reg   */
#define LP_WATCHINT     0x06    /* Watch for an interrupt to occur        */
#define LP_DIDINTOCC    0x07    /* Did an interrupt occur                    */
	int	value;		/* data read or to be written, 0 - 255	     */
#define LP_INTDIDOCC    0x01    /* Interrupt did occur                       */
#define LP_INTNOTOCC    0x02    /* Interrupt did not occur                   */
	char	rsvd[12];	/* reserved, set to binary value of 0	     */
};


/* RS232 parameter change structure for LPRGETA and LPRSETA */
struct lpr232 {
	unsigned c_cflag;
};
typedef volatile unsigned char REGTYPE; /* register data type */

struct ppdds {
	int     bus_type;       /* for intr.h struct            */
	int     level;          /* for intr.h struct            */
	int     priority;       /* for intr.h struct            */
	int     bus_id;         /* for intr.h struct            */
	int     i_flags;        /* for intr.h struct            */
	int     interface;      /* type of interface for status */
#define PPIBM_PC          0     /* Standered IBM PC interface   */
#define PPCONVERGED       1     /* Convereg interface           */
#define ppIBM_PC     PIBM_PC    /* BEING REMOVED DO NOT USE     */
#define ppCONVERGED  PPCONVERGED/* BEING REMOVED DO NOT USE     */
	char    name[16];       /* device name (/dev/lp0  )     */
	int     slot;
	int     v_timeout;
	int     pp_lin_default;
	int     pp_col_default;
	int     pp_ind_default;
	int     rg;
};

#endif    /* _SYS_LPIO_H_ */
