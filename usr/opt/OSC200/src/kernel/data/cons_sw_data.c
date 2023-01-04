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
 *	@(#)$RCSfile: cons_sw_data.c,v $ $Revision: 1.2.18.5 $ (DEC) $Date: 1993/11/17 23:12:45 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from cons_sw_data.c	4.4      (ULTRIX)  9/6/90";
 */

/*
 * Modification History
 *
 * 14-jan-92 - prm
 *      Modified routine names for ruby to match DUART device, 
 *      rather than platform.
 *
 * 03-Oct-91 - ald
 *      Added alpha support for FLAMINGO.
 *
 * 25-Sep-91 - prm
 *      Added ruby support.
 *
 * 10-May-91    Paul Grist
 *	Added Entries for 3MAX+/BIGMAX (DS5000_300).
 *
 * 03-May-91 - afd
 *      Added alpha support for ADU.
 *
 * 05-May-91	Joel Gringorten
 *	Changes for new WS graphics driver.
 *
 * 20-Dec-90 - Kuo-Hsiung Hsieh
 *	Changed DS5100 entry.  Mipsmate driver is now merged into DS3100.
 *	OSF started.
 *	-----------------------------------------------------------------
 *
 * 06-Sep-90 - Randall Brown
 *	Removed entries for 'cfb' and 'pm', they are now folded into fb driver
 *
 * 03-Apr-90 - Tim Burke
 * 	Added console support for Mipsmate (DS5100).
 *
 * 27-Feb-90 - Philip Gapuzte
 *	Added entries for DS5000_100 (3MIN) console ssc.
 *
 * 04-Dec-89 - Sam Hsu
 *	Added entries for DS5000 graphics accelerators to vcons_init.
 *
 * 29-Oct-89 - Randall Brown
 *
 *	Added the c_init entry to each switch entry.  Also added the 
 *	vcons_init table.
 *
 * 20-May-89 - Randall Brown
 *
 *	created file.
 *
 */
#include <hal/cons_sw.h>
#include <mach/machine/vm_types.h>	/* for vm_offset_t in cpuconf.h */
#include <hal/cpuconf.h>


extern int nocons();
extern int noconsmmap();

#if defined(DS3100) || defined(DS5000) || defined(DS5100) 
int dcopen(), dcclose(), dcread();
int dcwrite(), dcioctl(), dcstart();
int dcstop(), dcselect(), dcputc(), dcgetc();
int dcprobe(), dcintr(), dc_cons_init();
int dcparam(), dcmmap();
extern struct tty dc_tty[];
#endif /* DS3100 */

#if defined(DS5000_100) || defined(DS5000_300) || defined(DSPERSONAL_DECSTATION)
int sccopen(), sccclose(), sccread();
int sccwrite(), sccioctl(), sccstart();
int sccstop(), sccselect(), sccputc(), sccgetc();
int sccprobe(), sccintr(), scc_cons_init();
int sccparam(), sccmmap();
extern struct tty scc_tty[];
#endif

#if defined(DS5400) || defined(DS5800) || defined(DS5500)
int ssc_cnopen(), ssc_cnclose(), ssc_cnread();
int ssc_cnwrite(), ssc_cnioctl(), ssc_cnrint();
int ssc_cnxint(), ssc_cnstart(), ssc_cnputc();
int ssc_cngetc(),ttselect();
int ssc_cninit();
extern struct tty cons[];
#endif

#ifdef ALPHAADU
int adu_cnopen(), adu_cnclose(), adu_cnread();
int adu_cnwrite(), adu_cnioctl(), adu_cnrint();
int adu_cnxint(), adu_cnstart(), adu_cnputc();
int adu_cngetc(), adu_cons_init(), ttselect();
int adu_param();	/* not used yet; set to nocons */
extern struct tty cons[];
#endif

#ifdef DEC2000_300
int abc_cnopen(), abc_cnclose(), abc_cnread();
int abc_cnwrite(), abc_cnioctl(), abc_cnrint();
int abc_cnxint(), abc_cnstart(), abc_cnputc();
int abc_cngetc(), abc_cons_init(), abc_cnselect();
int abcprobe();
int abc_param();	/* not used yet; set to nocons */
extern struct tty cons[];
#endif /* DEC2000_300 */

#ifdef DEC7000
int alpha_8530_cnopen(), alpha_8530_cnclose(), alpha_8530_cnread();
int alpha_8530_cnwrite(), alpha_8530_cnioctl(), alpha_8530_cnrint();
int alpha_8530_cnxint(), alpha_8530_cnstart(), alpha_8530_cnputc();
int alpha_8530_cngetc(), alpha_8530_cons_init(), ttselect();
int ruby_param(), alpha_8530_cons_probe();	/* not used yet; set to nocons */
extern struct tty cons[];
#endif

#ifdef DEC4000
int cobra_cnopen(), cobra_cnclose(), cobra_cnread();
int cobra_cnwrite(), cobra_cnioctl(), cobra_cnrint();
int cobra_cnxint(), cobra_cnstart(), cobra_cnputc();
int cobra_cngetc(), cobra_cons_init(), ttselect();
int cobra_param(), cobra_cons_probe();	/* not used yet; set to nocons */
extern struct tty cons[];
#endif

#if defined(DEC3000_500) || defined(DEC3000_300)
int sccopen(), sccclose(), sccread();
int sccwrite(), sccioctl(), sccstart();
int sccstop(), sccselect(), sccputc(), sccgetc();
int sccprobe(), sccintr(), scc_cons_init();
int sccparam(), sccmmap();
extern struct tty scc_tty[];
#endif /* DEC3000_500 || DEC3000_300 */

struct cons_sw cons_sw[] =
{	/* no system */
#ifdef DS3100 
	{	/* PMAX - DECstation 3100 */
	  DS_3100,		dcopen,			dcclose,
	  dcread,	  	dcwrite,		dcioctl,
	  dcintr,		dcintr,			dcstart,
	  dcstop,		dcselect,	        dcputc,
	  dcprobe,		dcgetc,			dc_cons_init,
	  dcparam,		dcmmap,			dc_tty,
	},
#endif /* DS3100 */

#ifdef DS5100
	{	/* MIPSMATE - DECsystem 5100 */
	  DS_5100,		dcopen,			dcclose,
	  dcread,	  	dcwrite,		dcioctl,
	  dcintr,		dcintr,			dcstart,
	  dcstop,		dcselect,	        dcputc,
	  dcprobe,		dcgetc,			dc_cons_init,
	  dcparam,		dcmmap,			dc_tty,
	},
#endif /* DS5100 */

#ifdef DS5400
	{	/* MIPSFAIR */
	  DS_5400,		ssc_cnopen,		ssc_cnclose,
	  ssc_cnread,		ssc_cnwrite,		ssc_cnioctl,
	  ssc_cnrint,		ssc_cnxint,		ssc_cnstart,
	  nocons,		ttselect,		ssc_cnputc,
	  nocons,		ssc_cngetc,		ssc_cninit,
	  nocons,		noconsmmap,		cons,
	},
#endif /* DS5400 */

#ifdef DS5800
	{	/* ISIS */
	  DS_5800,		ssc_cnopen,		ssc_cnclose,
	  ssc_cnread,		ssc_cnwrite,		ssc_cnioctl,
	  ssc_cnrint,		ssc_cnxint,		ssc_cnstart,
	  nocons,		ttselect,		ssc_cnputc,
	  nocons,		ssc_cngetc,		ssc_cninit,
	  nocons,		noconsmmap,		cons,
	},
#endif /* DS5800 */

#ifdef DS5000
	{	/* 3MAX - DECstation 5000 */
	  DS_5000,		dcopen,			dcclose,
	  dcread,	  	dcwrite,		dcioctl,
	  dcintr,		dcintr,			dcstart,
	  dcstop,		dcselect,	        dcputc,
	  dcprobe,		dcgetc,			dc_cons_init,
	  dcparam,		dcmmap,			dc_tty,
	},
#endif /* DS5000 */

#ifdef DS5500
	{	/* MIPSFAIR - 2 */
	  DS_5500,		ssc_cnopen,		ssc_cnclose,
	  ssc_cnread,		ssc_cnwrite,		ssc_cnioctl,
	  ssc_cnrint,		ssc_cnxint,		ssc_cnstart,
	  nocons,		ttselect,		ssc_cnputc,
	  nocons,		ssc_cngetc,		ssc_cninit,
	  nocons,		noconsmmap,		cons,
	},
#endif /* DS5500 */

#ifdef DS5000_100 
	{	/* 3MIN */
	  DS_5000_100,		sccopen,		sccclose,
	  sccread,	  	sccwrite,		sccioctl,
	  sccintr,		sccintr,		sccstart,
	  sccstop,		sccselect,	        sccputc,
	  sccprobe,		sccgetc,		scc_cons_init,
	  sccparam,		sccmmap,		scc_tty,
	},
#endif /* DS5000_100 */

#ifdef DSPERSONAL_DECSTATION 
	{	/* MAXine */
	  DS_MAXINE,		sccopen,		sccclose,
	  sccread,	  	sccwrite,		sccioctl,
	  sccintr,		sccintr,		sccstart,
	  sccstop,		sccselect,	        sccputc,
	  sccprobe,		sccgetc,		scc_cons_init,
	  sccparam,		sccmmap,		scc_tty,
	},
#endif /* DSPERSONAL_DECSTATION */

#ifdef DEC2000_300
        {       /* Jensen Alpha PC */
          DEC_2000_300,         abc_cnopen,             abc_cnclose,
          abc_cnread,           abc_cnwrite,            abc_cnioctl,
          abc_cnrint,           abc_cnxint,             abc_cnstart,
          nocons,               abc_cnselect,           abc_cnputc,
          abcprobe,             abc_cngetc,             abc_cons_init,
          nocons,		noconsmmap,		cons,
        },
#endif

#ifdef ALPHAADU
        {       /* Alpha ADU */
          ALPHA_ADU,            adu_cnopen,             adu_cnclose,
          adu_cnread,           adu_cnwrite,            adu_cnioctl,
          adu_cnrint,           adu_cnxint,             adu_cnstart,
          nocons,               ttselect,               adu_cnputc,
          nocons,               adu_cngetc,             adu_cons_init,
          nocons,		noconsmmap,		cons,
        },
#endif

#ifdef DEC7000
        {       /* Alpha ADU */
          DEC_7000,           	     alpha_8530_cnopen,            alpha_8530_cnclose,
          alpha_8530_cnread,          alpha_8530_cnwrite,           alpha_8530_cnioctl,
          alpha_8530_cnrint,          alpha_8530_cnxint,            alpha_8530_cnstart,
          nocons,               ttselect,               alpha_8530_cnputc,
          alpha_8530_cons_probe,      alpha_8530_cngetc,            alpha_8530_cons_init,
          nocons,		noconsmmap,		cons,
        },
#endif

#ifdef DEC4000
        {       /* Alpha Cobra */
          DEC_4000,             cobra_cnopen,           cobra_cnclose,
          cobra_cnread,         cobra_cnwrite,          cobra_cnioctl,
          cobra_cnrint,         cobra_cnxint,           cobra_cnstart,
          nocons,               ttselect,               cobra_cnputc,
          cobra_cons_probe,     cobra_cngetc,           cobra_cons_init,
          nocons,		noconsmmap,		cons,
        },
#endif

#ifdef DEC3000_500
        {       /* Alpha FLAMINGO SCC console; from DS5000 */
	  DEC_3000_500,		sccopen,		sccclose,
	  sccread,	  	sccwrite,		sccioctl,
	  sccintr,		sccintr,		sccstart,
	  sccstop,		sccselect,	        sccputc,
	  sccprobe,		sccgetc,		scc_cons_init,
	  sccparam,		sccmmap,		scc_tty,
        },
#endif /* DEC3000_500 */

#ifdef DEC3000_300
        {       /* Alpha Pelican SCC console; from DS5000 */
	  DEC_3000_300,		sccopen,		sccclose,
	  sccread,	  	sccwrite,		sccioctl,
	  sccintr,		sccintr,		sccstart,
	  sccstop,		sccselect,	        sccputc,
	  sccprobe,		sccgetc,		scc_cons_init,
	  sccparam,		sccmmap,		scc_tty,
        },
#endif /* DEC3000_500 */

#ifdef DS5000_300 
	{	/* 3MAX+ and BIGMAX */
	  DS_5000_300,		sccopen,		sccclose,
	  sccread,	  	sccwrite,		sccioctl,
	  sccintr,		sccintr,		sccstart,
	  sccstop,		sccselect,	        sccputc,
	  sccprobe,		sccgetc,		scc_cons_init,
	  sccparam,		sccmmap,		scc_tty,
	},
#endif /* DS5000_300 */


	{	/* always need 0 to be last entry */
	  0,			0,			0,
	  0,			0,			0,
	  0,			0,			0,
	  0,			0,			0,
	  0,			0,			0,
	  0,			0,			0,
        }
};	


/*
 * If the ws driver is not defined we do not want a generic console 
 * driver 
 */
#ifdef __alpha
#include "ws.h"
#if NWS == 0
int install_generic_console() {return(0);}
#endif /* NWS == 0 */
#endif /* __alpha */

#if defined(mips) || defined(DEC3000_500) || defined(DEC3000_300) || defined(DEC2000_300)

#include "fb.h"
#if NFB > 0
int fb_cons_init();
#endif

#include "px.h"
#if NPX > 0
int px_cons_init();
#endif

#include "pv.h"
#if NPV > 0
int pv_cons_init();
#endif

#include "vga.h"
#if NVGA > 0
int vga_cons_init();
#endif

struct vcons_init_sw vcons_init[] = {

#if NFB > 0
#ifndef __alpha
    {	"PMAG-BA ", 	fb_cons_init	},	/* CFB */
    {	"PMAG-AA ",	fb_cons_init	},	/* MFB */
#endif /* !__alpha */
    {   "PMAGB-BA",     fb_cons_init	},	/* SFB */
    {	"PMAG-RO ",	fb_cons_init	},	/* RO*/
    {	"PMAG-JA ",	fb_cons_init	},	/* RO*/
    {	"PMAGD   ",	fb_cons_init	},	/* SFBP-8,-32,-32Z */
    {	"PMAGD-AA",	fb_cons_init	},	/* SFBP-8 */
    {	"PMAGD-BA",	fb_cons_init	},	/* SFBP-32 */
#endif

#if NPX > 0 
    {	"PMAG-CA ",	px_cons_init	},	/* 2DA */
    {	"PMAG-DA ",	px_cons_init	},	/* LM-3DA */
    {	"PMAG-FA ",	px_cons_init	},	/* HE-3DA */
    {	"PMAG-FB ",	px_cons_init	},	/* HE+3DA */
    {	"PMAGB-FA",	px_cons_init	},	/* HE+3DA */
    {	"PMAGB-FB",	px_cons_init	},	/* HE+3DA */
#endif

#if NPV > 0
    {   "PMAGC   ",	pv_cons_init	},	/* Low PV */
    {   "PMAGC-AA",	pv_cons_init	},	/* Low PV */
    {   "PMAGC-BA",	pv_cons_init	},	/* Mid PV */
#endif

#if NVGA > 0
    {   "VGA--VGA",	vga_cons_init	},	/* Generic VGA */
#endif

    {	"", 		0 }			/* we must have a 0 entry
						   to end the loop */
    };
#endif /* mips || DEC3000_500 || DEC3000_300 || DEC2000_300 */
