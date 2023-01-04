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
 * @(#)$RCSfile: bt_data.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/11/17 23:12:42 $
 */

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
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
 * Modification History
 * 3-March-93 -- Jay Estabrook
 *	         created from ws_data.c
 *
 ************************************************************************/
#define _BT_DATA_C_

#include "tc.h"

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <io/common/devio.h>
#include <sys/buf.h>

/* #if NTC > 0 */
#include <io/dec/ws/bt463.h>		/* specific to BT463 VDAC 	*/
#include <io/dec/ws/bt459.h>		/* ditto */
#include <io/dec/ws/bt431.h>		/* ditto */
#include <io/dec/ws/pmagro.h>
#include <io/dec/ws/pmagbba.h>		/* ditto to hx module (sfb)	*/
#include <io/dec/ws/px.h>
#include <io/dec/ws/pa.h>
#include <io/dec/ws/pq.h>
#include <io/dec/ws/pv.h>
#ifndef __alpha
#include <io/dec/ws/bt455.h>		/* ditto */
#include <io/dec/ws/ims_g332.h>		/* not ditto */
#include <io/dec/ws/pmagaa.h>		/* ditto to mx module		*/
#include <io/dec/ws/pmagdv.h>
#include <io/dec/ws/vfb03.h>		/* specific to cfb module 	*/
#include <io/dec/ws/pmvdac.h> 		/* PMAX def			*/
#endif /* !__alpha */
/*#endif */

#ifdef BINARY

#else /*BINARY*/

#include "fb.h"
#include "px.h"
#include "pv.h"

#if (NFB+NPX+NPV) > 0
#if (NFB+NPX+NPV) < NUMSCREENS
#define BT_SCREENS	(NFB+NPX+NPV)
#else
#define BT_SCREENS	(NUMSCREENS)
#endif /*nfb+npx+npv<numscreens*/

/* #if NTC > 0 */

struct bt459info bt459_softc[BT_SCREENS];	/* up to one per option slot */
struct bt459info2 bt459_softc2[BT_SCREENS];	/* up to one per option slot */
/*
 * Bt459 VDAC info indexed by BT459_<type>_TYPE in bt459.h.
 */
struct bt459type bt459_type[] = {
#ifndef __alpha
    {
	(struct bt459 *)VFB03_BT459_OFFSET,	/* CX		   */
	220, 35,				/* offsets to video position */
	256, 0,					/* range of dirty entries */
	0,					/* reset */
	0xff,					/* mask */
    },
#else
/* must have a NULL entry for CX... :-( */
    {
	(struct bt459 *)NULL,			/* CX		   */
	0, 0,					/* offsets to video position */
	0, 0,					/* range of dirty entries */
	0,					/* reset */
	0,					/* mask */
    },
#endif /* !__alpha */
    {	
	(struct bt459 *)PA_BT459_OFFSET,	/* PX			   */
	370, 37,				/* offsets to video position */
	256, 0,					/* range of dirty entries */
	(caddr_t)PA_BT459_RESET,		/* reset */
	0xff,					/* mask */
    },
    {	
	(struct bt459 *)PQ_BT459_OFFSET,	/* PXG			   */
	370, 37,				/* offsets to video position */
	256, 0,					/* range of dirty entries */
	(caddr_t)PQ_BT459_RESET,		/* reset */
	0x0,					/* mask - to be probed */
    },
    {
	(struct bt459 *)PMAGBBA_BT459_OFFSET,	/* HX		   */
	219, 34,				/* offsets to video position */
	256, 0,					/* range of dirty entries */
	0,					/* reset */
	0xff,					/* mask */
    },
};
int nbt_types = sizeof(bt459_type) / sizeof(struct bt459type);
int nbt_softc = sizeof(bt459_softc) / sizeof(struct bt459info);

struct bt459type2 bt459_type2[] = {
  {
      (unsigned int *) PMAGDA_RAMDAC_SETUP_OFFSET,
      (unsigned int *) PMAGDA_RAMDAC_DATA_OFFSET,
      0,
      0x174, 0x27,
      250, 0,
      0,
      0xff,
    },
  };
int nbt_types2 = sizeof(bt459_type2) / sizeof(struct bt459type2);
int nbt_softc2 = sizeof(bt459_softc2) / sizeof(struct bt459info2);

#if (NFB+NPV) > 0

struct bt463info bt463_softc[NFB+NPV];
struct bt463info2 bt463_softc2[NFB+NPV];

struct bt463info bt463_type[] = {
    {	
	(struct bt463 *)TCO_FB_REGISTERS, /*fix this jmg */
	BT463_TCO_TYPE,		/* type */
	1,			/* screen initally on 	   */
	0,			/* colormap clean		   */
	0,			/* cursor map clean		   */
	219, 34,		/* magic offsets to video position */
	-1,			/* module unit number */
	NULL,			/* enable cfb interrupt on V.R.	   */
	NULL,			/* hack */
	{ 0, 0, 0, 0, },
	{ 0, 0, 0, 0, },
	{ 0, 0, 0, 0, },
	256, 0,
    },
    {	
	(struct bt463 *) PV_BT463_OFFSET, /*fix this jmg */
	BT463_PV_TYPE,
	1,			/* screen initally on 	   */
	0,			/* colormap clean		   */
	0,			/* cursor map clean		   */
	227, 34,		/* magic offsets to video position */
	-1,			/* module unit number */
	NULL,			/* enable cfb interrupt on V.R.	   */
	NULL,			/* hack */
	{ 0, 0, 0, 0, },
	{ 0, 0, 0, 0, },
	{ 0, 0, 0, 0, },
	256, 0,
    },
};

struct bt463info2 bt463_type2[] = {
    {
      (unsigned int *) PMAGDA_RAMDAC_SETUP_OFFSET,
      (unsigned int *) PMAGDA_RAMDAC_DATA_OFFSET,
      0,
      BT463_SFBP_TYPE2,
      1,                      /* screen initally on      */
      0,                      /* colormap clean                  */
      0,                      /* cursor map clean                */
      227, 34,                /* magic offsets to video position */
      -1,                     /* module unit number */
      NULL,                   /* enable cfb interrupt on V.R.    */
      NULL,                   /* hack */
      { 0, 0, 0, 0, },
      { 0, 0, 0, 0, },
      256, 0,
    },
};

struct bt431info bt431_softc[NFB+NPV];
struct bt431info bt431_type[] = {
#ifndef __alpha
    {	
	(struct bt431 *) PMAGAA_BT431_OFFSET,
	(struct bt431 *) NULL,
	BT431_PMAGAA_TYPE,
	1,			/* screen initally on 	   	   */
	1,			/* cursor initially on		   */
	0,		        /* cursor clean 		   */
	0x168, 0x24,		/* magic offsets to video position */
	0, 0,			/* hot spot of current cursor 	   */
	-1,			/* module unit number */
	0,			/* not initialized yet - hack!!!   */
	NULL,			/* enable cfb interrupt on V.R.	   */
    },
#else
/* must have a null entry for PMAG-AA... :-( */
    {	
	(struct bt431 *) NULL,
	(struct bt431 *) NULL,
	0,
	0,			/* screen initally on 	   	   */
	0,			/* cursor initially on		   */
	0,		        /* cursor clean 		   */
	0, 0,			/* magic offsets to video position */
	0, 0,			/* hot spot of current cursor 	   */
	0,			/* module unit number */
	0,			/* not initialized yet - hack!!!   */
	NULL,			/* enable cfb interrupt on V.R.	   */
    },
#endif /* !__alpha */
    {	
	(struct bt431 *) TCO_CURSOR0,
	(struct bt431 *) NULL,
	BT431_TCO_TYPE,
	1,			/* screen initally on 	   	   */
	1,			/* cursor initially on		   */
	0,		        /* cursor clean 		   */
	0x16c, 0x23,		/* magic offsets to video position */
	0, 0,			/* hot spot of current cursor 	   */
	-1,			/* module unit number */
	0,			/* not initialized yet - hack!!!   */
	NULL,			/* enable cfb interrupt on V.R.	   */
    },
    {	
	(struct bt431 *) PV_BT431_0_OFFSET,
	(struct bt431 *) PV_BT431_1_OFFSET,
	BT431_PV_TYPE,
	1,			/* screen initally on 	   	   */
	1,			/* cursor initially on		   */
	0,		        /* cursor clean 		   */
	0x170, 0x24,		/* magic offsets to video position */
	0, 0,			/* hot spot of current cursor 	   */
	-1,			/* module unit number */
	0,			/* not initialized yet - hack!!!   */
	NULL,			/* enable cfb interrupt on V.R.	   */
    },
};


#ifndef __alpha

struct bt455info bt455_softc[NFB+NPV];
struct bt455info bt455_type[] = {
    {	
	(struct bt455 *)PMAGAA_BT455_OFFSET,
	0, 0xFF,		/* cursor fg and bg value      	   */
	NULL,
	0,
	0,
	NULL,
	-1
    },
};

struct ims_g332info ims_g332_softc[NFB+NPV];
struct ims_g332info ims_g332_type[] = {
    {	
	0,			/* control reg cache! */
	1,			/* screen initally on 	   */
	1,			/* cursor initially on		   */
	0, 0,			/* cursor and colormap celan	   */
	1, 0,		 	/* magic offsets to video position */
	0, 0,			/* hot spot of current cursor 	   */
	NULL,			/* enable cfb interrupt on V.R.	   */
	{ 0, 0, 0, 0, },
	{ 0, 0, 0, 0, },
	{ 0, 0, 0, 0, },
	256, 0,
    },
};
#endif /* !__alpha */

struct sfbp_curs_info sfbp_curs_softc[BT_SCREENS];
struct sfbp_curs_info sfbp_curs_type[] = {
    {
      (unsigned int *) PMAGDA_XY_REG_OFFSET,
      (unsigned int *) PMAGDA_VALID_REG_OFFSET,
      219, 34,                                /* offsets to video position */
      -1,
      1,
      0,
      0,
      0, 0,
      -65,
      { 0xff, 0xff, 0xff, 0 },
      { 0, 0, 0 },
      (void (*)()) NULL,
    },
};

#endif /*nfb*/
/*#endif /*NTC > 0*/
#endif /*nfb*/

#ifndef __alpha
/*#ifdef DS3100 */

struct pmvdacinfo pm_softc[1];
struct pmvdacinfo pmvdac_type[] = {
    {
	0,					/* vdac */
	0,					/* pcc */
	212, 34,				/* offsets to video position */
	0, 0,					/* hotspot of cursor 	   */
	{ 0, 0, 0, 0, },
	{ 0, 0, 0, 0, },
	{ 0, 0, 0, 0, },
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
};
/*#endif ds3100*/
#endif /* !__alpha */

#endif BINARY
