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
 *	@(#)$RCSfile: cm_db_defs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:32:37 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * OSF/1 Release 1.0
 */

/*

 */

/*
 *      DB:     Conversion list structure from stanza file string to unique
 *              int type
 */
typedef struct {
        char *  value;
        int     type;
} cvtlist_t;


extern cvtlist_t	cvtlist_type[];
extern cvtlist_t	cvtlist_lflags[];

/*
 *      DB:     List of conversion lists structure
 */
typedef struct cvtlistlist {
	int		type;
	cvtlist_t *	cvtlist;
} cvtlistlist_t;

cvtlistlist_t	cvtlists[] = {
	TYPE_LIST,	cvtlist_type,
	LFLAGS_LIST,	cvtlist_lflags,
	0,		NULL
};



/*
 * 	DB:	Conversion list for SUBSYS_MODE
 */
cvtlist_t	cvtlist_type[] = {
	"Static"		,TYPE_STATIC		,
	"Dynamic"		,TYPE_DYNAMIC		,
	""			,TYPE_NONE
};


/*
 * 	DB:	Conversion list for LFLAGS
 */
cvtlist_t	cvtlist_lflags[] = {
	"Nounload"		,LOADER_FLAGS_NOUNLOAD	,
	"Noinit"		,LOADER_FLAGS_NOINIT	,
	"Nounrefs"		,LOADER_FLAGS_NOUNREFS	,
	"Nopreexist"		,LOADER_FLAGS_NOPREXIST	,
	"Exportonly"		,LOADER_FLAGS_EXPORTONLY,
	"Wire"			,LOADER_FLAGS_WIRE	,
	"None"			,LOADER_FLAGS_NONE	,
	""			,LOADER_FLAGS_NONE
};

