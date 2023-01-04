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
/* BuildSystemHeader added automatically */
/* $Header: /usr/sde/x11/rcs/x11/src/./motif/clients/dxdiff/parsediff.h,v 1.1 90/01/01 00:00:00 devrcs Exp $ */
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *
 *	dxdiff
 *
 *	parsediff.h - parser header file for dxdiff
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 3rd March 1988
 *
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	31st March 1988		Laurence P. G. Cable
 *
 *	Ive changed the definition of NumberSequence to remove the dynamic
 *	allocation of 1 &  2 number sequences .. this makes some of the
 *	calculations involved in displaying differences less complex
 *	NumberSequences of 1 number have the 2nd one == the 1st
 *
 *	14th April 1988		Laurence P.  G. Cable
 *
 *	changed name of NumberSequence structure element from 'howmany'
 *	to 'oneortwo' to remove macro name clash with V2.4 sys/types.h
 *	macro.
 *
 *	21st April 1988		Laurence P. G. Cable
 *
 *	added extern for discard_diffs flag, a serial number and a flag
 *	field.
 *
 *	25th April 1988		Laurence P. G. Cable
 *
 *	06 Aug 1990		Colin Prosser
 *
 *	Fix storage allocation bugs and portability problems.
 *	Cures seg fault reported in UWS QAR 02624.
 */

#ifndef	PARSEDIFF.H
#define	PARSEDIFF.H

#ifndef	FILESTUFF.H
CAUSE COMPILER ERROR BECAUSE filestuff.h WAS NOT INCLUDED
#endif  FILESTUFF.H

#if	DEBUG || LEXDEBUG
extern	int	lexdebug;
#endif

#if	DEBUG || YACCDEBUG
extern	int	yaccdebug;
#endif

#ifdef	DEBUG
extern	int	debug;
#endif

extern	int discard_diffs;	/* throw away diff context lines ? */

#define	YYMAXDEPTH	(20 * 1024) /* size of yacc value stack !!! */

/*
 * union members for YYSTYPE
 */
		
typedef	union	_tokenval { /* lex interface */
				char *string;
				int  number;
				char ch;
			  } TokenVal;


typedef	enum _editypenum { 
				EAppend = YY_APPEND_CMD,
				EDelete = YY_DELETE_CMD,
				EChange = YY_CHANGE_CMD
		      } EditTypEnum;
 
		
typedef	enum _nodetypenum { 
			BFNotification,
			CFNotification,
			IFNotification,
			OFNotification,
			DCNotification,
			DENotification,
			EditLine,
			Difference,
		  }	NodeTypEnum;

#define	IsBFNotification(p)	((p) == BFNotification)
#define	IsCFNotification(p)	((p) == CFNotification)
#define	IsIFNotification(p)	((p) == IFNotification)

#define	IsOFNotification(p)	((p) == OFNotification)

#define	IsDCNotification(p)	((p) == DCNotification)
#define	IsDENotification(p)	((p) == DENotification)

#define	IsEditline(p)	((p) == Editline)


typedef	struct	_bfn	Bfn, *BfnPtr; /* Binary files notification */
typedef	struct	_cfn	Cfn, *CfnPtr; /* Common files notification */
typedef	struct	_ifn	Ifn, *IfnPtr; /* Identical files notification */
typedef	struct	_ofn	Ofn, *OfnPtr; /* Only in one notification */
typedef	struct	_dcn	Dcn, *DcnPtr; /* Diff cmd notification */
typedef	struct	_den	Den, *DenPtr; /* diff error notification */

typedef	struct	_edc	Edc, *EdcPtr; /* edit context */
typedef	struct	_dl	Dfl,  *DflPtr; /* diff line */


typedef	union	_nodes	*NodePtr, **PtrToNodePtr;


typedef	struct	_nodecommon	{
		NodePtr		next,
				prev;
		NodeTypEnum	type;
		unsigned int	serial; /* new - serial number */
		unsigned char	flags;	/* new - user flags ?? */
} NodeCommon, *NodeCommonPtr,**PtrToNodeCommonPtr;

/* defines for flags */

#define	CANFREE		0x80	/* all nodes */

#define	HIGHRIGHT	0x01	/* Edc - right highlight done */
#define	HIGHLEFT	0x02	/* Edc - left highlight done */

/* some macros for flags */

#define	ZeroFlags(p)	(((NodeCommonPtr)(p))->flags = 0)

#define	SetCanFree(p)	(((NodeCommonPtr)(p))->flags |= CANFREE)
#define	OkToFree(p)	((((NodeCommonPtr)(p))->flags & CANFREE) == CANFREE)

#define	HighLightLeft(p)	(((NodeCommonPtr)(p))->flags |= HIGHLEFT)
#define	HighLightRight(p)	(((NodeCommonPtr)(p))->flags |= HIGHRIGHT)
#define	LeftHighLighted(p)	((((NodeCommonPtr)(p))->flags & HIGHLEFT) == HIGHLEFT)
#define	RightHighLighted(p)	((((NodeCommonPtr)(p))->flags & HIGHRIGHT) == HIGHRIGHT)




union	_nodes	{
	BfnPtr		bfnp;
	CfnPtr		cfnp;
	IfnPtr		ifnp;
	OfnPtr		ofnp;
	DcnPtr		dcnp;
	DenPtr		denp;
	EdcPtr		edcp;
	DflPtr		dflp;
	NodeCommonPtr	cp;
};

#ifndef	_XtIntrinsic_h
typedef	char	*Pathname,
		*String;
#else	_XtIntrinsic_h
typedef	String	Pathname;
#endif	_XtIntrinsic_h

typedef	struct	_numseq	{
	unsigned char	oneortwo;   /* was howmany but clashed with V2.4 */
	unsigned int	numbers[2]; /* was 1 and alloc'd dynamically */
}	NumberSequence, *NumberSequencePtr;

	

struct	_bfn	{ /* binary file notification */
	NodeCommon	common;
	BfnPtr		next,
			prev;
	Pathname	path1,
			path2;
};
		

struct	_cfn	{ /* common file notification */
	NodeCommon	common;
	CfnPtr		next,
			prev;
	Pathname	path1,
			path2;
};
		

struct	_ifn	{ /* identical file notification */
	NodeCommon	common;
	IfnPtr		next,
			prev;
	Pathname	path1,
			path2;
};
		

struct	_ofn	{ /* only file notification */
	NodeCommon	common;
	OfnPtr		next,
			prev;
	Pathname	path1,
			path2;
};
		

struct	_dcn	{ /* diff command notification */
	NodeCommon	common;
	DcnPtr		next,
			prev;
	Pathname	path1,
			path2;
#if	0
	String		*flags; /* a vector of flags - not implemented */
#endif
};
		

struct	_den	{ /* diff error notification */
	NodeCommon	common;
	DenPtr		next,
			prev;
	String		error;
};


struct	_edc	{ /* edit line cmd */
	NodeCommon		common;
	EdcPtr			next,
				prev;
	EditTypEnum		et;
	DflPtr			diffsin1,
				diffsin2;
	NumberSequencePtr	ns1,
				ns2;
	HighLightInfoPtr	highl,highr;	/* highlight info for text */
};

typedef	enum	_diffnodenum	{
				 EDiffin1NotIn2 = YY_DIFF_IN_1_AND_NOT_2,
				 EDiffin2NotIn1 = YY_DIFF_IN_2_AND_NOT_1 
				}	DiffNodEnum;
				
struct	_dl	{ /* difference line */
	NodeCommon	common;
	DflPtr		next,
			prev;
	DiffNodEnum	dt;
	String		difference;
};

		
typedef	union	{
	TokenVal		token;
	EditTypEnum		et;
	NumberSequencePtr	ns;
}	YYSTYPE;


typedef	struct	_backendcache	*BackEndCachePtr;

typedef	struct	_dlb	{
	BfnPtr		bfnhead,
			bfntail;
	CfnPtr		cfnhead,
			cfntail;
	IfnPtr		ifnhead,
			ifntail;
	OfnPtr		ofnhead,
			ofntail;
	DcnPtr		dcnhead,
			dcntail;
	DenPtr		denhead,
			dentail;
	EdcPtr		edchead,
			edctail;
	DflPtr		dflhead,
			dfltail;
	NodeCommonPtr	nphead,
			nptail;
 
	/* stats */

	unsigned int    bfncnt;
	unsigned int    cfncnt;
	unsigned int    ifncnt;
	unsigned int    ofncnt;
	unsigned int    dcncnt;
	unsigned int    dencnt;
	unsigned int    edccnt;
	unsigned int    df1cnt;
	unsigned int    df2cnt;
	unsigned int    nccnt;

	BackEndCachePtr	caches;
}	DiffListBlk, *DiffListBlkPtr;

#endif	PARSEDIFF.H /* do not include anything after this line */
