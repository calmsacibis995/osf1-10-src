
/***********************************************************************
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
******************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      Image Display Services (IDS)
**
**  ABSTRACT:
**
**      This module implements functions for building and executing an
**	IDS rendering pipe.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**
**  AUTHOR(S):
**
**      Robert NC Shelley
**
**  CREATION DATE:  February 29, 1988
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

    /*
    **  IDS include fils
    */
#include    <ids__pipe.h>    /* IDS pipe  definitions			    */
#include    <ids__macros.h>  /* IDS macro definitions			    */
#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif

/*
**  Table of contents
*/
    /*
    **  Private IDS -- low level entry points
    */
#ifdef NODAS_PROTO
IdsPipeDesc    IdsAllocatePipeDesc();	    /* Allocate a pipeline desc	    */
unsigned long *IdsInsertPipe();		    /* Insert an element into pipe  */
unsigned long  IdsExecutePipe();	    /* Execute pipe lined functions */
void	       IdsDeallocatePipeDesc();	    /* Deallocate a pipeline desc   */
unsigned long  IdsCallG();                  /* Emulate VMS lib$callg()      */
#endif

/*
**  MACRO definitions -- ( see also: IDS$$PIPE.H )
*/

/*
**  Equated Symbols
*/
#define PIPE_SIZE    64	/* default number of pipe elements to allocate	      */
#define PIPE_ARGS   512	/* default number of longwords for all pipe arg lists */
#define ELEM_ARGS    32	/* default length of single pipe element arg list     */

/*
**  External References
*/

/*
**  Local Storage
*/

/*******************************************************************************
**  IdsAllocatePipeDesc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**	none
**
**  FUNCTION VALUE:
**
**	address of initialized pipeline descriptor structure
**
*******************************************************************************/
IdsPipeDesc IdsAllocatePipeDesc()
{
    IdsPipeDesc pd = (IdsPipeDesc) _ImgCalloc(1,sizeof(IdsPipeDescStruct));

#ifdef TRACE
printf( "Entering Routine IdsAllocatePipeDesc in module IDS__PIPE \n");
#endif

    /*
    **	NOTE:
    **
    **	CALLOC of Pipe Descriptor implies that pipe, pipe_fid, and temp_fid all
    **	have initial value NULL.
    */
    PDbase_(pd) = (IdsPipeElement) 
                           _ImgCalloc(PIPE_SIZE, sizeof(IdsPipeElementStruct));
    PDargs_(pd) = (unsigned long *)
                           _ImgCalloc(PIPE_ARGS, sizeof(unsigned long));

#ifdef TRACE
printf( "Leaving Routine IdsAllocatePipeDesc in module IDS__PIPE \n");
#endif

    return( pd );
}

/*******************************************************************************
**  IdsInsertPipe
**
**  FUNCTIONAL DESCRIPTION:
**
**      Load a function call and its arguments into an IdsPipeElementStruct.
**	The new Element is inserted between the element pointed at by the pipe
**	descriptor and the one that follows it (NULL if end of pipe).
**
**  FORMAL PARAMETERS:
**
**	pd	  - pipe descriptor.
**	new	  - IdsRenderPipe segment to be inserted.
**	call	  - routine to be called prior to 'function' execution.
**	id	  - identifier, for use by 'user' routine.
**	data	  - data for use by 'user' routine.
**	dst	  - result pointer or token: IdsVoid or IdsTemp.
**	function  - address of function to be called.
**	args	  - pointer to agument list block which contains the argument
**		    count followed by pointers to the function's arguments.
**
**  FUNCTION VALUE:
**
**	dst	  - dst pointer of "new"  (IdsVoid if VOID function).
**
*******************************************************************************/
unsigned long  *IdsInsertPipe(pd, new, call, id, data, dst, function, args)
 IdsPipeDesc	  pd;
 IdsPipeElement	  new;
 IdsPipeCallProc  call;
 unsigned long	  id;
 char		 *data;
 unsigned long	 *dst;
 IdsPipeFunction  function;
 unsigned long	 *args;
{
#ifdef TRACE
printf( "Entering Routine IdsInsertPipe in module IDS__PIPE \n");
#endif

    PEflink_(new) = PEflink_(PDpipe_(pd));	    /* link to next segment */
    PEflink_(PDpipe_(pd)) = new;		    /* link to previous	    */
    PDpipe_(pd)	  = new;			    /* descriptor gets new  */
    PEcall_(new)  = call;			    /* call-ahead proc ptr  */
    PEcid_(new)   = id;				    /* call-ahead id	    */
    PEdata_(new)  = data;			    /* call ahead data	    */
    PEdst_(new)   = dst != IdsTemp ? dst	    /* Void or result ptr   */
		  : (unsigned long *) &PEdst_(new); /* Temporary result ptr */
    PEfunc_(new)  = function;			    /* function ptr	    */
    PEargs_(new)  = args;			    /* arg_list ptr	    */

#ifdef TRACE
printf( "Leaving Routine IdsInsertPipe in module IDS__PIPE \n");
#endif
    return( PEdst_(new) );  /* ptr to where function result will be stored  */
}

/*******************************************************************************
**  IdsExecutePipe
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine calls the rendering pipe element functions in FIFO order.
**
**  FORMAL PARAMETERS:
**
**	pd	- pointer to pipe descriptor containing rendering sequence.
**
**  FUNCTION VALUE:
**
**	result	- last non-IdsVoid pipe element function value returned.
**
*******************************************************************************/
unsigned long IdsExecutePipe( pd )
 IdsPipeDesc  pd;
{
    IdsPipeElement  pe = PDbase_(pd);
    long **from, *to, cnt, args[ELEM_ARGS];
    unsigned long result = 0;

#ifdef TRACE
printf( "Entering Routine IdsExecutePipe in module IDS__PIPE \n");
#endif
    for(PDpipe_(pd)=PEflink_(pe); PDpipe_(pd) != NULL; PDpipe_(pd)=PEflink_(pe))
	{
	pe= PDpipe_(pd);
	if( PEcall_(pe) != NULL && PEcid_(pe) != 0 &&
	  (*PEcall_(pe))(pd, PEcid_(pe), PEdata_(pe)) != 0) /* do callahead */
	    break; /* the callahead routine has requested the pipe to abort */

	from = (long **) PEargs_(pe);	/* pointer to count and element args*/
	to   = (long  *) &args[1];	/* pointer to lib$callg arg list    */

	for( args[0] = (long) *from++, cnt = args[0];  cnt-- > 0;  from++ )
	    /*
	    **  Build lib$callg argument list from pipe element argument list:
	    **  if arg is an IdsNoPtr token: stash a 0,
	    **	else, if NOT IdsByVal token: stash value pointed at by arg,
	    **	    it is an IdsByVal token: stash value of arg (if it exists).
	    */
	    *to++  = (*from == (long *) IdsNoPtr ? (long) 0 
                   :  *from != (long *) IdsByVal ? (long)  **from
		   :   cnt-- * --args[0] > 0    ? (long)(*++from) : (long) 0 );
	/*
	**  Call the function and store the result if/where requested.
	*/
#if ( defined (VMS) && !defined (ALPHA) )
	if(  PEdst_(pe) == IdsVoid )
	     lib$callg( args, PEfunc_(pe) );	    /* void result	    */
	else
	    {
	    result = lib$callg(args, PEfunc_(pe));  /* keep a result copy   */
	    *PEdst_(pe) = result;		    /* ... and store it	    */
	    }
#else /* use this for Ultrix, OSF, and ALPHA */
	if(  PEdst_(pe) == IdsVoid )
	     IdsCallG( args, PEfunc_(pe) );	    /* void result	    */
	else
	    {
	    result = IdsCallG(args, PEfunc_(pe));  /* keep a result copy   */
	    *PEdst_(pe) = result;		    /* ... and store it	    */
	    }
#endif
	}
#ifdef TRACE
printf( "Leaving Routine IdsExecutePipe in module IDS__PIPE \n");
#endif
    return(result );	/* Return the last non-void result  */
}

/*******************************************************************************
**  IdsDeallocatePipeDesc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**	pd	- address of initialized pipeline descriptor structure
**
**  FUNCTION VALUE:
**
**	none
**
*******************************************************************************/
void IdsDeallocatePipeDesc( pd )
 IdsPipeDesc pd;
{
#ifdef TRACE
printf( "Entering Routine IdsDeallocatePipeDesc in module IDS__PIPE \n");
#endif

    _ImgCfree( PDargs_(pd) );
    _ImgCfree( PDbase_(pd) );
    _ImgCfree( pd );

#ifdef TRACE
printf( "Leaving Routine IdsDeallocatePipeDesc in module IDS__PIPE \n");
#endif
}

/*
** IdsCallG
**
**  Used on ultrix, OSF, and ALPHA to replace lib$callg which
**  is VAX/VMS specific.
*/
#if ( !defined (VMS) || defined(ALPHA) )
unsigned long IdsCallG(arglist,procedure)
long arglist[];
unsigned long (*procedure)();
{
#ifdef TRACE
printf( "Entering Routine IdsCallG in module IDS__PIPE \n");
#endif


switch (arglist[0])
{
case 0:
     return (*procedure)();
     break;
case 1:
     return (*procedure)(arglist[1]);
     break;
case 2:
     return (*procedure)(arglist[1],arglist[2]);
     break;
case 3:
     return (*procedure)(arglist[1],arglist[2],arglist[3]);
     break;
case 4:
     return (*procedure)(arglist[1],arglist[2],arglist[3],arglist[4]);
     break;
case 5:
     return (*procedure)(arglist[1],arglist[2],arglist[3],arglist[4],
			 arglist[5]);
     break;
case 6:
     return (*procedure)(arglist[1],arglist[2],arglist[3],arglist[4],
			 arglist[5],arglist[6]);
     break;
case 7:
     return (*procedure)(arglist[1],arglist[2],arglist[3],arglist[4],
			 arglist[5],arglist[6],arglist[7]);
     break;
case 8:
     return (*procedure)(arglist[1],arglist[2],arglist[3],arglist[4],
			 arglist[5],arglist[6],arglist[7],arglist[8]);
     break;
case 9:
     return (*procedure)(arglist[1],arglist[2],arglist[3],arglist[4],
			 arglist[5],arglist[6],arglist[7],arglist[8],
			 arglist[9]);
     break;
case 10:
     return (*procedure)(arglist[1],arglist[2],arglist[3],arglist[4],
			 arglist[5],arglist[6],arglist[7],arglist[8],
			 arglist[9],arglist[10]);
     break;
case 11:
     return (*procedure)(arglist[1],arglist[2],arglist[3],arglist[4],
			 arglist[5],arglist[6],arglist[7],arglist[8],
			 arglist[9],arglist[10],arglist[11]);
     break;
default:
     printf("IdsCallG called with unsupported # of arguments %d\n",arglist[0]);
     exit();
     }

#ifdef TRACE
printf( "Leaving Routine IdsCallG in module IDS__PIPE \n");
#endif
}
#endif
