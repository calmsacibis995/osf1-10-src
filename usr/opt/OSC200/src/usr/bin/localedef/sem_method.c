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
static char     *sccsid = "@(#)$RCSfile: sem_method.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 22:16:25 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/localedef.h>
#include <sys/method.h>
#include <string.h>
#include <dlfcn.h>
#include <loader.h>
#include "semstack.h"
#include "symtab.h"
#include <ctype.h>
#include <stdlib.h>
#include "err.h"


/* 
** GLOBAL variables 
*/

char *lib_array[LAST_METHOD+1];		/* array containing all the libraries */
			                /* specified in methods source file */



static char *grab(int *n, int want, char **previous )
/*
 * ARGUMENTS:
 *	n		# of values still on the stack
 *	want		which value we want.  If want > n, use previous
 *	previous	Remembers old values
 */
{
    item_t	*it;
    char	*str;

    if (*n == want) {
	it = sem_pop();		/* Pop top and reduce count */
	*n -= 1;

	if (!it || it->type != SK_STR) {
	    INTERNAL_ERROR;	/* Should _never_ happen */
	}

	str = strdup(it->value.str);
	destroy_item(it);

	if ( previous ) *previous = str;	/* This is NEW previous */
    } else {
	if ( previous )
	  str = *previous;
	else
	  str = NULL;
    }

    return (str);
}    

/*
*  FUNCTION: set_method
*
*  DESCRIPTION:
*	Used to parse the information in the methods file. An index into
*	the std_methods array is passed to this routine from the grammer.
*	The number of items is also passed (1 means just the c_symbol is
*	passed, the package and library name is inherited from previous and 2 means
*	the c_symbol and package name is passed. 3 means everything present.
*/
void set_method(int index, int number)
{
	char *sym, *lib, *pkg;
	static char *lastlib = "/usr/shlib/libc.so";
	static char *lastpkg = "libc";
	int i;

	/* get the strings for the new method name and the library off  */
	/* of the stack */
	
	lib = grab(&number, 3, &lastlib);
	pkg = grab(&number, 2, &lastpkg);
	sym = grab(&number, 1, NULL);

	for (i=0;i<=LAST_METHOD;i++){
	    if (lib_array[i] == NULL) {	/* Reached an empty slot, add lib */
		lib_array[i] = lib;
		break;
	    } else if (!strcmp(lib_array[i],lib)) {
		break;			/* Found it already on our list */
	    }
	    /* Keep looking */
	}	

	/* add the info to the std_methods table */

	std_methods[index].c_symbol[method_class] = sym;
	std_methods[index].lib_name[method_class] = lib;
	std_methods[index].package[method_class] = pkg;
}




/*
*  FUNCTION: load_method
*
*  DESCRIPTION:
*	Load a method from a shared library.
*  INPUTS
*	idx	index into std_methods of entry to load.
*/

static void
load_method(int idx)
{
	char		*sym = std_methods[idx].c_symbol[method_class];
	char		*pkg = std_methods[idx].package[method_class];
	char		*lib = std_methods[idx].lib_name[method_class];
	void		*handle   ;
	int		(*func)() ;

	/*
	 * Load the module to make certain that the referenced package is
	 * in our address space.
	 */

	handle = dlopen( lib, RTLD_LAZY );
	if (handle == NULL) {
	    perror("localedef");
	    error(ERR_LOAD_FAIL, sym, lib );	/* Never returns */
	}
#ifdef ADD_UNDERSCORE
	{
	    char *ptr = MALLOC(char, strlen(sym) + 2);

	    strcat(strcpy(ptr, "_"), sym);
	    sym = ptr;
	}
#endif /* ADD_UNDERSCORE */


	func = dlsym(handle, sym) ;

#ifdef ADD_UNDERSCORE
	free(sym);
#endif

	if ( !func ) {
	    /*
	     * lookup failed.
	     */
	    perror("localedef");
	    error(ERR_LOAD_FAIL, sym, lib);
	}

	std_methods[idx].instance[method_class] = func ;
}


/*
*  FUNCTION: check_methods
*
*  DESCRIPTION:
*  There are certain methods that do not have defaults because they are
*  dependent on the process code and file code relationship. These methods
*  must be specified by the user if they specify any new methods at all
*
*/

void check_methods(void)
{
    int MustDefine[] = { CHARMAP___MBSTOPCS, CHARMAP___MBTOPC,
			 CHARMAP___PCSTOMBS, CHARMAP___PCTOMB,
			 CHARMAP_MBLEN,
			 CHARMAP_MBSTOWCS, CHARMAP_MBTOWC, CHARMAP_WCSTOMBS,
			 CHARMAP_WCSWIDTH, CHARMAP_WCTOMB, CHARMAP_WCWIDTH };

    int j;

    for (j = 0; j < (sizeof(MustDefine)/sizeof(int)); j++){
	int idx = MustDefine[j];

	if (std_methods[idx].instance[method_class] == NULL) {
	    /*
	     * Need to load a method to handle this operation
	     */
	    if ( !std_methods[idx].c_symbol[method_class] ) {
		/*
		 * Did not get a definition for this method, and we need
		 * it to be defined
		 */
		diag_error(ERR_METHOD_REQUIRED, std_methods[j].method_name);
	    } else {
		/*
		 * load the shared module and fill in the table slot
		 */
		load_method(idx);
	    }
	}
    }

    /*
     * Finally, run thru the remaining methods and copy from the SB_CODESET
     * (which has all "standard" methods for the remaining non-null entries.
     * We only need the symbolic info for these methods, not the function pointers
     */

    for (j = 0; j <= LAST_METHOD; j++){

	if (std_methods[j].c_symbol[method_class] == NULL) {
	    /*
	     * Still not set up?
	     */
	    std_methods[j].c_symbol[method_class] = std_methods[j].c_symbol[SB_CODESET];
	    std_methods[j].package[method_class] = std_methods[j].package[SB_CODESET];
	    std_methods[j].lib_name[method_class] = std_methods[j].lib_name[SB_CODESET];
	}
    }
}
