
/***************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
***************************************************************************/

#ifndef IDSPIPE_H
#define IDSPIPE_H

#ifndef NULL
#define NULL 0
#endif

/*
**  Rendering pipe -- tokens.
*/
#define IdsVoid	    (unsigned long *)0	/* discard result		*/
#define IdsTemp	    (unsigned long *)1	/* temporary result		*/
#define IdsByVal    (unsigned long *)3	/* next arg is value to pass	*/
#define IdsNoPtr    (unsigned long *)4	/* arg pointer is Null		*/

typedef unsigned long	(*IdsPipeFunction) ();
typedef char		(*IdsPipeCallProc) ();

/*
**  Pipeline Element -- function call structure.
*/
#define	PEflink_(pe)	((pe)->flink)
#define	PEcall_(pe)	((pe)->callahead)
#define	PEcid_(pe)	((pe)->call_id)
#define	PEdata_(pe)	((pe)->call_data)
#define	PEdst_(pe)	((pe)->dst)
#define	PEfunc_(pe)	((pe)->function)
#define	PEargs_(pe)	((pe)->arglst)
typedef struct _PipeElement
    {
    struct _PipeElement	 *flink;	/* link to next pipe record	    */
    IdsPipeCallProc	  callahead;	/* call-ahead function pointer	    */
    unsigned long	  call_id;	/* call-ahead id (call if non-zero) */
    char		 *call_data;	/* call-ahead function data	    */
    unsigned long	 *dst;		/* dst of function return	    */
    IdsPipeFunction	  function;	/* function -- called by LIB$CALLG  */
    unsigned long	 *arglst;	/* pointer to function's arg list   */
    } IdsPipeElementStruct, *IdsPipeElement;
/*
**  Pipeline Descriptor
*/
#define	PDpipe_(pd)	    ((pd)->pipe)
#define	PDbase_(pd)	    ((pd)->base)
#define	PDcall_(pd)	    ((pd)->base->callahead)
#define	PDcid_(pd)	    ((pd)->base->call_id)
#define	PDdata_(pd)	    ((pd)->base->call_data)
#define	PDdst_(pe)	    ((pe)->base->dst)
#define	PDargs_(pd)	    ((pd)->base->arglst)
#define	PDfid_(pd)	    ((pd)->fid)
#define	PDtmp_(pd)	    ((pd)->tmp)
#define	PDrfid_(pd)	    ((pd)->rfid)
#define PDcopiedROI_(pd)    ((pd)->copiedROI)

typedef struct _PipeDesc
    {
    IdsPipeElement	  pipe;		/* pointer to active pipe element   */
    IdsPipeElement	  base;		/* pointer to root of pipe	    */
    unsigned long	 *fid;		/* pointer to intermediate fid	    */
    unsigned long	 *tmp;		/* pointer to temporaries	    */
    unsigned long	 *rfid;		/* pointer to fid that has had all */
					/* render options applied */
    char                 copiedROI;     /* has ROI been copied?     */
    } IdsPipeDescStruct, *IdsPipeDesc;

#endif /* IDSPIPE_H */
