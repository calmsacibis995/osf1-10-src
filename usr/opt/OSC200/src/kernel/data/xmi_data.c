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
static char     *sccsid = "@(#)xmi_data.c	9.2  (ULTRIX/OSF)    10/23/91";
#endif lint
/* ------------------------------------------------------------------------
 * Modification History:
 *
 *   03-Aug-90		rafiey (Ali Rafieymehr)
 *	Added XJA (VAX9000) entry. Also removed unnecessary
 *	code and comments for allocation of xminode which was taking a 
 *	lot of kernel BSS space (specially for multiple XMIs).
 *
 *   06-Jun-1990	Pete Keilty
 *	Added preliminary support for CIKMF.
 *
 *   05-Jun-1990	Joe Szczypek
 *	Removed XMA2.  Entry for XMA will handle XMA2 (both have
 *	same device id).
 *
 *   01-May-1990        Joe Szczypek
 *      Included XMP, XBI+, and XMA2.
 *
 *   08-Dec-1989	Fix support for rigel
 *
 *   18-Aug-1989	Pete Keilty
 *	Change CIXCB to CIXCD.
 *
 *   20-Jul-89		rafiey (Ali Rafieymehr)
 *	Included XNA, and KDM in xmisw[].
 *
 *   14-Mar-1989	Mark A. Parenti
 *	Add support for HSM.
 *
 *   16-Jun-1989	Darrell A. Dunnuck
 *	Removed cpup as an arg passed to functions.
 *
 *   05-Aug-1988	Todd M. Katz
 *	Refer to function xminotconf() instead of to function binotconf()
 *	when no CIs are configured.
 *
 *   05-May-1988	Todd M. Katz
 *	Add support for the CIXCB XMI to CI communications port.
 *
 * ------------------------------------------------------------------------
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <machine/cpu.h>
#ifdef JAA_TODO
#include <io/dec/bi/bireg.h>
#include <io/dec/bi/buareg.h>
#endif /*** JAA_TODO ***/
#include <io/dec/xmi/xmireg.h>
#include <io/dec/uba/ubavar.h>
#include "vaxbi.h"
#include "xmi.h"

int
noxmireset()
{
	/* no bi reset routine */
	return(0);
}

int
xminotconf(nxv, nxp, xminumber, xminode, xmidata) 
	struct xmi_nodespace *nxv;
	struct xmi_nodespace *nxp;
	int 	xminumber;
	int 	xminode;
	struct xmidata *xmidata;
{
	struct xmisw *pxmisw;	
	
	pxmisw = xmidata->xmierr[xminode].pxmisw;
	
	printf("%s at xmi%x node %d option not configured!\n",
	       pxmisw->xmi_name,xminumber,xminode);
	return(0);
}

#ifdef	DS5800
int x3p_init();
int xmainit();
#else	DS5800
#define x3p_init xminotconf
#define xmainit xminotconf
#endif	DS5800

#if NVAXBI > 0
int xbiinit();
#else
#define xbiinit xminotconf
#endif

#include "ci.h"
#if NCI > 0
int xminpinit();
#else
#define xminpinit xminotconf
#endif

#include "xna.h"
#if NXNA > 0
int xnaprobe(), xnaattach();
#else
#define xnaprobe xminotconf
#endif

#include "mfa.h"
#if NMFA > 0
int mfaprobe();
#else
#define mfaprobe xminotconf
#endif

#include "xza.h"
#if NXZA > 0
int kzmsaprobe();
int kfmsbprobe();
#else
#define kzmsaprobe xminotconf
#define kfmsbprobe xminotconf
#endif

#ifdef DEC7000
int xlambinit();
#else
#define xlambinit xminotconf
#endif /* DEC7000 */

int nNXMI = NXMI;

int (*x3p_probes[])() = { x3p_init, 0};
int (*xmaprobes[])() = {  xmainit, 0};
int (*xbiprobes[])() = {  xbiinit, 0};
/*int (*xcixmiprobes[])() = {  xmiciinit,0};*/
/*** we get to kdminit via the ctlr->port ***/
int (*xkdmprobes[])() = {  0};
int (*xnpxmiprobes[])() = {  xminpinit,0};
int (*xnaxmiprobes[])() = {  xnaprobe, 0};
int (*kzmsaxmiprobes[])() = {  kzmsaprobe, 0};
int (*kfmsbxmiprobes[])() = {  kfmsbprobe, 0};
int (*mfaxmiprobes[])() = {  mfaprobe, 0};
int (*xlambprobes[])() = {  xlambinit, 0};

struct xmisw xmisw [] =
{
/*	{ XMI_CIKMF, 	"cikmf",	xcixmiprobes ,	noxmireset,	
	  XMIF_ADAPTER}, */
	{ XMI_CIMNA, 	"cimna",	xnpxmiprobes ,	noxmireset,	
	  XMIF_ADAPTER|XMIF_SST},
	{ XMI_X3P,	"x3p",		x3p_probes,	noxmireset,
	  XMIF_NOCONF},
	{ XMI_XMA, 	"xma",		xmaprobes ,	noxmireset,	
	  XMIF_NOCONF},
	{ XMI_XBI, 	"xbi",		xbiprobes ,	noxmireset,	
	  XMIF_ADAPTER},
	{ XMI_XBIPLUS, 	"xbi+",		xbiprobes ,	noxmireset,	
	  XMIF_ADAPTER},
/*	{ XMI_CIXCD, 	"cixcd",	xcixmiprobes ,	noxmireset,	
	  XMIF_ADAPTER|XMIF_SST}, */
	{ XMI_KDM, 	"uq",		xkdmprobes ,	noxmireset,	
	  XMIF_CONTROLLER|XMIF_SST},
	{ XMI_KFMSB,    "KFMSB",	kfmsbxmiprobes , noxmireset,	
	  XMIF_ADAPTER|XMIF_SST},
	{ XMI_KZMSA, 	"KZMSA",	kzmsaxmiprobes , noxmireset,	
	  XMIF_ADAPTER|XMIF_SST},
	{ XMI_XNA, 	"xna",		xnaxmiprobes ,	noxmireset,	
	  XMIF_CONTROLLER|XMIF_SST},
	{ XMI_XFA, 	"mfa",	        mfaxmiprobes ,	noxmireset,	
	  XMIF_CONTROLLER|XMIF_SST},
	{ XMI_LAMB, 	"lamb",		xlambprobes ,	noxmireset,	
	  XMIF_ADAPTER},

	{ 0 }
};

int nxmitypes = sizeof (xmisw) / sizeof (xmisw[0]);

struct xmidata *head_xmidata;
