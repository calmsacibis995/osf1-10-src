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
 * static char *rcsid = "@(#)$RCSfile: template.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/05/12 15:15:06 $";
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/*
 *	template for the initialization routine for a module/driver
 *
 *	- #define STRNAME to driver/module name (with quotes)
 *	  (maximum length FMNAMESZ, which is fixed at 8 + trailing 0).
 *
 *	- #define STRCONFIG to the configure entry point's name
 *
 *	- #define STRINFO to the driver/module's info structure name.
 *
 *	- #define STRFLAGS to STR_IS_DEVICE or STR_IS_MODULE with others
 *	  as appropriate (e.g. STR_SYSV4_OPEN).
 *
 *	- #define STRSYNCL, STRSYNCI, STRTTYS to appropriate values if
 *	  not the defaults below. See the manual.
 *
 *	- Include this file into module's source file,
 *	  preferably after the streamtab definition
 *
 *	- The input buffer, indata, should be of the form "variable=value\n".
 *	  It should always begin with "subsys=subsystem-name\n" and it should
 *	  end with a '\0'. An example would be a subsystem, foo,
 *	  which wants to use a specific device number.  The input buffer,
 *	  indata, should be "subsys=foo\ndevno=123\n\0".
 *		
 *	- If your driver should take a specific character device number,
 *	  be sure to pass it within indata (e.g devno=123), else look at 
 *	  outdata for assigned value.
 *
 *	- Apart from this change, you'll have to update
 *
 *		- kernel/streams/str_config.c
 *		  (insert a call to this routine)
 *		- kernel/conf/files
 *		  (insert module's source file)
 */

#include <sys/sysconfig.h>

/* Version */
#ifndef	STRVERS
#define STRVERS		OSF_STREAMS_11
#endif
/* Default to module ??? */
#ifndef	STRFLAGS
#define STRFLAGS	STR_IS_MODULE|STR_SYSV4_OPEN
#endif
/* Minimal sync from STREAMS framework. >= SQLVL_QUEUEPAIR ??? */
#ifndef	STRSYNCL
#define STRSYNCL	SQLVL_QUEUE
#endif
#ifndef	STRSYNCI
#define STRSYNCI	0
#endif
/* Not a tty driver/module. */
#ifndef	STRTTYS
#define STRTTYS		0
#endif

#if	!defined(STRCONFIG) || !defined(STRNAME) || !defined(STRINFO)
#error Must pre-define STRCONFIG, STRNAME, and STRINFO.
#endif

static int tmpl_maj;
static struct streamadm	tmpl_sa;


int
STRCONFIG (op, indata, indatalen, outdata, outdatalen)
        sysconfig_op_t  op;
        char	     *  indata;
        size_t          indatalen;
        char	     *  outdata;
        size_t          outdatalen;
{
	static dev_t		devno;
	int                     configured;
	int			size;
	int 		  	ret = 0;

	switch (op) {

	case SYSCONFIG_CONFIGURE:

	    /*
	     * If an input buffer has been supplied, then look for a devno
	     * entry.  If found then devno is set to the given value.  If a 
	     * user hasn't specified a device number, then initialize it to 
	     * NODEV.
	     */

	    if (indata != NULL && indatalen == sizeof(str_config_t)
		&& ((str_config_t *)indata)->sc_version == OSF_STREAMS_CONFIG_10)
		  devno = ((str_config_t *)indata)->sc_devnum;
	    else
		  devno = NODEV;

            tmpl_sa.sa_version           = STRVERS;
            tmpl_sa.sa_flags             = STRFLAGS;
            tmpl_sa.sa_ttys              = STRTTYS;
            tmpl_sa.sa_sync_level        = STRSYNCL;
            tmpl_sa.sa_sync_info         = STRSYNCI;
            strcpy(tmpl_sa.sa_name,      STRNAME);

            if ((devno = strmod_add(devno, &STRINFO, &tmpl_sa)) == NODEV)
                ret = ENODEV;

            if (outdata && outdatalen>=0)
                bcopy(indata,outdata,outdatalen);
	    break;

	default:
	    ret = EINVAL;
	    break;
	}

	return(ret);
}
