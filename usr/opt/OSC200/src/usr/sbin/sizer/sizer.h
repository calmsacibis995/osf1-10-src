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
/*	@(#)$RCSfile: sizer.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/11/09 21:55:09 $                                       */
/************************************************************************
 *									*
 *			Copyright (c) 1987 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/


/************************************************************************
 *
 * Name: sizer.h
 *
 * Modification History
 *
 * Jun 5, 1991 - jaa
 *	ported to OSF/1.  modified to use new bus/controller/device
 *	kernel structures.  in porting, we don't use /dev/kmem anymore.
 *	getsysinfo(3) was modified to return the needed information.
 *	NOTE: ifdef TODO's in this code will have to be done as the 
 *	pieces are added (i.e. floating csr space, workstation specifics)
 *
 * Aug 10, 1990 - Randall Brown
 *	Added include file <sys/devio.h>
 *
 * Dec 12, 1989 - Alan Frechette
 *	Added include file <sys/sysinfo.h>. Added new field
 *	"alive_unit" to alive device structure.
 *
 * July 14, 1989 - Alan Frechette
 *	Added define for XMINODE.
 *
 * Jun 19, 1989 - Darrell Dunnuck (darrell)
 *	Fixed include of cpuconf.h -- had given Alan wrong info.
 * 
 * May 10, 1989 - Alan Frechette
 *	Added a new name list element "NL_roottype" for determining 
 *	network boots.
 *
 * May 02, 1989 - Alan Frechette
 *	Changes to deal with new unique "cpu" handling for both
 *	vax and mips architectures. 
 *
 * Feb 12, 1989 - Alan Frechette
 *	New sizer code which supports multiple architectures.
 *      This file is a complete redesign and rewrite of the 
 *	original V3.0 sizer code by Tungning Cherng.
 *
 ***********************************************************************/

#include <stdio.h>
#include <signal.h>
#include <nlist.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <machine/hal_sysinfo.h>
#include <sys/param.h>
#include <sys/vm.h>
#include <sys/stat.h>
#include <sys/buf.h>
#include <io/common/devio.h>

#include <machine/cpuconf.h>
#include <machine/devdriver.h>

/************************************************************************/
/*									*/
/* The following declarations define the CONFIGURATION FILE DEFINITION  */
/* TYPES, the CONFIGURATION FILE TABLE STRUCTURES, an the ALIVE DEVICE  */
/* LIST STRUCTURE.  						        */
/*									*/
/************************************************************************/

/* The CONFIGURATION FILE device definition types */
#define UNKNOWN	       -1
#define	BUS		0
#define	CONTROLLER	1
#define	MASTER		2
#define	DISK		3
#define	TAPE		4
#define	DEVICE		5

/* The CONFIGURATION FILE node definition types */
#define BINODE	0
#define CINODE		1
#define MSINODE		2
#define XMINODE		3

/* The CONFIGURARTION FILE bus definition types */
#define UNIBUS		0

#define MAXDEVICES		400
#define PATHSIZE		255
#define DEVNAMESIZE		20
#define MAXUPRC			50
#define NODISPLAY		0
#define DISPLAY			1

/* Structure for configuring the NODE information. */
struct config_node {
    char *conn_name;		/* The connection point name */
    char *nodename;		/* The associated node name  */
    int nodetype;		/* The associated node type  */
};

/* Structure for configuring the CPU information. */
struct config_cpu {
    int cputype;		/* The type of the system CPU	*/
    char *cpuname;		/* The name of the system CPU  	*/
    int maxusers;		/* Maximum # of users allowed	*/
    char *bootdev;		/* The name of the boot device  */
};

/* Structure for configuring the DEVICE information. */
struct config_device {
    char *devname;		/* The name of the I/O device	*/
    char *devstr;		/* Generic name of the device	*/
    int makedevflag;		/* Make device special file flag*/
				/* (1-MAKEDEV)	(0-NO MAKEDEV)	*/
    int supportedflag;		/* Supported I/O device flag	*/
				/* (1-SUPPORTED) (0-UNSUPPORTED)*/
    char *ivectors;		/* The interrupt vectors 	*/
    int flags;			/* Flags field of I/O device	*/
    int devtype;                /* Generic type of I/O device   */
};

/* Structure for defining the configuration file hardware name. */
struct hardware {
	char	*typename;	/* The hardware device name */
};

/* Structure for defining the FLOATING ADDRESS DEVICES information. */
struct float_devices {
	char 	*name;		/* Floating address device name */
	int 	gap;		/* Floating address device gap 	*/
};

/*
 * Structure used for AUTO-CONFIGURATION of the system. As sizer
 * figures out the configuration for the system it places each 
 * alive adapter, controller, and device it finds in this structure.
 */
struct alive_device_list {
    short bus_type;		/* The bus type (UNIBUS ...)    	*/
    short device_index;		/* Index of device in "devtbl" table 	*/
    short device_unit;  	/* The unit # of the device		*/
    short device_drive;		/* The drive # or slave # of the device	*/
    char  unsupp_devname[10];	/* The unsupported device name          */
    char  conn_name[10];	/* The name of the connection point     */
    short conn_number;		/* The connection number		*/
    short node_number;		/* The node number      		*/
    short node_index;		/* Index of node in "nodetbl" table 	*/
    short rctlr;		/* The remote controller number	        */
    long  csr;			/* The csr addr of device in I/O space  */
    short alive_unit;		/* The device is alive		        */
} adltbl[MAXDEVICES];


/************************************************************************/
/*									*/
/* The following declarations define the GLOBAL INFORMATION used by the	*/
/* "sizer" program.							*/
/*									*/
/************************************************************************/

/* The NAMELIST elements to lookup in the kernel image file */
#define	NL_umem			1
#define	NL_qmem			2
#define	NL_ci_first_port	3
#define	NL_dmb_lines		4
#define	NL_dhu_lines		5

char	pname[DEVNAMESIZE];
int	Cpu, Maxcpu, Adlindex; 
FILE 	*Fp, *Fpdevs;
int 	tc_callout;
int 	eisa_callout;
int	Scs_sysidflag;
#ifdef TODO_FLOAT
int     Float_flag;
#endif
long 	reset_anythg();


/* External declarations for the CONFIGURATION FILE TABLES */
extern 	struct config_node		nodetbl[];
extern 	struct config_cpu		cputbl[];
extern 	struct config_device		devtbl[];
extern	struct nlist			nl[];
extern 	struct hardware			hardtbl[];
extern 	struct float_devices		floattbl[];

/* External declarations for the CONFIGURATION FILE TABLES SIZES */
extern 	int Nodetblsize;
extern 	int Cputblsize;
extern 	int Devtblsize;
extern 	int Nltblsize;
extern 	int Hardtblsize;
extern 	int Flttblsize;



























