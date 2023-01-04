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
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1992 BY            			    *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/

/****************************************************************************
 ****
 **** Author: Jim Ludwig
 **** Creation: Aug 1992
 **** Description: loadable.h
 ****	typedefs and externs for loadable server
 ****
 ****************************************************************************/

#ifndef LOADABLE_H
#define LOADABLE_H
#include <stdio.h>
#include <dlfcn.h>

#include "argparse.h"

typedef enum {False, True} Boolean;
#define NullString	(char *)NULL

/* List of version numbers for each global symbol in the library.
 */
typedef struct {
	char		* SymbolName;	/* symbol name 			   */
	void		* Symbol;	/* pointer to symbol		   */
	short		SymVersionMaj;	/* symbol version major number	   */
	short		SymVersionMin;	/* symbol version minor number 	   */
	char		SymReplaced;	/* symbol has been replaced	   */
} VersionEntry, * VersionList;

/* These are the masks to build the symbol strings for the major and minor *
 * version numbers of the libraries, the list of symbols the library
 * exports and their versions, and the list of all the symbols in the
 * the library so we can make sure everything is resolved.
 */
#define LIB_VERSION_MAJOR_SYMBOL_MASK 	"__%s_VersionMaj__"
#define LIB_VERSION_MINOR_SYMBOL_MASK 	"__%s_VersionMin__"
#define LIB_EXTERNALS_SYMBOL_MASK 	"__%s_externals__"
#define LIB_SYMBOLS_SYMBOL_MASK 	"__%s_symbols__"

/* This structure is used globally */
typedef struct _LS_Library {
    char 	* LibName;	/* name of library (symbolic name) */
    char 	* LibFileName;	/* name of library file to load	   */
    char 	* ProcName;	/* name of init procedure, if any  */
    char 	* DeviceName;	/* name of device moduleID, if any */
    void 	(*Proc)();	/* pointer to that procedure       */
    void	* dlHandle;	/* handle to loaded library 	   */
    Boolean 	LibLoaded;	/* library has been loaded	   */
    Boolean 	LibInited;	/* library has been initialized    */
    int 	LibVersionMaj;	/* major version of library	   */
    int		LibVersionMin;	/* minor version of library	   */
    Boolean	markedForUnload;/* deferred unloading flag   	   */
    VersionList Versions;	/* version list of globals in library */
    int		RefCnt;		/* reference count		   */
} LS_Library , * LS_Libraries;


/* this structure is used between the parser and the main loader,
 * and to store the lists of default and configured library options.
 * These will be opaque to the rest of the server
 */
typedef struct LS_LibraryReq {
    char 		* LibName;	/* name of library (symbolic name) */
    char 		* LibFileName;	/* name of library file to load	   */
    char 		* ProcName;	/* name of init procedure, if any  */
    char 		* DeviceName;	/* name of device moduleID, if any */
    struct LS_LibraryReq * SubLibs;	/* Sublibs for this extension 	   */
    int			NumSubLibs;	/* number of sub libs		   */
    int			_OpenLibIndex;	/* quick index lookup into openlibs*/
} LS_LibraryReq;
extern LS_LibraryReq **Libs;

#define NullLibReq	(struct LS_LibraryReq *)NULL

/* types of library sections we look for */
typedef enum {
    C_SYSTEM, 		C_CORE, 	C_DEVICE, 	C_EXTENSIONS, 
    C_FONT_RENDERERS, 	C_AUTH, 	C_TRANSPORTS, 	C_INPUT 
} LibraryTypes;
#define NUM_SECTIONS 8

extern int  * LibListCounts;
extern char * LdLibraryPath;	/* for LD_LIBRARY_PATH */

#define LD_SERVER_PATH_COMP	"Xserver" /* subdirectory to search first */
extern Boolean	ls_core_replace;

extern LS_LibraryReq	*LS_SystemLibraries;
extern LS_LibraryReq	*LS_CoreLibraries;
extern LS_LibraryReq	*LS_DeviceLibraries;
extern LS_LibraryReq	*LS_ExtensionsLibraries;
extern LS_LibraryReq	*LS_FontRenderersLibraries;
extern LS_LibraryReq	*LS_InputLibraries;
extern LS_LibraryReq	*LS_AuthProtoLibraries;
extern LS_LibraryReq	*LS_TransportsLibraries;
extern LS_Library	*LS_OpenLibraries;
extern int 		LS_NumSystemLibraries;
extern int 		LS_NumCoreLibraries;
extern int 		LS_NumDeviceLibraries;
extern int 		LS_NumExtensionsLibraries;
extern int 		LS_NumFontRenderersLibraries;
extern int 		LS_NumInputLibraries;
extern int 		LS_NumAuthProtoLibraries;
extern int 		LS_NumTransportsLibraries;
extern int 		LS_NumOpenLibraries;

extern char 		** ConfigArgv;
extern int 		ConfigArgc;

/* Some error handling stuff. 
 * This needs to be solidified a bit more.
 * We also need to work out a set of exit codes so that xdm can
 * do some more intelligent things upon server failure.
 */
extern int 		LS_errno;
extern char 		* LS_error_strings[];
#define LS_ENOMEM	1	/* can't alloc memory for something */
#define LS_ENOENT	2	/* library doesn't exist	    */
#define LS_EACCES	3	/* cannot access library	    */
#define LS_EDLOPEN	4	/* dlopen failed		    */
#define LS_EDLSYM	5	/* symbol lookup failed		    */
#define LS_EINVAL	6	/* invalid argument 		    */

#define CopyString(dst, src) \
    if ( src != (char *)NULL ) { \
        dst = (char *)malloc(strlen(src)+1); \
        if ( dst == (char *)NULL ) \
	    _lsFatalError("Out of memory.\n"); \
        strcpy(dst, src); \
    } \
    else  \
	dst = (char *)NULL

#define ReplaceString(dst, src) \
    if ( dst != (char *)NULL ) \
        free(dst); \
    CopyString(dst, src)

#define Strcpy(dst, src) \
    ((src == (char *)NULL) ? dst = NULL : strcpy(dst, src))
#define Strncpy(dst, src, n) \
    ((src == (char *)NULL) ? dst = NULL : strncpy(dst, src, n))
#define Strcmp(dst, src) \
    ((dst == (char *)NULL || src == (char *)NULL) ? 1 : strcmp(dst, src))
#define Strncmp(dst, src, n) \
    ((dst == (char *)NULL || src == (char *)NULL) ? 1 : strcmp(dst, src, n))

#define SprintfString(dst, mask, src) \
    dst = (char *)malloc(strlen(src) + strlen(mask) +1); \
    if ( dst == (char *)NULL ) \
	_lsFatalError("Out of memory.\n"); \
    sprintf(dst, mask, src)

#define BoolString(boolean) (boolean == True ? "yes" : "no")

typedef enum {LS_Failure, LS_Success} LS_Status ;

#endif /* LOADABLE_H */
