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
static char *rcsid = "@(#)$RCSfile: stemplate.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:14:14 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	stemplate.c,v $
 * Revision 1.1.1.1  92/05/12  01:45:00  devrcs
 *  *** OSF1_1B29 version ***
 * 
 * Revision 1.1.2.3  1992/04/05  18:20:49  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:54:17  marquard]
 *
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1989-90, SecureWare, Inc.
 *   All rights reserved.
 *
 * Based on OSF version:
 * 	@(#)stemplate.c	1.13 16:31:38 5/17/91 SecureWare 
 */

/* @(#)stemplate.c	1.1 11:19:07 11/8/91 SecureWare */

/* Template routine that will get a screen.
 * This file is meant to be included in a C file that has all
 * of the necessary support routines defined.
 * Following are the #defines which must be taken care of:
 *
 *	PARMTEMPLATE	Template scrn_parms for this screen
 *	STRUCTTEMPLATE	Template scrn_struct[] for the variables
 *	DESCTEMPLATE	Template scrn_desc[] for the screen
 *	FILLINSTRUCT	screen value structure definition
 *	FILLIN		screen value structure instance
 *
 *	FIRSTDESC	Table entry of first scrn_struct field to get
 *	NSCRNSTRUCT	number of scrn_structs for this screen.
 *
 *	SETUPFUNC	Routine to do pre-screen-initialization setup
 *	AUTHFUNC	Routine to determine if user is authorized for screen
 *	BUILDFILLIN	Routine that fills in the fillin structure
 *
 *	INITFUNC	Routine to do post-screen-initialization setup
 *	BUILDSTRUCT	Returns filled-in scrn_struct pointer for this screen
 *
 *	ROUTFUNC	Routine called after user enters data
 *	VALIDATE	Routine that validates arguments to ROUTFUNC
 *	SCREENACTION	Once structure is validated, this does the action
 *
 *	FREEFUNC	Routine to free dynamically allocated structures
 */

/* all screen functions take an argv-like argument.
 * argv[0] is the choice from the screen.
 * argv[1,2, . . .] are the menu prompt fields.
 */

#ifdef SETUPFUNC

SETUPFUNC (argv)
char	**argv;
{
	PARMTEMPLATE.first_desc = FIRSTDESC;
	memset((char *) FILLIN, '\0', sizeof(struct FILLINSTRUCT));
	return (scrnsetup (argv, (char *) FILLIN,
	  AUTHFUNC, BUILDFILLIN));
}

#endif


#ifdef INITFUNC

INITFUNC (argv)
char	**argv;
{
	return (scrn_init (argv, (char *) FILLIN, BUILDSTRUCT, &STRUCTTEMPLATE,
		&PARMTEMPLATE));
}

#endif


#ifdef ROUTFUNC

ROUTFUNC (argv)
char	**argv;
{
	return (scrnexit (argv, (char *) FILLIN,
	  VALIDATE, SCREENACTION, FIRSTDESC, NSCRNSTRUCT,
	  &PARMTEMPLATE, DESCTEMPLATE, STRUCTTEMPLATE)
	);
}

#endif


#ifdef FREEFUNC

FREEFUNC (argv)
char	**argv;
{
	return (scrnfree (argv, (char *) FILLIN, NSCRNSTRUCT, FREESTRUCT,
		&PARMTEMPLATE, DESCTEMPLATE, STRUCTTEMPLATE)
	);
}

#endif

/* so that this template can be used multiple times in the same routine,
 * undefine everything.
 */

#undef PARMTEMPLATE
#undef STRUCTTEMPLATE
#undef DESCTEMPLATE
#undef FILLINSTRUCT
#undef FILLIN

#undef FIRSTDESC
#undef NSCRNSTRUCT

#undef SETUPFUNC
#undef AUTHFUNC
#undef BUILDFILLIN

#undef INITFUNC
#undef BUILDSTRUCT

#undef ROUTFUNC
#undef VALIDATE
#undef SCREENACTION

#undef FREEFUNC
#undef FREESTRUCT
