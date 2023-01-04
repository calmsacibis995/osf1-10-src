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

/***********************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	The Xie dispatch tables.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      April 20, 1990
**
************************************************************************/

/*
**  Include files
*/
#include <stdio.h>
    /*
    **  Core X Includes
    */
#include <X.h>
#include <Xproto.h>
    /*
    **  XIE Includes
    */
#include <XieAppl.h>
#include <XieDdx.h>
#include <XieProto.h>

/*
**  Equated Symbols
*/
#ifndef externaldef
#ifdef VMS
#define externaldef(psect) globaldef {"psect"} noshare
#else
#define externaldef(psect)
#endif
#endif

    /*
    **	DIX protocol functions
    */
extern int ProcNotRequest();
extern int ProcInitSession();
extern int ProcTermSession();
extern int ProcQueryEvents();
extern int ProcSelectEvents();
extern int ProcSetOpDefaults();
extern int ProcQueryOpDefaults();
extern int ProcClonePhoto();
extern int ProcCreateResource();
extern int ProcDeleteResource();
extern int ProcQueryResource();
extern int ProcBindMapToFlo();
extern int ProcAbortFlo();
extern int ProcExecuteFlo();
extern int ProcTapFlo();
extern int ProcAbortTransport();
extern int ProcGetStream();
extern int ProcGetTile();
extern int ProcPutStream();
extern int ProcPutTile();
extern int ProcSetTransport();
extern int ProcExport();
extern int ProcFreeExport();
extern int ProcImport();
extern int ProcQueryExport();
extern int ProcArea();
extern int ProcAreaStats();
extern int ProcArith();
extern int ProcCalcHistogram();
extern int ProcChromeCom();
extern int ProcChromeSep();
extern int ProcCompare();
extern int ProcConstrain();
extern int ProcCrop();
extern int ProcDither();
extern int ProcFill();
extern int ProcLogical();
extern int ProcLuminance();
extern int ProcMatchHistogram();
extern int ProcMath();
extern int ProcMirror();
extern int ProcPoint();
extern int ProcPointStats();
extern int ProcRotate();
extern int ProcScale();
extern int ProcTranslate();
extern int ProcNotImplemented();

    /*
    **	DIX protocol function dispatch table
    */
externaldef(xieDixProc) int (*xieDixProc[])() = {
        ProcNotRequest,			/*  0 */
        ProcInitSession,		/*  1 */
	ProcTermSession,		/*  2 */
	ProcQueryEvents,		/*  3 */
	ProcSelectEvents,		/*  4 */
	ProcSetOpDefaults,              /*  5 */
	ProcQueryOpDefaults,            /*  6 */
        ProcClonePhoto,			/*  7 */
        ProcCreateResource,		/*  8 */
	ProcDeleteResource,		/*  9 */
	ProcQueryResource,		/* 10 */
	ProcBindMapToFlo,		/* 11 */
	ProcAbortFlo,			/* 12 */
	ProcExecuteFlo,			/* 13 */
	ProcTapFlo,			/* 14 */
        ProcAbortTransport,		/* 15 */
	ProcGetStream,			/* 16 */
	ProcGetTile,			/* 17 */
	ProcPutStream,			/* 18 */
	ProcPutTile,			/* 19 */
        ProcSetTransport,		/* 20 */
        ProcExport,			/* 21 */
	ProcFreeExport,			/* 22 */
	ProcImport,			/* 23 */
	ProcQueryExport,		/* 24 */
	ProcArea,	                /* 25 */
	ProcAreaStats,			/* 26 */
	ProcArith,	                /* 27 */
	ProcCalcHistogram,		/* 28 */
	ProcChromeCom,		        /* 29 */
	ProcChromeSep,			/* 30 */
	ProcCompare,                    /* 31 */
	ProcConstrain,                  /* 32 */
	ProcCrop,			/* 33 */
	ProcDither,			/* 34 */
	ProcFill,			/* 35 */
	ProcLogical,			/* 36 */
	ProcLuminance,			/* 37 */
	ProcMatchHistogram,             /* 38 */
	ProcMath,		        /* 39 */
	ProcMirror,			/* 40 */
	ProcPoint,			/* 41 */
	ProcPointStats,			/* 42 */
	ProcRotate,			/* 43 */
	ProcScale,			/* 44 */
	ProcTranslate			/* 45 */
	};

    /*
    **	DIX swap protocol functions
    */
int SProcXieSimpleReq();
int SProcXieResourceReq();

int SProcInitSession();
int SProcQueryEvents();
int SProcSelectEvents();
int SProcSetOpDefaults();
int SProcCreateResourceByRef();
int SProcCreateResourceByValue();
int SProcBindMapToFlo();
int SProcAbortFlo();
int SProcExecuteFlo();
int SProcTapFlo();
int SProcAbortTransport();
int SProcGetStream();
int SProcGetTile();
int SProcPutStream();
int SProcPutTile();
int SProcSetTransport();
int SProcExport();
int SProcFreeExport();
int SProcImport();
int SProcQueryExport();
int SProcArea();
int SProcAreaStats();
int SProcArith();
int SProcCalcHistogram();
int SProcChromeCom();
int SProcChromeSep();
int SProcCompare();
int SProcConstrain();
int SProcCrop();
int SProcDither();
int SProcFill();
int SProcLogical();
int SProcLuminance();
int SProcMatchHistogram();
int SProcMath();
int SProcMirror();
int SProcPoint();
int SProcPointStats();
int SProcRotate();
int SProcScale();
int SProcTranslate();

    /*
    **	DIX swap protocol function procedure dispatch table
    */
externaldef(xieDixProc) int (*xieDixSwap[])() = {
        ProcNotRequest,			/*  0 */
        SProcInitSession,		/*  1 */
	SProcXieSimpleReq,		/*  2 */
	SProcQueryEvents,		/*  3 */
	SProcSelectEvents,		/*  4 */
	SProcSetOpDefaults,             /*  5 */
	SProcXieSimpleReq,              /*  6 */
        SProcCreateResourceByRef,	/*  7 */
        SProcCreateResourceByValue,	/*  8 */
	SProcXieResourceReq,		/*  9 */
	SProcXieResourceReq,		/* 10 */
	SProcBindMapToFlo,		/* 11 */
	SProcAbortFlo,			/* 12 */
	SProcExecuteFlo,		/* 13 */
	SProcTapFlo,			/* 14 */
        SProcAbortTransport,		/* 15 */
	SProcGetStream,			/* 16 */
	SProcGetTile,			/* 17 */
	SProcPutStream,			/* 18 */
	SProcPutTile,			/* 19 */
        SProcSetTransport,		/* 20 */
        SProcExport,			/* 21 */
	SProcFreeExport,		/* 22 */
	SProcImport,			/* 23 */
	SProcQueryExport,		/* 24 */
	SProcArea,	                /* 25 */
	SProcAreaStats,			/* 26 */
	SProcArith,	                /* 27 */
	SProcCalcHistogram,		/* 28 */
	SProcChromeCom,		        /* 29 */
	SProcChromeSep,			/* 30 */
	SProcCompare,                   /* 31 */
	SProcConstrain,                 /* 32 */
	SProcCrop,			/* 33 */
	SProcDither,			/* 34 */
	SProcFill,			/* 35 */
	SProcLogical,			/* 36 */
	SProcLuminance,			/* 37 */
	SProcMatchHistogram,            /* 38 */
	SProcMath,		        /* 39 */
	SProcMirror,			/* 40 */
	SProcPoint,			/* 41 */
	SProcPointStats,		/* 42 */
	SProcRotate,			/* 43 */
	SProcScale,			/* 44 */
	SProcTranslate			/* 45 */
	};

extern int SReplySimple();
extern int SReplyInitSession();
extern int SReplyQueryEvents();
extern int SReplyQueryOpDefaults();
extern int SReplyQueryResource();
extern int SReplyGetStream();
extern int SReplyQueryExport();

    /*
    **	DIX swap protocol reply procedure dispatch table
    */
externaldef(xieDixProc) int (*xieDixSwapReply[])() = {
        ProcNotRequest,			/*  0 */
        SReplyInitSession,		/*  1 */
	ProcNotRequest,			/*  2 */
	SReplyQueryEvents,		/*  3 */
	ProcNotRequest,		        /*  4 */
	ProcNotRequest,                 /*  5 */
	SReplyQueryOpDefaults,          /*  6 */
        ProcNotRequest,			/*  7 */
        ProcNotRequest,			/*  8 */
	ProcNotRequest,			/*  9 */
	SReplyQueryResource,		/* 10 */
	ProcNotRequest,			/* 11 */
	ProcNotRequest,			/* 12 */
	ProcNotRequest,			/* 13 */
	ProcNotRequest,			/* 14 */
        ProcNotRequest,			/* 15 */
	SReplyGetStream,		/* 16 */
	SReplySimple,			/* 17 */
	ProcNotRequest,			/* 18 */
	ProcNotRequest,			/* 19 */
        ProcNotRequest,			/* 20 */
        ProcNotRequest,			/* 21 */
	ProcNotRequest,			/* 22 */
	ProcNotRequest,			/* 23 */
	SReplyQueryExport,		/* 24 */
	ProcNotRequest,	                /* 25 */
	ProcNotRequest,			/* 26 */
 	ProcNotRequest,	                /* 27 */
	SReplySimple,			/* 28 */
	ProcNotRequest,		        /* 29 */
	ProcNotRequest,			/* 30 */
	ProcNotRequest,	                /* 31 */
	ProcNotRequest,                 /* 32 */
	ProcNotRequest,			/* 33 */
	ProcNotRequest,			/* 34 */
	ProcNotRequest,			/* 35 */
	ProcNotRequest,			/* 36 */
	ProcNotRequest,			/* 37 */
	ProcNotRequest,	                /* 38 */
	ProcNotRequest,		        /* 39 */
	ProcNotRequest,			/* 40 */
	ProcNotRequest,			/* 41 */
	SReplySimple,			/* 42 */
	ProcNotRequest,			/* 43 */
	ProcNotRequest,			/* 44 */
	ProcNotRequest			/* 45 */
    };

extern void WriteToClient();
extern void Swap16Write();
extern void Swap32Write();

/*
 * Routines to swap arbitrary chunks of data that are part of replies.
 * Index is size of data unit in bytes.
 */

externaldef(xieDixProc) void (*xieDixSwapData[])() = {
        NULL,
	WriteToClient,		/* No swap needed */
	Swap16Write,
	NULL,
	Swap32Write
    };
/* end module XieTables.c */
