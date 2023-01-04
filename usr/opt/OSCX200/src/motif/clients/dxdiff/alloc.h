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
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxdiff/alloc.h,v 1.1.2.2 92/08/03 09:47:18 Dave_Hill Exp $ */


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
 *	alloc.h - header file for allocation of dynamic structures .. etc
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 13th March 1988
 *
 *
 *	Description
 *	-----------
 *
 *	Contains macro's etc for the convenient creation of dynamic 
 *	data structures etc ......
 *	
 *	Note	parsediff.h must also be included in the target source before
 *		using this file !!!!
 *
 *	Modification History
 *	------------ -------
 *	
 *	31st March 1988		Laurence P.  G. Cable
 *
 *	Modified AllocNumberSequenceOf{1|2} macros in line with changes
 *	(i.e removal of dynamic allocation of the sequences) to NumberSequence
 *	see parsediff.h
 *
 *	14th April 1988		Laurence P.  G. Cable
 *
 *	changed name of NumberSequence structure element from 'howmany'
 *	to 'oneortwo' to remove macro name clash with V2.4 sys/types.h
 *	macro.
 *
 *	21st April 1988		Laurence P. G. Cable
 *
 *	Modify Allocs to support new caching performance stuff
 *
 *	25th April 1988		Laurence P. G. Cable
 *
 *	Added FreeNode macro and used some macros in parsediff.h to test/set
 *	flag fields
 *
 *	30th June  1988		Laurence P. G. Cable
 *
 *	removed XtMalloc/XtFree defines .... lets try the real ones now!
 *
 *	06 Aug 1990		Colin Prosser
 *
 *	Fix storage allocation bugs and portability problems.
 *	Cures seg fault reported in UWS QAR 02624.
 *
 *	Fix text widget highlight initialization bug.  Fixes UWS QAR 2717.
 */

#ifndef	PARSEDIFF.H
CAUSE COMPILER ERROR BECAUSE parsediff.h WAS NOT INCLUDED
#endif	PARSEDIFF.H

#ifndef	ALLOC.H
#define	ALLOC.H

/*****#define	XtMalloc	malloc	/***** remove this later *****/
/*****#define	XtFree		free	/***** remove this later *****/


/* new caching for dynamic memory */

typedef	struct	_storecache	{
	NodePtr		cache,
			prev;
	unsigned int	numincache,
			numalloc;
}	StoreCache ,*StoreCachePtr;

#define	DefineStoreCache(name, num)					\
static	StoreCache name = { (NodePtr)NULL, (NodePtr)NULL,(num), (num) }
				
#define	ResetStoreCache(name)	(name)->cache = (name)->prev = (NodePtr)NULL; (name)->numalloc = (name)->numincache
				
#define	InitializeStoreCache(name, size)	\
		(name)->cache = (name)->prev = (NodePtr)NULL; (name)->numalloc = (name)->numincache = (size)

#define	CacheBaseAddress(p)	((NodePtr *)(p) - 1)	/* the previous word */
#define	PreviousCacheAddress(p)	*CacheBaseAddress(p)	/* the previous word */

typedef	struct	_backendcache	{
	int		cachesempty;

	StoreCache	nssc;
	StoreCache	edcsc;
	StoreCache	dflsc;
	StoreCache	bfnsc;
	StoreCache	cfnsc;
	StoreCache	ofnsc;
	StoreCache	ifnsc;
	StoreCache	densc;
	StoreCache	dcnsc;
}	BackEndCache;

#define	_freestorecache(name)						\
		{							\
			register	NodePtr	this, destroy;		\
									\
			this = (name)->prev;				\
			while ( this != (NodePtr)NULL) {		\
				destroy = (NodePtr)CacheBaseAddress(this);	\
				this = (NodePtr)PreviousCacheAddress(this);	\
				XtFree((char *)destroy);		\
			}						\
		}							\
		ResetStoreCache(name)

#define	_refreshandallocfromcache(sc, utype, utypeptr)			  					\
	((sc->cache = (NodePtr)XtMalloc(sc->numincache * sizeof(utype) + sizeof(NodePtr)))			\
	 == (NodePtr)NULL ? (NodePtr)NULL : (sc->numalloc = 1,		  					\
	 (*(NodePtr *)sc->cache = sc->prev),(sc->prev = sc->cache = (NodePtr)((NodePtr *)sc->cache + 1)),	\
	 ZeroFlags(sc->cache),SetCanFree(sc->cache),sc->cache))

#define	_allocfromcache(sc, utype, utypeptr)				  \
	(sc->numalloc++,						  \
	 (sc->cache = (NodePtr)((unsigned long)sc->cache + sizeof(utype))),  \
	 ZeroFlags(sc->cache), sc->cache)

#define	MallocFromCache(sc, utype, utypeptr)				\
		((sc->numalloc < sc->numincache) ?			\
		     (utypeptr)_allocfromcache(sc, utype, utypeptr) :	\
		     (utypeptr)_refreshandallocfromcache(sc, utype, utypeptr))	


extern	char *strcpy();

/********************************
 *
 *	DeepCopyString
 *
 ********************************/

#define	DeepCopyString(x)	strcpy(XtMalloc(strlen(x) + 1), (x))

/* some initialisation macro's for dynamic data structures */

/********************************
 *
 *	AllocError
 *
 ********************************/

#define	AllocError(macroname)						\
		fprintf(stderr,"%s: failed in file %s at line %d\n", 	\
				macroname, __FILE__, __LINE__)


/********************************
 *
 *	AllocNumberSequenceOf1	
 *
 ********************************/

#define	AllocNumberSequenceOf1(p, sc)					    \
	if (((p) = MallocFromCache(sc, NumberSequence, NumberSequencePtr))  \
	     == (NumberSequencePtr)NULL) {				    \
		AllocError("AllocNumberSequenceOf1");			    \
	} else {							    \
		(p)->oneortwo = (unsigned char)1;			    \
		(p)->numbers[0] = (p)->numbers[1] = 0;			    \
	}
	

/********************************
 *
 *	AllocNumberSequenceOf2	
 *
 ********************************/

#define	AllocNumberSequenceOf2(p, sc)					   \
	if (((p) = MallocFromCache(sc, NumberSequence, NumberSequencePtr)) \
	     ==	(NumberSequencePtr)NULL) {				   \
		AllocError("AllocNumberSequenceOf2");		 	   \
	} else {							   \
		(p)->oneortwo = (unsigned char)2;			   \
		(p)->numbers[0] = (p)->numbers[1] = 0;			   \
	}
	

/********************************
 *
 *	AllocEdc
 *
 ********************************/

#define	AllocEdc(p, v1, sc) if (((p) = MallocFromCache(sc, Edc, EdcPtr))\
				== (EdcPtr)NULL) {			\
				AllocError("AllocEdc");			\
			     } else {					\
				(p)->common.next = (p)->common.prev =	\
					(NodePtr)NULL;			\
				(p)->common.type = EditLine;		\
				(p)->next = (p)->prev = (EdcPtr)NULL;	\
				(p)->diffsin1 = (p)->diffsin2 = 	\
					(DflPtr) NULL;			\
				 (p)->ns1 = (p)->ns2 = 			\
					(NumberSequencePtr)NULL;	\
				(p)->et = (EditTypEnum)(v1);		\
				(p)->highl = (p)->highr = 		\
					(HighLightInfoPtr)NULL;		\
			     }


/********************************
 *
 *	AllocDfl
 *
 ********************************/

#define	AllocDfl(p, v1, sc) if (((p) = MallocFromCache(sc, Dfl, DflPtr))\
				== (DflPtr)NULL) {			\
				AllocError("AllocDfl");			\
			    } else {					\
				(p)->common.type = Difference;		\
				(p)->next = (p)->prev = (DflPtr)NULL;	\
				(p)->dt = (DiffNodEnum)(v1);		\
				(p)->difference = (String)NULL;		\
			    }


/********************************
 *
 *	AllocBfn
 *
 ********************************/

#define	AllocBfn(p, sc)	if (((p) = MallocFromCache(sc, Bfn, BfnPtr)) ==	\
			    (BfnPtr)NULL) {				\
				AllocError("AllocBfn");			\
			} else {					\
				(p)->common.type = BFNotification;	\
				(p)->next = (p)->prev = (BfnPtr)NULL;	\
				(p)->path1 = (p)->path2 = (Pathname)NULL; \
			}


/********************************
 *
 *	AllocIfn
 *
 ********************************/

#define	AllocIfn(p,sc) if (((p) = MallocFromCache(sc, Ifn, IfnPtr)) ==	\
			    (IfnPtr)NULL) {				\
				AllocError("AllocIfn");			\
			} else {					\
				(p)->common.type = IFNotification;	\
				(p)->next = (p)->prev = (IfnPtr)NULL;	\
				(p)->path1 = (p)->path2 = (Pathname)NULL; \
			}


/********************************
 *
 *	AllocOfn
 *
 ********************************/

#define	AllocOfn(p, sc) if (((p) = MallocFromCache(sc, Ofn, OfnPtr)) ==	\
			    (OfnPtr)NULL) {				\
				AllocError("AllocOfn");			\
			} else {					\
				(p)->common.type = OFNotification;	\
				(p)->next = (p)->prev = (OfnPtr)NULL;	\
				(p)->path1 = (p)->path2 = (Pathname)NULL; \
			}


/********************************
 *
 *	AllocCfn
 *
 ********************************/

#define	AllocCfn(p, sc)	if (((p) = MallocFromCache(sc, Cfn, CfnPtr)) ==	\
			    (CfnPtr)NULL) {				\
				AllocError("AllocCfn");			\
			} else {					\
				(p)->common.type = CFNotification;	\
				(p)->next = (p)->prev = (CfnPtr)NULL;	\
				(p)->path1 = (p)->path2 = (Pathname)NULL; \
			}


/********************************
 *
 *	AllocDen
 *
 ********************************/

#define	AllocDen(p, sc) if (((p) = MallocFromCache(sc, Den, DenPtr)) ==	\
			    (DenPtr)NULL) {				\
				AllocError("AllocDen");			\
			} else {					\
				(p)->common.type = DENotification;	\
				(p)->next = (p)->prev = (DenPtr)NULL;	\
				(p)->error = (String)NULL;		\
			}


/********************************
 *
 *	AllocDcn
 *
 ********************************/

#define	AllocDcn(p, sc)	if (((p) = MallocFromCache(sc, Dcn, DcnPtr)) ==	\
			    (DcnPtr)NULL) {				\
				AllocError("AllocDcn");			\
			} else {					\
				(p)->common.type = DENotification;	\
				(p)->next = (p)->prev = (DcnPtr)NULL;	\
				(p)->path1 = (p)->path2 = (String)NULL;	\
			}



/********************************
 *
 *	FreeNode
 *
 ********************************/

#define	FreeNode(p)	if (OkToFree(p)) XtFree((char *)(p))

#endif	ALLOC.H	/* do not include anything after this line */

