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
static char *rcsid = "@(#)$RCSfile: str_config.c,v $ $Revision: 4.2.13.11 $ (DEC) $Date: 1993/12/15 20:02:53 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#include <streams/str_stream.h>
#include <streams/str_debug.h>
#include <sys/stropts.h>
#include <sys/sysconfig.h>

/*
 *	Configuration of statically bound modules.
 *	If sources for the module/driver are available, then the
 *	configuration call should probably go there. If not, then
 *	those few lines of code can be coded here.
 *
 *	In any case, insert the configure call in str_config().
 *
 *	Note: If the cfgmgr is not used and instead this static
 *	scheme is chosen as your default, the order of the optional
 *	device configuration calls is important. There are also device
 *	numbers in streamsm/standard.c.
 */

extern int	bufcall_configure();
extern int	strstd_configure();

/*
 * XTI/TLI modules and devices.
 */
#include <timod.h>
#include <tirdwr.h>
#include <xtiso.h>
#include <svvs.h>
extern int	svvs_configure();
extern int	timod_configure();
extern int	tirdwr_configure();
extern int	xtiso_configure();

/*
 * STREAMS tty modules and drivers
 */
#include <ldtty.h>
#include <rpty.h>
#include <pckt.h>

/*
 * misc modules and drivers
 */
#include <strifnet.h>
#include <dlpi.h>
#include <strkinfo.h>
#include <bba.h>

extern int	ldtty_configure();
extern int      kinfo_configure();
extern int 	pts_configure();  /* STREAMS slave side */
extern int 	ptm_configure();  /* SYS V style master side */
extern int 	pckt_configure();  /* SYS V style pty packet mode */
extern int      ifnet_configure();
extern int      strdlb_configure();
extern int      bba_configure();

void
strdev_conf(name, entrypt, buf, bufsize)
	char *			name;
	sysconfig_entrypt_t	entrypt;
	char *			buf;
	int			bufsize;
{
	int			retval;

	if (retval = (entrypt)(SYSCONFIG_CONFIGURE,
			buf, strlen(buf), NULL, 0)) {
#if STREAMS_DEBUG
		printf("STREAMS: %s CONFIGURE call failed (error=%d)\n",
			name, retval);
#endif
		return;
	}
}

void
strdev_print(sc, name, retval)
	str_config_t *sc;
	char *name;
	int retval;
{
	if (retval)
		printf("STREAMS: '%s' configure failed (%d)\n", name, retval);
#if	STREAMS_DEBUG
	else
		printf("STREAMS: '%s' configured (device %d/%d)\n",
		    sc->sc_sa_name, major(sc->sc_devnum), minor(sc->sc_devnum));
#endif
}

void
strmod_print(sc, name, retval)
	str_config_t *sc;
	char *name;
	int retval;
{
	if (retval)
		printf("STREAMS: module '%s' configure failed (%d)\n",
			name, retval);
}

void
str_config()
{
	str_config_t	sb, sc;
	
#define sc_size		sizeof(sc)
	int		maj, retval;
	char		str_parm_buf[256];

	bzero((caddr_t)&sb, sizeof(sb));
	sb.sc_version = OSF_STREAMS_CONFIG_10;

	/*
	 * Machine independent STREAMS modules
	 */
	retval = bufcall_configure(SYSCONFIG_CONFIGURE,
				NULL, 0, &sc, sc_size);
	strmod_print(&sc, "bufcall", retval);

#if	TIMOD && !TIMOD_DYNAMIC
	strdev_conf("timod", timod_configure, "subsys=timod\n", 14);
#endif

#if	TIRDWR && !TIRDWR_DYNAMIC
	strdev_conf("tirdwr", tirdwr_configure, "subsys=tirdwr\n", 15);
#endif

#if     STRIFNET && !STRIFNET_DYNAMIC
        retval = ifnet_configure(SYSCONFIG_CONFIGURE,
                                &sb, sc_size, &sc, sc_size);
        strdev_print(&sc, "ifnet", retval);
#endif

#if     LDTTY && !LDTTY_DYNAMIC
	retval = ldtty_configure(SYSCONFIG_CONFIGURE,
				NULL, 0, &sc, sc_size);
	strmod_print(&sc, "ldtty", retval);
#endif


	/*
	 * Machine independent STREAMS drivers
	 */

	bzero((caddr_t)&sb, sc_size);
	sb.sc_version = OSF_STREAMS_CONFIG_10;

#if	NRPTY && !RPTY_DYNAMIC
	{extern dev_t pts_cdev;

	sb.sc_devnum = pts_cdev;
	retval = pts_configure(SYSCONFIG_CONFIGURE,
				&sb, sc_size, &sc, sc_size);
	strdev_print(&sc, "pts", retval);
	}

	sb.sc_devnum = NODEV;
	retval = ptm_configure(SYSCONFIG_CONFIGURE,
				&sb, sc_size, &sc, sc_size);
	strdev_print(&sc, "ptm", retval);
#endif

#if	PCKT && !PCKT_DYNAMIC
	retval = pckt_configure(SYSCONFIG_CONFIGURE,
				NULL, 0, &sc, sc_size);
	strmod_print(&sc, "pckt", retval);
#endif
	
	/* Strstd_configure lives in streamsm/standard.c */
	while ((retval = strstd_configure(SYSCONFIG_CONFIGURE,
				NULL, 0, &sc, sc_size)) != ENOENT) {
		if (retval || (sc.sc_sa_flags & STR_IS_DEVICE))
			strdev_print(&sc, "std", retval);
		else
			strmod_print(&sc, "std", retval);
	}


        sb.sc_devnum = NODEV;   /* will find available slot */

#if	STRKINFO && !STRKINFO_DYNAMIC
	retval = kinfo_configure(SYSCONFIG_CONFIGURE,
				&sb, sc_size, &sc, sc_size);
	strdev_print(&sc, "kinfo", retval);
#endif

#if	XTISO && !XTISO_DYNAMIC
	/*
	 * Xtiso is multiply configurable...
	 */
	for (;;) {
		retval = xtiso_configure(SYSCONFIG_CONFIGURE,
					&sb, sc_size, &sc, sc_size);
		if (retval != EADDRINUSE)
			strdev_print(&sc, "xtiso???", retval);
		if (retval)
			break;
	}
#endif

	/* data link bridge */
#if     DLPI && !DLPI_DYNAMIC
        retval = strdlb_configure(SYSCONFIG_CONFIGURE,
                                &sb, sc_size, &sc, sc_size);
        strdev_print(&sc, "dlpi", retval);
#endif

#if	NBBA && !NBBA_DYNAMIC
	retval = bba_configure(SYSCONFIG_CONFIGURE,
				&sb, sc_size, &sc, sc_size);
	strdev_print(&sc, "bba", retval);
#endif

#if	SVVS && !SVVS_DYNAMIC
	/*
	 * See comments in streamsm/svvs.c
	 */
	for (;;) {
		retval = svvs_configure(SYSCONFIG_CONFIGURE,
					&sb, sc_size, &sc, sc_size);
		if (retval == ENOENT)
			break;
		if (retval) {
			if (sc.sc_sa_flags & STR_IS_DEVICE)
				strdev_print(&sc, "svvs???", retval);
			else
				strmod_print(&sc, "svvs???", retval);
			break;
		}
#if STREAMS_DEBUG
		if (sc.sc_sa_flags & STR_IS_DEVICE) {
			printf("STREAMS: SVVS '%s' configured (device %d/%d)\n",
				sc.sc_sa_name,
				major(sc.sc_devnum), minor(sc.sc_devnum));
		} else
			printf("STREAMS: SVVS '%s' configured\n",
				sc.sc_sa_name);
#endif 
	}
#endif


/*
 * add new configurations above this comment
 */

}
