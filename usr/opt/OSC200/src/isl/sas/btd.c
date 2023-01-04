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
static char *rcsid = "@(#)$RCSfile: btd.c,v $ $Revision: 1.2.3.5 $ (DEC) $Date: 1992/06/24 15:08:42 $";
#endif
/*
 * Modification History
 *
 * 17-Sep-1990 Joe Szczypek
 *	Added TURBOchannel console support.  
 *
 * 18-Dec-1989 Jon Wallace
 *	Added multiple controller support for tz and rz devices.
 *
 * 14-Aug-1989 Jon Wallace
 *	Added Calypso support.
 *
 * 24-May-1989 Jon Wallace
 *	Added MipsFAIR support.
 *
 * 01-May-1989 Jon Wallace
 *	Added TEAMATE support.
 *
 * 29-Jun-88 tresvik
 * 	Added support for tape units other than 0.  This was needed to
 *	to support Lynx and Pvax.  
 * 
 * 7-Jun-88 Jon Wallace
 * 	added PVAX support for TZ30.  Note UWS distribution assumed.
 *
 * 23-sep-88 Jon Wallace
 *	Modified Pvax code to handle dual controllers
 *
 * 2-mar-1989 Jon Wallace
 *      Modified code to include TEAMMATE II support.
 *
 */

#include <sys/file.h>
#include <sys/types.h>
#include <machine/cpuconf.h>
#ifdef vax
#include "../machine/vax/rpb.h"
#include "../sas/vax/vmb.h"
#endif vax

#include <sys/sysinfo.h>
#include <machine/hal_sysinfo.h>

main()
{
#ifdef vax
    struct rpb info;
    int fd;

    if((fd = open("/dev/kmem", O_RDONLY)) == -1){
	perror("Couldn't open /dev/kmem");
	exit(1);
    }

    lseek(fd, 0x80000000, 0);
    read(fd, &info, sizeof(struct rpb));
    switch (info.devtyp)
    {
	case BTD$K_QNA:
	case BTD$K_KA640_NI:
		printf("NETWORK");
		break;

	case BTD$K_SII:
		printf("rz%d", info.unit/100);
		break;

	case BTD$K_TK50:
	case BTD$K_AIE_TK50:
		printf("tms%d", info.unit);
		break;

	case BTD$K_KA640_TAPE:
		/*
		 * for the PVAX we assume UWS distribution
		 */
		if ((info.cpu == C_VAXSTAR) && 
		    (info.cpu_subtype == ST_VAXSTAR) &&
                        (info.ws_display_type)) {
			printf("tz%d", ((info.unit/100) + ((info.ctrllr - 1) * 8))); 
			break;
		}
		if (info.ws_display_type)
			printf("st%d", info.unit);
		break;

	case BTD$K_KA420_DISK:
		printf("rz%d", ((info.unit/100) + ((info.ctrllr - 1) * 8)));
		break;

	default:
		break;
    }
#endif vax
#if defined(mips) || defined(__alpha)
#define BOOTDEVLEN 80
	static char	bootdev[BOOTDEVLEN] = "";
	static char	bootctlr[4] = "";
	static char	constype[4] = "";
	char	*cp, cn;

	getsysinfo(GSI_BOOTDEV, bootdev, sizeof(bootdev));
	getsysinfo(GSI_BOOTCTLR, bootctlr, sizeof(bootctlr));
	getsysinfo(GSI_CONSTYPE, constype, sizeof(constype));

	if (strncmp(constype,"TCF0", 4)==0) {
		if(!strncmp(bootdev+2,"mop",3)) {
			printf("NETWORK");
		}
		if(!strncmp(bootdev+2,"tftp",4)) {
			printf("NETWORK");
		}
		if(!strncmp(bootdev+2,"rz",2)) {
			cn = atoi(bootctlr);
			printf("rz%d",cn * 8 + atoi(bootdev+4));
		}	
		if(!strncmp(bootdev+2,"tz",2)) {
			cn = atoi(bootctlr);
			printf("tz%d",cn * 8 + atoi(bootdev+4));
		}	

	} else {

#ifdef __alpha
		if (!strncmp(bootdev, "MOP", 3)) {
			printf("MOP");
		} 
		else if(!strncmp(bootdev,"BOOTP",5)) {
			printf("BOOTP");
		}
#ifdef notyet
		else if (!strncmp(bootdev, "tm", 2)) {
			cp = bootdev;
			while ((*cp++ != ',') && *cp);
			printf("tms%d", atoi(cp)); 
		}
		else if (!strncmp(bootdev, "tz", 2)) {
			cp = bootdev;
			while ((*cp++ != '(') && *cp);
			cn = atoi(cp);
			while ((*cp++ != ',') && *cp);
			printf("tz%d", cn * 8 + atoi(cp)); 
		}
#endif /* notyet */
		else if (!strncmp(bootdev, "rz", 2)) {
			printf("rz%d", atoi(bootdev+2)); 
		}
		else if (!strncmp(bootdev, "ra", 2)) {
			printf("ra%d", atoi(bootdev+2)); 
		}
#else /* __alpha */
		if (!strncmp(bootdev, "mop", 3)) {
			printf("NETWORK");
		} 
		else if(!strncmp(bootdev,"tftp",4)) {
			printf("NETWORK");
		}
		else if (!strncmp(bootdev, "tm", 2)) {
			cp = bootdev;
			while ((*cp++ != ',') && *cp);
			printf("tms%d", atoi(cp)); 
		}
		else if (!strncmp(bootdev, "tz", 2)) {
			cp = bootdev;
			while ((*cp++ != '(') && *cp);
			cn = atoi(cp);
			while ((*cp++ != ',') && *cp);
			printf("tz%d", cn * 8 + atoi(cp)); 
		}
		else if (!strncmp(bootdev, "rz", 2)) {
			cp = bootdev;
			while ((*cp++ != '(') && *cp);
			cn = atoi(cp);
			while ((*cp++ != ',') && *cp);
			printf("rz%d", cn * 8 + atoi(cp)); 
		}
#endif /* __alpha */
		else
			printf("%s", bootdev);
	}
#endif /* mips || __alpha */
exit(0);
}
