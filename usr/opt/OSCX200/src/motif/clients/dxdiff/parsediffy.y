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
%{
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
 *	parsediffy.y - yacc parser for diff input
 *
 *	Author: Laurence P. G. Cable
 *
 *	Created : 3rd March 1988
 *
 *
 *	Description
 *	-----------
 *
 *	This yacc description file is used to create an syntax analyser
 *	to parse the output from diff. 
 *
 *	Modification History
 *	------------ -------
 *	
 *	31st March		Laurence P. G. Cable
 *
 *	Changed the number_sequence rule to reflect changes to the 
 *	NumberSequence Data structure ..... now NumberSequences of 1
 *	have the howmany field set to 1 but the 2nd number now == the 1st
 *
 *	21st April		Laurence P. G. Cable
 *
 *	Added discard_diffs support
 *
 *	
 *	25th April 1988		Laurnce P. G. Cable
 *
 *	Added support for collection of stats & init HighLightInfoPtrs in
 *	struct _edc to NULL
 *
 *	06 Aug 1990		Colin Prosser
 *
 *	Fix storage allocation bugs and portability problems.
 *	Cures seg fault reported in UWS QAR 02624.
 */

static char sccsid[] = "@(#)parsediffy.y	2.2";
%}


%token	YY_DIFF_CMD_LINE
%token	YY_DIFF_ERROR

%token	YY_COMMON_NOTIFICATION	YY_BINARY_NOTIFICATION	
%token	YY_FILES_NOTIFICATION YY_ONLY_NOTIFICATION

%token	<token.ch>	YY_DIFF_IN_1_AND_NOT_2	YY_DIFF_IN_2_AND_NOT_1
%token	<token.string>	YY_STRING

%token	<token.ch>	YY_APPEND_CMD	YY_CHANGE_CMD	YY_DELETE_CMD	

%token	YY_COMMA
%token	<token.number>	YY_LINE_NUMBER 
%token	<token.string>	YY_PATHNAME
%token	YY_EOLN
%token	YY_SEP


%type	<ns>	number_sequence
%type	<et>	edit_cmd

%{
#include <stdio.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include "dxdiff.h"
#include "filestuff.h"
#include "parsediff.h"
#include "alloc.h"

#if	YACCDEBUG
#define	NDEBUG	/* for the assert */
#endif


#if	YACCDEBUG

int yaccdebug = 1; /***** REMOVE THIS LATER *****/
extern int debug;

#define	DEBUGMSG(x)	\
	if (debug || yaccdebug) {	\
		printf(x);		\
	}

#define	DEBUGMSG1(x,y)	\
	if (debug || yaccdebug) {	\
		printf(x,y);		\
	}

#endif	YACCDEBUG



/* vars used to create chains */

/* used to chain diff's onto edit cmd nodes */

static	DflPtr	*dfl1ptr,dfl1tail;
static	DflPtr	*dfl2ptr,dfl2tail;

static  unsigned int serial = 0;

static DiffListBlk dlb;

static	BfnPtr		*bfnptr;
static	CfnPtr		*cfnptr;
static	IfnPtr		*ifnptr;
static	OfnPtr		*ofnptr;
static	DcnPtr		*dcnptr;
static	DenPtr		*denptr;
static	EdcPtr		*edcptr;
static	DflPtr		*dflptr;
static	NodeCommonPtr	*ncpptr;

#define	ChainNodeInstance(np,pptr,last)	(np)->prev = (last);		\
					*(pptr) = (last) = (np);	\
					(pptr) = &((np)->next)

#define	ChainNodeCommonInstance(np)	(np)->common.prev = (NodePtr)dlb.nptail;	\
					(np)->common.serial = serial++;	\
					*ncpptr = dlb.nptail = 		\
						(NodeCommonPtr)(np);	\
					ncpptr = (NodeCommonPtr *)&((np)->common.next)
					

StoreCachePtr	nssc;
StoreCachePtr	edcsc;
StoreCachePtr	dflsc;
StoreCachePtr	bfnsc;
StoreCachePtr	cfnsc;
StoreCachePtr	ofnsc;
StoreCachePtr	ifnsc;
StoreCachePtr	densc;
StoreCachePtr	dcnsc;

%}

%left	YY_DELETE_CMD YY_APPEND_CMD YY_CHANGE_CMD
%left 	YY_COMMA

%start	diff_output

%%

diff_output	: 
		| diff_output an_edit_context
		| diff_output common_files_notification YY_EOLN
		| diff_output binary_files_notification YY_EOLN
		| diff_output identical_files_notification YY_EOLN
		| diff_output only_in_one_notification YY_EOLN
		| diff_output diff_command_notification YY_EOLN
		| diff_output diff_error_notification YY_EOLN
		| diff_output YY_EOLN
		| diff_output error YY_EOLN
			{ yyerrok; }
		;


number_sequence	: YY_LINE_NUMBER
{
	register NumberSequencePtr tp;

#ifdef	YACCDEBUG
	DEBUGMSG1("<<number_sequence(1)>>: %d\n",$1);
#endif
	
	AllocNumberSequenceOf1(tp, nssc);
	tp->numbers[0] = $1;
	tp->numbers[1] = $1;

	$$ = tp;
}
		| YY_LINE_NUMBER YY_COMMA YY_LINE_NUMBER
%prec	YY_COMMA
{
	register NumberSequencePtr tp;

#ifdef	YACCDEBUG
	DEBUGMSG1("<<number_sequence(2)>>: %d\t",$1);
	DEBUGMSG1(" %d\n",$3);
#endif
	
	AllocNumberSequenceOf2(tp, nssc);
	tp->numbers[0] = $1;
	tp->numbers[1] = $3;

	$$ = tp;
}
		;

edit_cmd	:	YY_APPEND_CMD
			{ $$ = EAppend; }
		|	YY_DELETE_CMD
			{ $$ = EDelete; }
		|	YY_CHANGE_CMD
			{ $$ = EChange; }
		;



an_edit_cmd	: number_sequence edit_cmd number_sequence YY_EOLN
{
	register EdcPtr	tp;

#ifdef	YACCDEBUG
	DEBUGMSG("<<an_edit_cmd>>\n");
#endif

	dlb.nccnt++;
	dlb.edccnt++;

	AllocEdc(tp, $2, edcsc);
	tp->ns1 = $1;
	tp->ns2 = $3;

	tp->highl = tp->highr = (HighLightInfoPtr)NULL;

	dfl1ptr = &(tp->diffsin1);
	dfl2ptr = &(tp->diffsin2);
	
	dfl1tail = dfl2tail = (DflPtr)NULL;

	ChainNodeInstance(tp, edcptr, dlb.edctail);
	ChainNodeCommonInstance(tp);
}
		;


difference1_line	: YY_DIFF_IN_1_AND_NOT_2 YY_STRING YY_EOLN
{
	register DflPtr	tp;

#ifdef	YACCDEBUG
	DEBUGMSG("<<difference1_line>>\n");
#endif
	
	if (!discard_diffs) {
		dlb.df1cnt++;
		dlb.nccnt++;

		AllocDfl(tp, EDiffin1NotIn2, dflsc);
		(tp)->difference = $2;

		ChainNodeInstance(tp, dfl1ptr, dfl1tail);
		ChainNodeCommonInstance(tp);
	}
}
		;

difference2_line	: YY_DIFF_IN_2_AND_NOT_1 YY_STRING YY_EOLN
{
	register DflPtr	tp;

#ifdef	YACCDEBUG
	DEBUGMSG("<<difference2_line>>\n");
#endif
	
	if (!discard_diffs) {
		dlb.df2cnt++;
		dlb.nccnt++;

		AllocDfl(tp, EDiffin2NotIn1, dflsc);
		(tp)->difference = $2;

		ChainNodeInstance(tp, dfl2ptr, dfl2tail);
		ChainNodeCommonInstance(tp);
	}
}
		;

list_of_1differences 	: difference1_line list_of_differences	
			| difference1_line
			;

list_of_2differences 	: difference2_line list_of_differences	
			| difference2_line
{
}
			;

list_of_differences 	: list_of_1differences YY_SEP list_of_2differences
			| list_of_1differences
			| list_of_2differences
			;

an_edit_context	: an_edit_cmd 
{
#ifdef	YACCDEBUG
	DEBUGMSG("<<an_edit_context(1)>>\n");
#endif
}
		| an_edit_cmd list_of_differences
{
#ifdef	YACCDEBUG
	DEBUGMSG("<<an_edit_context(2)>>\n");
#endif

}
		;

binary_files_notification	: YY_BINARY_NOTIFICATION YY_PATHNAME YY_PATHNAME 
{
	register BfnPtr	tp;

#ifdef	YACCDEBUG
	DEBUGMSG1("<<binary_files_notification>>: '%s'\t",$2);
	DEBUGMSG1(" '%s'\n",$3);
#endif

	dlb.bfncnt++;
	dlb.nccnt++;

	AllocBfn(tp, bfnsc);
	tp->path1 = $2;
	tp->path2 = $3;

	ChainNodeInstance(tp, bfnptr, dlb.bfntail);
	ChainNodeCommonInstance(tp);
}
				;

identical_files_notification	: YY_FILES_NOTIFICATION YY_PATHNAME YY_PATHNAME 
{
	register IfnPtr	tp;

#ifdef	YACCDEBUG
	DEBUGMSG1("<<identical_files_notification>>: '%s'\t",$2);
	DEBUGMSG1(" '%s'\n",$3);
#endif

	dlb.ifncnt++;
	dlb.nccnt++;

	AllocIfn(tp, ifnsc);
	tp->path1 = $2;
	tp->path2 = $3;

	ChainNodeInstance(tp, ifnptr, dlb.ifntail);
	ChainNodeCommonInstance(tp);
}
				;

common_files_notification	: YY_COMMON_NOTIFICATION YY_PATHNAME YY_PATHNAME 
{
	register CfnPtr	tp;

#ifdef	YACCDEBUG
	DEBUGMSG1("<<common_files_notification>>: '%s'\t",$2);
	DEBUGMSG1(" '%s'\n",$3);
#endif

	dlb.cfncnt++;
	dlb.nccnt++;

	AllocCfn(tp, cfnsc);
	tp->path1 = $2;
	tp->path2 = $3;

	ChainNodeInstance(tp, cfnptr, dlb.cfntail);
	ChainNodeCommonInstance(tp);
}
				;

only_in_one_notification	: YY_ONLY_NOTIFICATION YY_PATHNAME YY_PATHNAME 
{
	register OfnPtr	tp;

#ifdef	YACCDEBUG
	DEBUGMSG1("<<only_in_one_notification(1)>>: '%s'\t",$2);
	DEBUGMSG1(" '%s'\n",$3);
#endif

	dlb.ofncnt++;
	dlb.nccnt++;

	AllocOfn(tp, ofnsc);
	tp->path1 = $2;
	tp->path2 = $3;

	ChainNodeInstance(tp, ofnptr, dlb.ofntail);
	ChainNodeCommonInstance(tp);
}
				| YY_ONLY_NOTIFICATION YY_PATHNAME
{
	register OfnPtr	tp;
#ifdef	YACCDEBUG
	DEBUGMSG1("<<only_in_one_notification(1)>>: '%s'\n",$2);
#endif

	dlb.ofncnt++;
	dlb.nccnt++;

	AllocOfn(tp, ofnsc);
	tp->path1 = $2;
	tp->path2 = (Pathname)NULL;

	ChainNodeInstance(tp, ofnptr, dlb.ofntail);
	ChainNodeCommonInstance(tp);
}
				;

diff_command_notification	: YY_DIFF_CMD_LINE YY_PATHNAME YY_PATHNAME 
{
	register DcnPtr	tp;

#ifdef	YACCDEBUG
	DEBUGMSG1("<<diff_command_notification(1)>>: '%s'\t",$2);
	DEBUGMSG1(" '%s'\n",$3);
#endif

	dlb.dcncnt++;
	dlb.nccnt++;

	AllocDcn(tp, dcnsc);
	tp->path1 = $2;
	tp->path2 = $3;

	ChainNodeInstance(tp, dcnptr, dlb.dcntail);
	ChainNodeCommonInstance(tp);
}
				;

diff_error_notification		: YY_DIFF_ERROR	YY_STRING 
{
	register DenPtr	tp;

#ifdef	YACCDEBUG
	DEBUGMSG1("<<diff_error_notification>>: '%s'\n",$2);
#endif

	dlb.dencnt++;
	dlb.nccnt++;

	AllocDen(tp, densc);
	tp->error = $2;

	ChainNodeInstance(tp, denptr, dlb.dentail);
	ChainNodeCommonInstance(tp);
}
				;

%%


/*
 * Note: ptrs in *dlbp MUST be set to NULL on entry !!!!
 */

parsediff(dlbp)
	DiffListBlkPtr	dlbp;
{
	int	     ret;

/*
	assert((char *)&nptail - (char *)&bfnhead != sizeof (DiffListBlk));
*/
	
	/*
	 *	this next bit is tacky, but quick
	 *
	 *	copy the (zeroed) DiffListBlk into the static head
	 * 	and tail pointers at the begining of this file
	 *	also init the stats counters at the end ....
	 */

	bcopy((char *)dlbp, (char *)&dlb, sizeof (DiffListBlk) - sizeof (dlbp->caches));

	/*
	 *	now initialise the indirect pointers to point to the
	 *	static 'head' variables
	 *
	 */
	bfnptr = &dlb.bfnhead;
	cfnptr = &dlb.cfnhead;
	ifnptr = &dlb.ifnhead;
	ofnptr = &dlb.ofnhead;
	dcnptr = &dlb.dcnhead;
	denptr = &dlb.denhead;
	edcptr = &dlb.edchead;
	dflptr = &dlb.dflhead;
	ncpptr = &dlb.nphead;
		     
	FreeBackEndStore(dlbp->caches);

	nssc =  &dlbp->caches->nssc;
	edcsc = &dlbp->caches->edcsc;
	dflsc = &dlbp->caches->dflsc;
	bfnsc = &dlbp->caches->bfnsc;
	cfnsc = &dlbp->caches->cfnsc;
	ofnsc = &dlbp->caches->ofnsc;
	ifnsc = &dlbp->caches->ifnsc;
	densc = &dlbp->caches->densc;
	dcnsc = &dlbp->caches->dcnsc;

	/* parse the input */

	serial = 1;
	ret = yyparse();

	dlbp->caches->cachesempty = False;

	/*
	 * and use this nasty copy technique to return the created
	 * lists !!
	 */

	bcopy((char *)&dlb, (char *)dlbp, sizeof (DiffListBlk) - sizeof (dlbp->caches));

	if (ret) { /* OK */
	} else { /* error */
	}

	return ret;
}
