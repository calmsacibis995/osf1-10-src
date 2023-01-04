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
static char *rcsid = "@(#)$RCSfile: loadable.c,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1994/01/04 17:49:31 $";
#endif
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1992 BY                  		    *
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
 **
 ** MODIFICATION HISTORY
 **
 ** Sep 1992: Creation: JLudwig
 **
 ****************************************************************************/


#include "loadable.h"
#include <unistd.h>
#include <errno.h>

extern int errno;

/* #define DEBUG_TIME */
#ifdef DEBUG_TIME
	/* getting some time info about how expensive loading is */
#include <sys/types.h>
#include <sys/time.h>
struct timeval 	tp;
long 		time_total, 
		time_diff, 
		time_used = 0, 
		time_null = 0;
#define TIME  gettimeofday(&tp,0)
#define CUR_TIME (long)(((tp.tv_sec * 1000) + (tp.tv_usec / 1000)) & 0xffffffff)
#define TIME_START {TIME;time_diff = CUR_TIME;}
#define TIME_CLOSE {TIME;time_diff = CUR_TIME - time_diff;\
	time_used += time_diff;\
	fprintf(stderr,"time: %ld elapsed time %ld %ld\n",\
	time_diff,CUR_TIME - time_total, time_used);}
#define TIME_CLOSE_NULL {TIME;time_diff = CUR_TIME - time_diff;\
	time_null += time_diff;\
	fprintf(stderr,"time: %ld elapsed time %ld %ld\n",\
	time_diff,CUR_TIME - time_total, time_null);}
#define TIME_INIT 	TIME; time_total = CUR_TIME;
#define dprintf(a)	fprintf a ;fflush(stderr)
#else /* DEBUG_TIME */
#define CUR_TIME
#define TIME_START
#define TIME_CLOSE
#define TIME_CLOSE_NULL
#define TIME_INIT
#define dprintf(a)
#endif /* DEBUG_TIME */

/****************************************************************************
 **									   **
 **  Default library lists.						   **
 **									   **
 **  In the case of the core libraries, this list can have things appended **
 **    to it or the list can be replaced, so the list isn't set in stone.  **
 **  For the rest, the lists can be appended to and only represent options.**
 **  The libraries do not have to exist.				   **
 **									   **
 ****************************************************************************/

  /* These libraries are system libraries to load.
   * They are loaded separately since they will not have any 
   * version information in them.
   */
static LS_LibraryReq _LS_DefaultSystemLibraries[] = {
    /* math library                 */
    {"m",	"libm.so",	NullString, 	NullString,	NullLibReq, 0},
#ifdef DNETCONN
    /* DECnet stub library          */
    {"dnet_stub", "libdnet_stub.so", NullString, NullString,    NullLibReq, 0},
#endif
};

  /* These libraries comprise the core portion of the server.
   * Be careful replacing them. The interfaces of the routines and structures
   * in these libraries are the specification for the loadable server 
   * interfaces and should not be modified without changing the major
   * version numbers of the libraries and coordinating the change with
   * Digital and it's partners
   */
static LS_LibraryReq _LS_DefaultCoreLibraries[] = {
  /*** Device independent and operating system layers 			     */
    /* device independent layer     */
    {"dix",	"libdix.so",	"dix_main",	NullString,	NullLibReq, 0},
    /* font code                    */
    {"font",	"libfont.so",	NullString,	NullString,	NullLibReq, 0},
    /* operating system int.        */
    {"os",	"libos.so",	NullString,	NullString,	NullLibReq, 0},
  /*** Utility layers                                                        */
    /* x display manager protocol   */
    {"Xdmcp",	"libXdmcp.so",	NullString,	NullString,	NullLibReq, 0},
    /* authorization                */
    {"Xau",	"libXau.so",	NullString,	NullString,	NullLibReq, 0},
  /*** DDX Layers (generic ddx needed for all devices)                       */
    /* machine independent ddx      */
    {"mi",	"libmi.so",	NullString,	NullString,	NullLibReq, 0},
    /* dec workstation device int.  */
    {"_dec_ws",	"lib_dec_ws.so",NullString,	NullString,	NullLibReq, 0},
    /* monochrome ddx               */
    {"mfb",	"libmfb.so",	NullString,	NullString,	NullLibReq, 0},
    /* 8-plane color ddx            */
    {"cfb",	"libcfb.so",	NullString,	NullString,	NullLibReq, 0},
    /* 16-plane color ddx           */
    {"cfb16",	"libcfb16.so",	NullString,	NullString,	NullLibReq, 0},
    /* 24/32-plane color ddx        */
    {"cfb32",	"libcfb32.so",	NullString,	NullString,	NullLibReq, 0},
  /*** Some core server code requires the shape and shared memory (MIT)      *
     * extension to be available before InitExtensions is reached. This lib  *
     * must be loaded at startup rather than at extension load. However,     *
     * there is no init call for this extension until the InitExtensions     *
     * is reached, so we must enter this library in the extensions list as   *
     * well as here.                                                         */
    /* MIT extensions               */
    {"ext",	"libext.so",	NullString,	NullString,	NullLibReq, 0},
    /* dec shared memory transport  */
    {"_dec_smt","lib_dec_smt.so","SmtExtensionInit",NullString,	NullLibReq, 0}, 
};


/* The device libraries contain code to handle a specific device. They will  
 * be called from InitOutput in libdec. The procedure name specified         
 * below should initialize the screen and set up any screen proc vectors
 * needed for the device. It can optionally use a more generic screen
 * initialization routine. If it does, it is responsible for making sure
 * that that library is also loaded. For instance, the tx device requires
 * libcfb32 and libcfb and must load those libraries if not available
 * already. Use the routine LS_LoadLibrary.
 */
static LS_LibraryReq _LS_DefaultDeviceLibraries[] = {
    /* denali   */
    {"_kpc_denali", "lib_kpc_denali.so", "kpcInitProc", "KWS_TD  " , 
        NullLibReq, 0},
    /* sierra   */
    {"_kpc_sierra", "lib_kpc_sierra.so", "sraInitProc", "KWS_SR  " , 
        NullLibReq, 0},
    /* pvg     */
    { "_dec_pvg","lib_dec_pvg.so","pvInitProc",  "PMAGC-AA" ,	NullLibReq, 0},
    { "_dec_pvg","lib_dec_pvg.so","pvInitProc",  "PMAGC-BA" ,	NullLibReq, 0},
    /* pxg     */
    { "_dec_pxg","lib_dec_pxg.so","pxInitProc",  "PMAG-DA " ,	NullLibReq, 0},
    { "_dec_pxg","lib_dec_pxg.so","pxInitProc",  "PMAG-FA " ,	NullLibReq, 0},
    { "_dec_pxg","lib_dec_pxg.so","pxInitProc",  "PMAG-FB " ,	NullLibReq, 0},
    { "_dec_pxg","lib_dec_pxg.so","pxInitProc",  "PMAGB-FA " ,	NullLibReq, 0},
    { "_dec_pxg","lib_dec_pxg.so","pxInitProc",  "PMAGB-FB " ,	NullLibReq, 0},

    /* hx      */
    { "_dec_sfb","lib_dec_sfb.so","sfbInitProc", "PMAG-BB " ,	NullLibReq, 0},
    { "_dec_sfb","lib_dec_sfb.so","sfbInitProc", "PMAGB-BA" ,	NullLibReq, 0},

    /* hx+     */
     { "_dec_ffb","lib_dec_ffb.so","ffbInitProc", "PMAGD-AA" , NullLibReq, 0},
     { "_dec_ffb","lib_dec_ffb.so","ffbInitProc", "PMAGD   " , NullLibReq, 0},
     { "_dec_ffb","lib_dec_ffb.so","ffbInitProc", "PMAGD-BA" , NullLibReq, 0},

    /* _dec_tx      */
    { "_dec_tx","lib_dec_tx.so","ropInitProc", 	"PMAG-RO " ,	NullLibReq, 0},
    { "_dec_tx","lib_dec_tx.so","ropInitProc", 	"PMAG-JA " ,	NullLibReq, 0},
};

/* Extensions to load by default
 */
static LS_LibraryReq _LS_DefaultExtensionsLibraries[] = {
    /* Core MIT extensions */
    {"ext",	"libext.so",	"LibExtExtensionInit",	
	NullString,	NullLibReq, 0},
    /* dec shared memory transport  */
    {"_dec_smt",	"lib_dec_smt.so",	"SmtExtensionInit",	
	NullString,	NullLibReq, 0}, 
};

/* Nothing done here yet... */
static LS_LibraryReq *_LS_DefaultFontRenderersLibraries	= NULL;
static LS_LibraryReq *_LS_DefaultInputLibraries  	= NULL;
static LS_LibraryReq *_LS_DefaultAuthProtoLibraries  	= NULL;
static LS_LibraryReq *_LS_DefaultTransportsLibraries  	= NULL;

static LS_Library *LS_OpenLibraries 	= (LS_Library *)NULL;
static int 	LS_NumOpenLibraries 	= 0;

Boolean	ls_core_replace;

int 	LS_errno;
char 	* LS_error_strings[] = {
	"Cannot allocate memory for operation.",
	"Library file does not exist.",
	"Library file cannot be accessed.",
	"Cannot dynamically load library file.",
	"Cannot dynamically lookup symbol in library.",
	"Invalid argument.",
};


/****************************************************************************
 **									   **
 ** This are the lists of libraries exported to the world, though as 	   **
 ** opaque types only (see loadable_server.h)				   **
 **									   **
 ****************************************************************************/
LS_LibraryReq 	*LS_SystemLibraries;
LS_LibraryReq 	*LS_CoreLibraries;
LS_LibraryReq 	*LS_DeviceLibraries;
LS_LibraryReq 	*LS_ExtensionsLibraries;
LS_LibraryReq 	*LS_FontRenderersLibraries;
LS_LibraryReq 	*LS_InputLibraries;
LS_LibraryReq 	*LS_AuthProtoLibraries;
LS_LibraryReq 	*LS_TransportsLibraries;
int 		LS_NumSystemLibraries 		= -1;
int 		LS_NumCoreLibraries 		= -1;
int 		LS_NumDeviceLibraries 		= -1;
int 		LS_NumExtensionsLibraries 	= -1;
int 		LS_NumFontRenderersLibraries 	= -1;
int 		LS_NumInputLibraries 		= -1;
int 		LS_NumAuthProtoLibraries 	= -1;
int 		LS_NumTransportsLibraries 	= -1;

/*************************************************************************
 **									**
 ** Procedure: 		_lsFatalError					**
 ** Description: 							**
 **	prints fatal error message					**
 ** Arguments:								**
 **	string		string to print					**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Private						**
 **									**
 *************************************************************************/
void _lsFatalError(string)
char * string;
{
	fprintf(stderr, string);
	exit(-1);
}


/*************************************************************************
 **									**
 ** Procedure: 		_LS_MergeLibs					**
 ** Description: 							**
 **	Merge the default library list with the configuration list and	**
 **	produce a final list.						**
 **	If replace is set, then do not use any default libraries (this	**
 **	is really only appropriate for core stuff).			**
 **	Otherwise, if an configuration library has a corresponding 	**
 **	default library (names match) then replace the default library 	**
 **	with the configuration library.					**
 ** Arguments:								**
 **	defLib, defLibSize	default library list and count		**
 **	confLib, confLibSize	configured library list and count	**
 **	newLib, newLibSize	resultant library list and count	**
 **	replace			do not use default library list		**
 ** Return:								**
 **	newLib, newLibSize						**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Private						**
 **									**
 *************************************************************************/
static void
_LS_MergeLibs ( defLib, defLibSize, 
	    	confLib, confLibSize, 
	    	newLib, newLibSize, 
	    	replace)
    LS_LibraryReq * defLib, * confLib, **newLib;
    int defLibSize, confLibSize, *newLibSize;
    Boolean replace;
{
    register int i, j, total = 0;
    LS_LibraryReq * ptr;

    /* Determine the number of libraries needed */
    if ( replace == True ) {
	/* use the configured list only */
	total = confLibSize;
	defLibSize = 0;
    }
    else {
	total = defLibSize;

        /* see how many of the configured libs are ones we already 
	 * know we need. Found out how many additional ones are 
	 * being added.  
	*/
        for ( i = 0; i < confLibSize; i++ ) {
	    for ( j = 0; j < defLibSize; j++ ) {
	        if ( Strcmp(confLib[i].LibName, defLib[j].LibName) == 0 ) {
		    break; /* found one, don't count it */
	        }
	    }
	    if ( j == defLibSize )
	        total++;
        }
    }

    if ( total == 0 ) {
	*newLib = (LS_LibraryReq *)NULL;
	*newLibSize = 0;
	return;
    }

    *newLib = (LS_LibraryReq *)calloc(total, sizeof(LS_LibraryReq));
    if ( *newLib == (LS_LibraryReq *)NULL ) 
	_lsFatalError("Not enough space to allocate library lists.\n");
    *newLibSize = total;

    ptr = *newLib;

    if ( replace == False ) {
	/* start with the default libraries */
        for ( i = 0; i < defLibSize; i++ ) {
	    CopyString(ptr->LibName, 		defLib[i].LibName);
	    CopyString(ptr->LibFileName, 	defLib[i].LibFileName);
	    CopyString(ptr->ProcName, 		defLib[i].ProcName);
	    CopyString(ptr->DeviceName, 	defLib[i].DeviceName);
	    ptr->SubLibs 			= NullLibReq;
	    ptr->NumSubLibs 			= 0;
	    ptr++;
        }
    }
    /* replace any default libraries with corresponding config libraries,
     * if any are found. This means that configured library names cannot
     * conflict with default ones unless the intention is that the 
     * configured libraries are to replace default ones. This is useful
     * if you want to try out new versions of a library.
     */
    for ( i = 0; i < confLibSize; i++ ) {
	for ( j = 0; j < defLibSize; j++ ) {
	    if ( Strcmp(confLib[i].LibName, defLib[j].LibName) == 0 ) {
		ReplaceString((*newLib)[j].LibName, 	confLib[i].LibName);
		ReplaceString((*newLib)[j].LibFileName, confLib[i].LibFileName);
		ReplaceString((*newLib)[j].ProcName, 	confLib[i].ProcName);
		ReplaceString((*newLib)[j].DeviceName, 	confLib[i].DeviceName);
	    	ptr->SubLibs 				= confLib[i].SubLibs;
	    	ptr->NumSubLibs 			= confLib[i].NumSubLibs;
		break; /* found one, don't count it */
	    }
	}
	if ( j == defLibSize ) {
	    CopyString(ptr->LibName,	confLib[i].LibName);
	    CopyString(ptr->LibFileName,confLib[i].LibFileName);
	    CopyString(ptr->ProcName,	confLib[i].ProcName);
	    CopyString(ptr->DeviceName,	confLib[i].DeviceName);
	    ptr->SubLibs 		= confLib[i].SubLibs;
	    ptr->NumSubLibs 		= confLib[i].NumSubLibs;
	    ptr++;
	}
    }

    for ( i = 0; i < total; i++ ) {
	(*newLib)[i]._OpenLibIndex = -1;
	if ( (*newLib)[i].NumSubLibs > 0 ) 
	    for ( j = 0; j < (*newLib)[i].NumSubLibs; j++ ) 
		(*newLib)[i].SubLibs[j]._OpenLibIndex = -1;
    }

    return;
}

/*************************************************************************
 **									**
 ** Procedure: 		_LS_ListLibraryReqs 				**
 ** Description: 							**
 **	debugging routine for listing library reqs			**
 **	used for displaying info about what is being loaded		**
 ** Arguments:								**
 **	libraryList	list of libraries				**
 **	numEntries	in the list					**
 **	string		name of library list				**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Private						**
 **									**
 *************************************************************************/
static void
_LS_ListLibraryReqs(libraryList, numEntries, string)
    LS_LibraryReq 	* libraryList;
    int 		numEntries;
    char 		* string;
{
    register int i,j;

    fprintf(stderr, "%s\n", string);
    if ( numEntries <= 0 ) {
	fprintf(stderr, "\tnone\n");
	return;
    }
    fprintf(stderr, "%15.15s %15.15s %20.20s %10.10s\n",
	"Library Name", "Library File Name", "Init Proc Name", "DeviceName");
    for ( i = 0; i < numEntries; i++ )  {
        fprintf(stderr, "%15.15s %15.15s %20.20s %10.10s\n",
	    libraryList[i].LibName,
	    libraryList[i].LibFileName,
	    libraryList[i].ProcName,
	    libraryList[i].DeviceName);
	if ( libraryList[i].NumSubLibs > 0 ) 
	    for ( j = 0; j < libraryList[i].NumSubLibs; j++ )
        	fprintf(stderr, "%15.15s %15.15s %15.15s %20.20s %10.10s\n",
		    "",
	    	    libraryList[i].SubLibs[j].LibName,
	    	    libraryList[i].SubLibs[j].LibFileName,
	    	    libraryList[i].SubLibs[j].ProcName,
	    	    libraryList[i].SubLibs[j].DeviceName);
    }
    fprintf(stderr,"\n");
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_ListOpenLibraries 				**
 ** Description: 							**
 **	routine for listing all open libraries and their states		**
 **	used for displaying info about what has been loaded		**
 ** Arguments:								**
 **	none								**
 ** Globals:								**
 **	LS_NumOpenLibraries, LS_OpenLibraries				**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Public						**
 **									**
 *************************************************************************/
void
LS_ListOpenLibraries()
{
    register int i,j;

    fprintf(stderr, "%s\n", "Open librararies");
    if ( LS_NumOpenLibraries <= 0 ) {
	fprintf(stderr, "\tnone\n");
	return;
    }
    fprintf(stderr, "%15.15s %15.15s %20.20s %10.10s\n",
	"Library Name", "Library File Name", "Init Proc Name", "DeviceName");
    fprintf(stderr, "\t%16.16s %16.16s %3.3s %3.3s %3.3s %3.3s %3.3s %3.3s\n",
	"proc addr", "handle", "ld", "in", "maj", "min", "ref", "mrk");
    for ( i = 0; i < LS_NumOpenLibraries; i++ ) {
        fprintf(stderr, "%15.15s %15.15s %20.20s %10.10s\n",
	    LS_OpenLibraries[i].LibName,
	    LS_OpenLibraries[i].LibFileName,
	    LS_OpenLibraries[i].ProcName,
	    LS_OpenLibraries[i].DeviceName);
        fprintf(stderr, 
	    "\t%16.16lx %16.16lx %3.3s %3.3s %3.3d %3.3d %3.3d %3.3s\n",
	    LS_OpenLibraries[i].Proc,
	    LS_OpenLibraries[i].dlHandle,
	    BoolString(LS_OpenLibraries[i].LibLoaded),
	    BoolString(LS_OpenLibraries[i].LibInited),
	    LS_OpenLibraries[i].LibVersionMaj,
	    LS_OpenLibraries[i].LibVersionMin,
	    LS_OpenLibraries[i].RefCnt,
	    BoolString(LS_OpenLibraries[i].markedForUnload));
    }
    fprintf(stderr,"\n");
}

/*************************************************************************
 **									**
 ** Procedure: 		_LS_SetLdLibraryPath				**
 ** Description: 							**
 **	we want to add the subdirectory Xserver to every path component **
 ** 	in the LD_LIBRARY_PATH. This routines finds the current path	**
 **	and modifies it. We get the path from 				**
 **	1) configuration file						**
 **	2) environment variable						**
 **	3) system default path (listed here)				**
 **	If the configuration file path starts or ends with a :, then	**
 **	append or prepend that path to the environment or default path	**
 ** Arguments:								**
 **	none								**
 ** Globals used:							**
 **	LdLibraryPath							**
 ** Environment variables:						**
 **	LD_LIBRARY_PATH							**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	set LdLibraryPath						**
 **	set environment variable LD_LIBRARY_PATH			**
 ** Scope: 		Private						**
 **									**
 *************************************************************************/

#define LD_LIBRARY_PATH_GETENV "LD_LIBRARY_PATH"
#define LD_LIBRARY_PATH_DEFAULT "/usr/shlib"

static void
_LS_SetLdLibraryPath()
{
    char 	* tmp, 
		* ptr,
		* head, 
		* tail,
		* save_path;
    int 	component_count = 0;

    /* If the library load path provided begins with : or ends with :
     * then prepend or append the default path.
     * Do this by saving the ldlibrarypath and faking out the default
     * load below.
     */
    if ( LdLibraryPath != (char *)NULL &&
	(LdLibraryPath[0] == ':' || 
	LdLibraryPath[strlen(LdLibraryPath)-1] == ':') )  {
	save_path = LdLibraryPath;
	LdLibraryPath = NullString;
    }
    else
	save_path = (char *)NULL;

    /* get the library load path if none has been provided */
    if ( LdLibraryPath == (char *)NULL ) {
	/* they haven't overridden the default or environment path */
	/* NOTE: getenv and putenv don't make copies of things -- 
	 * programmer must allocate the space! */
	tmp = getenv (LD_LIBRARY_PATH_GETENV);
	if ( tmp == (char *)NULL ) 
	    tmp = LD_LIBRARY_PATH_DEFAULT;
	LdLibraryPath = (char *)malloc(strlen(tmp) + 1);
	strcpy(LdLibraryPath, tmp);
    }

    /* If we started with a path to prepend or append, do the 'ending now */
    if ( save_path != (char *)NULL &&
	 (save_path[0] == ':' || save_path[strlen(save_path)-1] == ':') )  {
	ptr = (char *)malloc(strlen(LdLibraryPath) + strlen(save_path) + 1 );
        if ( save_path[0] == ':' ) 
	    /* append the saved path */
	    sprintf(ptr, "%s%s", LdLibraryPath, save_path);
        else
	    /* prepend the path */
	    sprintf(ptr, "%s%s", save_path, LdLibraryPath);
	free(save_path);
	LdLibraryPath = ptr;
    }

    /* duplicate and add a suffix to each component */
    ptr = LdLibraryPath;
    while(*ptr != '\0') 
	if ( *ptr++ == ':' ) 
	    component_count++;
    component_count++;
    tmp = (char *)malloc(strlen(LdLibraryPath)*2 + 
	component_count * (1 + strlen (LD_SERVER_PATH_COMP)) + 1);

    /* special case of a null LdLibraryPath */
    if ( LdLibraryPath[0] == '\0' ) {
	sprintf(tmp, "./%s:./", LD_SERVER_PATH_COMP);
    }
    else {
        ptr = tmp;
        head = tail = LdLibraryPath;
        while( *head != '\0' ) {
	    tail++;
	    if ( *tail == '\0' ) {
	        sprintf(ptr, "%s/%s:%s", head, LD_SERVER_PATH_COMP, head );
		ptr += strlen(ptr);
		head = tail;
	    }
	    else if ( *tail == ':' ) {
	        *tail = '\0';
	        sprintf(ptr, "%s/%s:%s:", head, LD_SERVER_PATH_COMP, head );
		ptr += strlen(ptr);
	        head = ++tail;
	    }
        }
    }

    LdLibraryPath = tmp;

    tmp = (char *)malloc(strlen(LdLibraryPath) + 
	strlen(LD_LIBRARY_PATH_GETENV) + 2);
    sprintf(tmp,"%s=%s",LD_LIBRARY_PATH_GETENV,LdLibraryPath);
    if ( putenv(tmp) != 0 ) {
	fprintf(stderr, "Cannot modify %s to be \n\t%s\n",
	    LD_LIBRARY_PATH_GETENV, LdLibraryPath);
	fprintf(stderr, "Aborting.\n");
	exit(1);
    }
    /* DO NOT FREE tmp here. It's not putenv's job to create new
     * space, it only uses what you pass it.
     */

    return;
}


/*************************************************************************
 **									**
 ** Procedure: 		_LS_LoadLibrary					**
 ** Description: 							**
 **	Loads a library and adds it to the global list of open libaries.**
 **	Libraries are opened if not already opened.			**
 **	Reference count is incremented.					**
 **	Libraries are opened with lazy evaluation.			**
 **	Version info and externals list must be found in library (if	**
 **	requested). This info won't be found in system libraries.	**
 ** Arguments:								**
 **	library		library record					**
 **	enforeVersion	must find version and externals info		**
 ** Globals used:							**
 **	LS_OpenLibraries						**
 **	LS_NumOpenLibraries						**
 ** Return:								**
 **	LS_Success or LS_Failure					**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Private						**
 ** Todo:								**
 **	Version verification						**
 **	Symbol overloading						**
 **									**
 *************************************************************************/

int open3d_seen_3d = 0 ;

#define ALLOC_LIBS	50

static LS_Status 
_LS_LoadLibrary(library, enforceVersion, evalMode)
    LS_LibraryReq 	* library;
    Boolean		enforceVersion;
    int			evalMode;
{
    void 	* handle;
    register int found = -1;
    LS_Library	* ptr;
    Boolean	reuseEntry = False;
    char 	scratch[256];
    void 	* symbol;
    char 	* libName 	= library->LibName,
		* libFileName 	= library->LibFileName, 
		* procName 	= library->ProcName,
		* deviceName 	= library->DeviceName;
    static int 	_LS_NumOpenLibsAlloced = 0;

    /* First, look to see if the library has already been opened. 
     * If there was already an entry, but the library was closed. Most 
     * likely closed on a previous incantation of the server and it 
     * needs to be reopened.
     */
    if ( (found = library->_OpenLibIndex) > 0 ) {
	if ( LS_OpenLibraries[found].RefCnt == 0 )  
	    reuseEntry = True;
	else {
	    LS_OpenLibraries[found].RefCnt++;
	    return (LS_Success);
	}
    }
    if ( found < 0 ) {
      for ( found = 0; found < LS_NumOpenLibraries; found++ ) {
	if ( Strncmp(libName,LS_OpenLibraries[found].LibName, 
	    strlen(libName))==0 ){
	    library->_OpenLibIndex = found;
	    if ( LS_OpenLibraries[found].RefCnt == 0 )  {
		/* There was already an entry, but the library was
		 * closed. Most likely closed on a previous incantation
		 * of the server and it needs to be reopened.
		 */
		reuseEntry = True;
		break;
	    }

	    LS_OpenLibraries[found].RefCnt++;
	    if ( Strncmp(libFileName, LS_OpenLibraries[found].LibFileName, 
		    strlen(libFileName)) != 0 ) {
		fprintf(stderr, 
		    "Warning: request to load library %s again.\n",
		    libName);
		fprintf(stderr, 
		    "         new request to load library from file %s.\n",
		    libFileName);
		fprintf(stderr,
		    "         library already loaded from file %s.\n",
		    LS_OpenLibraries[found].LibFileName);
		fprintf(stderr,
		    "         new library will not be loaded.\n");
		fprintf(stderr,
		    "         problems may result.\n");
		return(LS_Success);
	    }
	    if ( Strncmp(procName, LS_OpenLibraries[found].ProcName, 
		    strlen(procName)) != 0 ) {
		/* replace the proc name */
		ReplaceString(LS_OpenLibraries[found].ProcName,procName);
	    }
	    return (LS_Success);
	}
      }
    }

    if ( reuseEntry == False ) {
        if ( LS_NumOpenLibraries == 0 ) {
	    LS_OpenLibraries = (LS_Library *)malloc(
		    sizeof(LS_Library) * ALLOC_LIBS);
	    if ( LS_OpenLibraries == (LS_Library *)NULL ) {
	        LS_errno = LS_ENOMEM;
	        return (LS_Failure);
	    }
	    _LS_NumOpenLibsAlloced = ALLOC_LIBS;
        } 
        else if ( LS_NumOpenLibraries == _LS_NumOpenLibsAlloced ) {
	    _LS_NumOpenLibsAlloced += ALLOC_LIBS;
	    LS_OpenLibraries = (LS_Library *)realloc(LS_OpenLibraries,
		    sizeof(LS_Library) * _LS_NumOpenLibsAlloced);
	    if ( LS_OpenLibraries == (LS_Library *)NULL ) {
	        LS_errno = LS_ENOMEM;
	        return (LS_Failure);
	    }
        }
    }

    /* This code hack will prevent Xdec from loading 3d extensions when no
     * 3d device specific shareable is available to resolve all the symbols
     * the 3d extensions need.
     */

    /* lib_dec_pvg.so and lib_dec_pxg.so and lib_dec_ffb.so contains 3d support
     * If we see any of them, set a flag.
     */

    if( open3d_seen_3d == 0 &&
        ( Strcmp( libFileName, "lib_dec_pvg.so" ) == 0 ||
          Strcmp( libFileName, "lib_dec_pxg.so" ) == 0 ||
          Strcmp( libFileName, "lib_dec_ffb.so" ) == 0 ) )
      open3d_seen_3d = 1 ;

    /* If there are attempts to load any of these 3d extensions libraries,
     * usually because it's requested by Xserver.conf, refuse to load them
     * if none of the 3d shareables have been seen.
     */

    if( open3d_seen_3d == 0 &&
        ( Strcmp( libFileName, "lib_dec_3dlib.so"   ) == 0 ||
          Strcmp( libFileName, "lib_dec_x3d_pex.so" ) == 0 ||
          Strcmp( libFileName, "lib_dec_opengl.so"  ) == 0 ) ) {
	fprintf(stderr, "Cannot load library %s: 3D supported graphics device not available\n",
		libName ) ;
	LS_errno = LS_EDLOPEN;
	return (LS_Failure);
    }

    dprintf((stderr,"dlopen(%s)\n", libFileName));
    TIME_START

    handle = dlopen(libFileName, evalMode);

    TIME_CLOSE
    dprintf((stderr,"dlopen(%s) returns %lx\n", libFileName,handle));

    if ( handle == (void *)NULL ) {
	fprintf(stderr, "Cannot load library %s: %s\n",libName,dlerror());
	fprintf(stderr, "Cannot find or open library %s in path:\n%s\n",
		libFileName, LdLibraryPath);
	LS_errno = LS_EDLOPEN;
	return (LS_Failure);
    }

    if ( reuseEntry == False ) {
        ptr = &LS_OpenLibraries[LS_NumOpenLibraries];
	library->_OpenLibIndex = LS_NumOpenLibraries;
        LS_NumOpenLibraries ++;
    }
    else {
	library->_OpenLibIndex = found;
	ptr = &LS_OpenLibraries[found];
    }

    ptr->LibName 		= (char *)NULL;
    ptr->LibFileName 		= (char *)NULL;
    ptr->ProcName 		= (char *)NULL;
    ptr->DeviceName 		= (char *)NULL;
    CopyString(ptr->LibName,	libName);
    CopyString(ptr->LibFileName,libFileName);
    CopyString(ptr->ProcName,	procName);
    CopyString(ptr->DeviceName,	deviceName);
    ptr->dlHandle 	 	= handle;
    ptr->LibLoaded 	 	= True;
    ptr->LibInited 	 	= False;
    ptr->markedForUnload 	= False;
    ptr->RefCnt 	 	= 1;

    /* Resolve version symbols */

    if ( enforceVersion == False ) {
	/* must be a system library (by convention at least) */
	ptr->Proc = NULL;
	ptr->LibVersionMaj = 0;
	ptr->LibVersionMin = 0;
	ptr->Versions = (VersionList)NULL;
	return (LS_Success);
    }

    if ( procName != (char *)NULL ) {
        symbol = (void *)dlsym(ptr->dlHandle,procName);
        if ( symbol == (void *)NULL ) 
	    ptr->Proc = NULL;
        else
	    ptr->Proc = symbol;
    }
    else
	ptr->Proc = NULL;

    sprintf(scratch,LIB_VERSION_MAJOR_SYMBOL_MASK,ptr->LibName);
    symbol = (void *)dlsym(ptr->dlHandle,scratch);
    if ( symbol == (void *)NULL ) {
	fprintf(stderr, "Cannot find major version in library %s\n",
	    ptr->LibName);
	fprintf(stderr, "Improperly constructed library.\n");
	LS_errno = LS_EDLSYM;
	return (LS_Failure);
    }
    else
	ptr->LibVersionMaj = *(short *)symbol;

    sprintf(scratch,LIB_VERSION_MINOR_SYMBOL_MASK,ptr->LibName);
    symbol = (void *)dlsym(ptr->dlHandle,scratch);
    if ( symbol == (void *)NULL ) {
	fprintf(stderr, "Cannot find minor version in library %s\n",
	    ptr->LibName);
	fprintf(stderr, "Improperly constructed library.\n");
	LS_errno = LS_EDLSYM;
	return (LS_Failure);
    }
    else
	ptr->LibVersionMin = *(short *)symbol;

    sprintf(scratch,LIB_EXTERNALS_SYMBOL_MASK,ptr->LibName);
    symbol = (void *)dlsym(ptr->dlHandle,scratch);
    if ( symbol == (void *)NULL ) {
	fprintf(stderr, "Cannot find symbol version list in library %s\n",
	    ptr->LibName);
	fprintf(stderr, "Improperly constructed library.\n");
	fprintf(stderr, 
	"Assuming versions of all symbols in library match library version.\n");
	ptr->Versions = (VersionList)NULL;
    }
    else
	ptr->Versions = (VersionList)symbol;

    return (LS_Success);
}

/*************************************************************************
 **									**
 ** Procedure: 		_LS_CloseLibrary				**
 ** Description: 							**
 **	Closes library and resets structure elements			**
 ** Arguments:								**
 **	library		library pointer 				**
 ** Globals used:							**
 **	none								**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Private						**
 **									**
 *************************************************************************/
static void
_LS_CloseLibrary(ptr)
    LS_Library	* ptr;
{
    dprintf((stderr,"dlclose(%lx) %s\n", ptr->dlHandle, ptr->LibFileName));
    TIME_START

    dlclose(ptr->dlHandle);

    TIME_CLOSE

    ptr->Proc 		= NULL;
    ptr->dlHandle 	= (void *)NULL;
    ptr->LibLoaded 	= False;
    ptr->LibInited 	= False;
    ptr->markedForUnload= False;
    ptr->LibVersionMaj 	= 0;
    ptr->LibVersionMin 	= 0;
    ptr->Versions 	= (VersionList)NULL;
    ptr->RefCnt 	= 0;
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_FreeMarkedLibraries				**
 ** Description: 							**
 **	Closes all libraries marked for closure.			**
 **	In some cases, we didn't want to modify existing server code 	**
 **	too much. The obvious places to close libraries were places	**
 **	that cleaned up subportions of the server, such as 		**
 **	CloseDownExtensions. However, after these sections, there may	**
 **	have been other sections that still used code from these 	**
 **	libraries, such as FreeResources which calls destroy routines	**
 **	in the libraries. So, we provide this and the MarkLibrary	**
 **	routines to mark libraries for future closure and then allow	**
 **	closure to be deferred until after we know everything has been	**
 **	used. Marking can still occur at the place where we have the	**
 **	best handle on what to close.					**
 ** Arguments:								**
 **	none								**
 ** Globals used:							**
 **	LD_OpenLibraries (all marked entries)				**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Public						**
 **									**
 *************************************************************************/
void 
LS_FreeMarkedLibraries()
{
    LS_Library	* ptr;
    register 	int i;

    /* make sure you do this backwards, or else there will be unresolved's...*/
    for ( i = LS_NumOpenLibraries-1; i >= 0; i-- ) {
	ptr = &LS_OpenLibraries[i];
	if ( ptr->markedForUnload == True )
	    _LS_CloseLibrary(ptr);
    }
}
	
/*************************************************************************
 **									**
 ** Procedure: 		_LS_UnLoadLibrary				**
 ** Description: 							**
 **	Decrements the reference count on a library. When it goes	**
 **	to zero, closes the library.					**
 ** Arguments:								**
 **	library		library record	 				**
 **	markOnly	only mark for closure, don't actually close	**
 ** Globals used:							**
 **	LS_OpenLibraries, LS_NumOpenLibraries				**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Private						**
 **									**
 *************************************************************************/
static void
_LS_UnLoadLibrary(library, markOnly)
    LS_LibraryReq 	* library;
    Boolean		markOnly;
{
    char 	* libName 	= library->LibName;
    LS_Library	* ptr;

    if ( library->_OpenLibIndex < 0 )
	/* not opened yet */
	return;
    ptr = &LS_OpenLibraries[library->_OpenLibIndex];
    ptr->RefCnt--;
    if ( ptr->RefCnt == 0 ) 
	if ( markOnly == True ) 
	    ptr->markedForUnload = True;
	else 
	    _LS_CloseLibrary(ptr);
    return;
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_LoadLibraryReqs				**
 ** Description: 							**
 **	Load a set of libraries from the library records.		**
 **	A range can be specified from a list (since to the server, 	**
 **	the libraryReq is a opaque type and we only really know		**
 **	the index and the number of entries and the start of the list)	**
 **	enforceVersion symbols when appropriate.			**
 ** Arguments:								**
 **	libraries	list of library records				**
 **	index 		to start with					**
 **	number 		of libraries to load starting at index		**
 **	enforceVersion	symbol resolution				**
 ** Globals used:							**
 **	none								**
 ** Return:								**
 **	LS_Success or LS_Failure					**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Public						**
 **									**
 *************************************************************************/
LS_Status
LS_LoadLibraryReqs(libraries, ndx, num, enforceVersion)
    LS_LibraryReq 	* libraries;
    int			ndx;
    int 		num;
    Boolean		enforceVersion;
{
    int i;
    int result = 0;

    for ( i = ndx; i < ndx+num; i++) {
	if (_LS_LoadLibrary(&libraries[i], enforceVersion, RTLD_NOW) 
		== LS_Failure )
	    return LS_Failure;
    }
    return LS_Success;
}
LS_Status
_LS_LoadLibraryReqs(libraries, ndx, num, enforceVersion)
    LS_LibraryReq 	* libraries;
    int			ndx;
    int 		num;
    Boolean		enforceVersion;
{
    int i;
    int result = 0;

    for ( i = ndx; i < ndx+num; i++) {
	if (_LS_LoadLibrary(&libraries[i], enforceVersion, RTLD_LAZY) 
		== LS_Failure )
	    return LS_Failure;
    }
    return LS_Success;
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_UnLoadLibraryReqs				**
 ** Description: 							**
 **	Unload libraries, immediately. Do not mark.			**
 **	Do the unloading backwards, assuming that things will be loaded **
 **	in a foward manner. 						**
 ** Arguments:								**
 **	libraries	list of library records				**
 **	index		to start with					**
 **	number		of libraries to unload				**
 ** Globals used:							**
 **	none								**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Public						**
 ** Todo:								**
 **	Do the unloading backwards, assuming that things will be loaded **
 **	in a foward manner. This is heuristic and probably should be	**
 **	changed at some point. Possibly marking the sequence in the 	**
 **	list of open libraries and trying to sort backwards in that list**
 **									**
 *************************************************************************/
void 
LS_UnLoadLibraryReqs(libraries, ndx, num)
    LS_LibraryReq 	* libraries;
    int			ndx;
    int 		num;
{
    int i;

    /* make sure you do this backwards, or else there will be unresolved's...*/
    for ( i = (ndx+num-1); i >= ndx;  i-- )
	_LS_UnLoadLibrary(&libraries[i], False);
    return ;
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_MarkForUnloadLibraryReqs			**
 ** Description: 							**
 **	Mark libraries for unloading at a future time.			**
 **	Do the unloading backwards, assuming that things will be loaded **
 **	in a foward manner. 						**
 ** Arguments:								**
 **	libraries	list of library records				**
 **	index		to start with					**
 **	number		of libraries to unload				**
 ** Globals used:							**
 **	none								**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Public						**
 ** Todo:								**
 **	Do the unloading backwards, assuming that things will be loaded **
 **	in a foward manner. This is heuristic and probably should be	**
 **	changed at some point. Possibly marking the sequence in the 	**
 **	list of open libraries and trying to sort backwards in that list**
 **									**
 *************************************************************************/
void 
LS_MarkForUnloadLibraryReqs(libraries, ndx, num)
    LS_LibraryReq 	* libraries;
    int			ndx;
    int 		num;
{
    int i;

    /* make sure you do this backwards, or else there will be unresolved's...*/
    for ( i = (ndx+num-1); i >= ndx;  i-- )
	_LS_UnLoadLibrary(&libraries[i], True);
    return ;
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_Get* Routines				**
 ** Description: 							**
 **	For handling opaque data type of library records.		**
 ** Arguments:								**
 **	libraries	list of libraries				**
 **	index		into list of libraries				**
 ** Globals used:							**
 **	none								**
 ** Scope: 		Public						**
 **									**
 **	LS_GetLibName		returns pointer to library name		**
 **	LS_GetLibFileName	returns pointer to library file name	**
 **	LS_GetDeviceName	returns pointer to device name		**
 **	LS_GetInitProcName	returns pointer to init procedure name	**
 **	LS_GetInitProc		returns address of init procedure	**
 ** 	LS_GetLibraryReqBy*	returns index for library name		**
 **									**
 *************************************************************************/
char * 
LS_GetLibName(libraries, ndx)
    LS_LibraryReq	* libraries;
    int			ndx;
{
    return(libraries[ndx].LibName);
}

char * 
LS_GetLibFileName(libraries, ndx)
    LS_LibraryReq	* libraries;
    int			ndx;
{
    return(libraries[ndx].LibFileName);
}

char * 
LS_GetDeviceName(libraries, ndx)
    LS_LibraryReq	* libraries;
    int			ndx;
{
    return(libraries[ndx].DeviceName);
}

char * 
LS_GetInitProcName(libraries, ndx)
    LS_LibraryReq	* libraries;
    int			ndx;
{
    return(libraries[ndx].ProcName);
}

void * 
LS_GetInitProc(libraries, ndx)
    LS_LibraryReq	* libraries;
    int			ndx;
{
    char * libName = libraries[ndx].LibName;
    LS_Library	* ptr;

    if ( libraries[ndx]._OpenLibIndex < 0 )
	/* not opened yet */
	return ((void *)NULL);

    ptr = &LS_OpenLibraries[libraries[ndx]._OpenLibIndex];

    /* if there is already an address, return it */
    if ( ptr->Proc != NULL )
	return(ptr->Proc);

    /* if there is no procname, there is no address */
    if ( ptr->ProcName == (char *)NULL )
	return((void *)NULL);

    /* if there is a name, try to resolve it */
    if ( ptr->dlHandle != (void *)NULL )
	return(dlsym(ptr->dlHandle, ptr->ProcName));

    return ((void *)NULL);
}

int
LS_GetLibraryReqByLibName(libraries, count, name)
    LS_LibraryReq	* libraries;
    int			count;
    char		* name;
{
    register int i;
    if ( name == (char *)NULL || name[0] == '\0' )
	return -1;
    for ( i = 0; i < count; i++ ) {
	if ( libraries[i].LibName != (char *)NULL &&
	    strncmp(libraries[i].LibName, name, 
	    strlen(libraries[i].LibName)) == 0 ) 
	    return i;
    }
    return -1;
}

int
LS_GetLibraryReqByDeviceName(libraries, count, name)
    LS_LibraryReq	* libraries;
    int			count;
    char		* name;
{
    register int i;
    if ( name == (char *)NULL || name[0] == '\0' )
	return -1;
    for ( i = 0; i < count; i++ ) {
	if ( libraries[i].DeviceName != (char *)NULL &&
	    strncmp(libraries[i].DeviceName, name, 
	    strlen(libraries[i].DeviceName)) == 0 ) 
	    return i;
    }
    return -1;
}


/*************************************************************************
 **									**
 ** Procedure: 		LS_GetSubLibList 				**
 ** Description: 							**
 **	Returnes sub library list of records.				**
 **	This allows an extension to access the list of device 		**
 **	specific components for an extension.				**
 ** Arguments:								**
 **	libraries	list of libraries				**
 **	index		into list of libraries				**
 **	library list	return: pointer of library list			**
 **	count		return: number of entries in list		**
 ** Return:								**
 **	True or False, there is a sub list				**
 ** Globals used:							**
 **	none								**
 ** Scope: 		Public						**
 **									**
 *************************************************************************/
Boolean
LS_GetSubLibList(libraries, ndx, liblist_return, count_return)
    LS_LibraryReq	* libraries;
    int			ndx;
    LS_LibraryReq	** liblist_return;
    int			* count_return;
{
    if ( libraries[ndx].NumSubLibs > 0 ) {
	*liblist_return = (LS_LibraryReq *)libraries[ndx].SubLibs;
	*count_return   = libraries[ndx].NumSubLibs;
	return(True);
    }
    *count_return = 0;
    return(False);
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_GetSymbol* Routines				**
 ** Description: 							**
 **	returns addresses of symbols in either the global name space	**
 **	or the name space of a library.					**
 ** Arguments:								**
 **	symbol		name of symbol					**
 **	library 	name of library to search			**
 ** Globals used:							**
 **	Global name space available through the loader			**
 **	LS_OpenLibraries list of open libraries.			**
 ** Return:								**
 **	addresses of symbols						**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Public						**
 **									**
 *************************************************************************/
void *
LS_GetSymbol(symbolName)
    char * symbolName;
{
    if ( symbolName == (char *)NULL)
	return (void *)NULL;
    return(dlsym((void *)NULL, symbolName));
}
void *
LS_GetSymbolInLibrary(symbolName, libraries, ndx)
    char 		* symbolName;
    LS_LibraryReq	* libraries;
    int			ndx;
{
    LS_Library	* ptr;

    if ( libraries[ndx]._OpenLibIndex < 0 )
	/* not opened yet */
	return ((void *)NULL);

    ptr = &LS_OpenLibraries[libraries[ndx]._OpenLibIndex];
    return(dlsym(ptr->dlHandle, symbolName));
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_MarkLibraryInited				**
 ** Description: 							**
 **	Mark a library as being initialized. This is not the same as 	**
 **	opening a library since it will be up to the code in the server **
 **	to determine the requirements for saying a library is 		**
 **	initialized, not the generic routine here that opens the library**
 ** Arguments:								**
 **	libraries	list of libraries				**
 **	index		into list of libraries				**
 ** Globals used:							**
 **	none								**
 ** Return:								**
 **	LS_Success or LS_Failure					**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Public						**
 **									**
 *************************************************************************/
void 
LS_MarkLibraryInited(libraries, ndx)
    LS_LibraryReq	* libraries;
    int			ndx;
{
    char * libName = libraries[ndx].LibName;
    LS_Library	* ptr;

    if ( libraries[ndx]._OpenLibIndex < 0 )
	/* not opened yet */
	return ;

    ptr = &LS_OpenLibraries[libraries[ndx]._OpenLibIndex];
    ptr->LibInited = True;

    return;
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_IsLibraryInited				**
 ** Description: 							**
 **	Is a library marked as being initialized. 			**
 ** Arguments:								**
 **	libraries	list of libraries				**
 **	index		into list of libraries				**
 ** Globals used:							**
 **	none								**
 ** Return:								**
 **	True or False 							**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Public						**
 **									**
 *************************************************************************/
Boolean 
LS_IsLibraryInited(libraries, ndx)
    LS_LibraryReq	* libraries;
    int			ndx;
{
    char * libName = libraries[ndx].LibName;
    LS_Library	* ptr;

    if ( libraries[ndx]._OpenLibIndex < 0 )
	/* not opened yet */
	return False ;

    ptr = &LS_OpenLibraries[libraries[ndx]._OpenLibIndex];
    return ptr->LibInited;
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_ForceSymbolResolution			**
 ** Description: 							**
 ** 	Forces all currently loaded symbols to be resolved.		**
 **	Since we open everything lazy evaluated, we need checkpoints	**
 **	from time to time to make sure that we have everything we 	**
 **	think we do, or else things will just crash and burn.		**
 ** Arguments:								**
 **	none								**
 ** Globals used:							**
 **	none								**
 ** Return:								**
 **	LS_Success or LS_Failure					**
 ** Global impact:							**
 **	entire name space is resolved					**
 ** Scope: 		Public						**
 **									**
 *************************************************************************/
LS_Status
LS_ForceSymbolResolution()
{
    void * handle;

    dprintf((stderr,"dlopen(null)\n"));
    TIME_START

    handle = dlopen((char *)NULL, RTLD_NOW);

    TIME_CLOSE_NULL
    dprintf((stderr,"dlopen(null) returns %lx\n", handle));

    if ( handle == (void *)NULL ) {
	fprintf(stderr, "Cannot resolve all symbols present at this time.\n");
	fflush(stderr);
	return LS_Failure;
    }
    else
	return LS_Success;
}

/* This is a simple little stub for debugging purposes.
 * set a break point here and you will stop after the core libraries
 * are loaded and set to go so you can then set breakpoints
 * to stop in other places.
 */
void CallDixMain()
{
}

/*************************************************************************
 **									**
 ** Procedure: 		LS_ParseArguments				**
LS_ParseArguments( options, num_options, argc, argv )
 ** Description: 							**
 **	Parse a command line argument list for the members of the 	**
 **	options list.							**
 ** Arguments:								**
 **	options		list of options					**
 **	number		of options in list				**
 **	argv, argc	command line argument list to pull them from	**
 ** Globals used:							**
 **	the option list contains pointers to fill in			**
 **	argv and argc are adjusted if matches are found			**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Public						**
 ** Todo:								**
 **	this needs some more testing...					**
 **									**
 *************************************************************************/
static char 	* configFile = "/usr/lib/X11/Xserver.conf";
static char 	* errorFile  = "-";
static Boolean 	debug 		= False, 
		showDefaults 	= False, 
		showConfigs 	= False, 
		showUsed 	= False;

static apOptionDescRec options[] = {
  {"-config", 		apOptionSepArg,		apPointer,
	(void *)NULL, 				(void *)&configFile},
  {"-errorFile",	apOptionSepArg,		apPointer,
	(void *)NULL, 				(void *)&errorFile},
  {"-debug", 		apOptionNoArg,		apInt,
	(void *)True, 				(void *)&debug},
  {"-showDefaults", 	apOptionNoArg,		apInt,
	(void *)True, 				(void *)&showDefaults},
  {"-showConfigs", 	apOptionNoArg,		apInt,
	(void *)True, 				(void *)&showConfigs},
  {"-showUsed", 	apOptionNoArg,		apInt,
	(void *)True, 				(void *)&showUsed},
};

static void 
_ReportParseError(options, msg)
    apOptionDescRec	* options;
    char 		* msg;
{
    (void) fprintf(stderr, "Error parsing argument \"%s\"; %s\n",
		   options->option, msg);
}

#define PutCommandResource(val) \
    switch(options[i].type) { \
	case apChar: 	*(char *)options[i].value =   (char)val;  	break; \
	case apShort: 	*(short *)options[i].value = (short)val;	break; \
	case apInt: 	*(int *)options[i].value =     (int)val;	break; \
	case apLong: 	*(long *)options[i].value =   (long)val;	break; \
	case apPointer:  						       \
	default:							       \
			*(void **)options[i].value = (void *)val;	break; \
    }

void 
LS_ParseArguments( options, num_options, argc, argv )
    apOptionDescList	options;
    int			num_options;
    int 		*argc;
    char 		** argv;
{
    int 		foundOption;
    char		**argsave;
    register int	i, myargc;
    char		*optP, *argP, optchar, argchar;
    int			matches;
    enum {DontCare, Check, NotSorted, Sorted} table_is_sorted;
    char		**argend;

    myargc = (*argc); 
    argend = argv + myargc;
    argsave = ++argv;

    table_is_sorted = (myargc > 2) ? Check : DontCare;
    for (--myargc; myargc > 0; --myargc, ++argv) {
	foundOption = False;
	matches = 0;
	for (i=0; i < num_options; ++i) {
	    /* checking the sort order first insures we don't have to
	       re-do the check if the arg hits on the last entry in
	       the table.  
	     */
	    if (table_is_sorted == Check && i > 0 &&
		Strcmp(options[i].option, options[i-1].option) < 0) {
		table_is_sorted = NotSorted;
	    }
	    for (argP = *argv, optP = options[i].option;
		 (optchar = *optP++) &&
		 (argchar = *argP++) &&
		 argchar == optchar;);
	    if (!optchar) {
		if (!*argP ||
		    options[i].argKind == apOptionStickyArg ||
		    options[i].argKind == apOptionIsArg) {
		    /* give preference to exact matches, StickyArg and IsArg */
		    matches = 1;
		    foundOption = i;
		    break;
		}
	    }
	    else if (!argchar) {
		/* may be an abbreviation for this option */
		matches++;
		foundOption = i;
	    }
	    else if (table_is_sorted == Sorted && optchar > argchar) {
		break;
	    }
	    if (table_is_sorted == Check && i > 0 &&
		Strcmp(options[i].option, options[i-1].option) < 0) {
		table_is_sorted = NotSorted;
	    }
	}
	if (table_is_sorted == Check && i >= (num_options-1))
	    table_is_sorted = Sorted;
	if (matches == 1) {
	    i = foundOption;
	    switch (options[i].argKind){
		case apOptionNoArg:
		    --(*argc);
		    PutCommandResource(options[i].defValue);
		    break;
			    
		case apOptionIsArg:
		    --(*argc);
		    PutCommandResource(*argv);
		    break;

		case apOptionStickyArg:
		    --(*argc);
		    PutCommandResource(argP);
		    break;

		case apOptionSepArg:
		    if (myargc > 1) {
			++argv; --myargc; --(*argc); --(*argc);
			PutCommandResource(*argv);
		    } else
			(*argsave++) = (*argv);
		    break;
		
		case apOptionSkipArg:
		    if (myargc > 1) {
			--myargc;
			(*argsave++) = (*argv++);
		    }
		    (*argsave++) = (*argv); 
		    break;

		case apOptionSkipLine:
		    for (; myargc > 0; myargc--)
			(*argsave++) = (*argv++);
		    break;

		case apOptionSkipNArgs:
		    {
			register int j = 1 + (int) options[i].defValue;

			if (j > myargc) j = myargc;
			for (; j > 0; j--) {
			    (*argsave++) = (*argv++);
			    myargc--;
			}
			argv--;		/* went one too far before */
			myargc++;
		    }
		    break;

		default:
		    _ReportParseError (&options[i], "unknown kind");
		    break;
		}
	}
	else
	    (*argsave++) = (*argv);  /*compress arglist*/ 
    }

    if (argsave < argend)
	(*argsave)=NULL; /* put NULL terminator on compressed argv */
}

#undef _PutCommandResource

/*************************************************************************
 **									**
 ** Procedure: 		_LS_OpenConfigFile				**
 ** Description: 							**
 **	opens the configuration file for reading.			**
 **	sets stdin to be the file if it can be opened.			**
 ** Arguments:								**
 **	filename to use							**
 ** Globals used:							**
 **	none								**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	stdin is the place to read from					**
 ** Scope: 		Private						**
 **									**
 *************************************************************************/
static void 
_LS_OpenConfigFile(filename)
    char * filename;
{
    if ( filename == NULL || filename[0] == '\0' ) {
	fprintf(stderr, "Null filename specified for configuration file\n");
	fprintf(stderr, "Using standard input.\n");
	return;
    }
    if ( filename[0] == '-' )
	/* use stdin */
	return;
    if ( access(filename, R_OK) != 0 ) {
	perror(filename);
	fprintf(stderr, "Cannot access configuration file.\n");
	exit(1);
    }
    if ( freopen(filename, "r", stdin) == NULL ) {
	perror(filename);
	fprintf(stderr, "Cannot open configuration file.\n");
	exit(1);
    }
    return;
}

/*************************************************************************
 **									**
 ** Procedure: 		_LS_OpenErrorFile				**
 ** Description: 							**
 **	open the error file, if possible, and make it stderr.		**
 ** Arguments:								**
 **	filename to use							**
 ** Globals used:							**
 **	none								**
 ** Return:								**
 **	none								**
 ** Global impact:							**
 **	stderr is the place to write to 				**
 ** Scope: 		Private						**
 **									**
 *************************************************************************/
static void 
_LS_OpenErrorFile(filename)
    char * filename;
{
    if ( filename == NULL || filename[0] == '\0' ) {
	fprintf(stderr, "Null filename specified for error file\n");
	fprintf(stderr, "Using standard error.\n");
	return;
    }
    if ( filename[0] == '-' )
	/* use error */
	return;
    if ( access(filename, W_OK) != 0 ) {
	perror(filename);
	fprintf(stderr, "Cannot access error file.\n");
	exit(1);
    }
    if ( freopen(filename, "w", stderr) == NULL ) {
	perror(filename);
	fprintf(stderr, "Cannot open error file.\n");
	exit(1);
    }
    return;
}

/*************************************************************************
 **									**
 ** Procedure: 		_LS_CombineArgs					**
 ** Description: 							**
 **	Combine command line argument lists.				**
 ** Arguments:								**
 **	argc, argv		set to add to				**
 **	configArgc, configArgv	set to add				**
 ** Globals used:							**
 **	none								**
 ** Return:								**
 **	modified argc and argv						**
 ** Global impact:							**
 **	none								**
 ** Scope: 		Private						**
 **									**
 *************************************************************************/
static void
_LS_CombineArgs( argc, argv, ConfigArgc, ConfigArgv )
    int	  	* argc;
    char 	*** argv;
    int		ConfigArgc;
    char 	**  ConfigArgv;
{
    char 	** newargv;
    int		newargc = *argc + ConfigArgc;
    register    int i;

    newargv = (char **)malloc(sizeof(char *) * newargc);
    if ( newargv == (char **)NULL ) {
	_lsFatalError("Out of memory.\n"); 
	exit(1);
    }
    for ( i = 0; i < *argc; i++ )
	newargv[i] = (*argv)[i];
    for ( i = *argc; i < newargc; i++ )
	newargv[i] = ConfigArgv[i-*argc];
    
    *argv = newargv;
    *argc = newargc;

    return;
}


#define NumberOf(array)	((unsigned int)(sizeof(array) / sizeof(array[0])))

main(argc, argv)
int argc;
char ** argv;
{

	int baseLibSize, i, j;
	int (*dix_main)();

	TIME_INIT
	TIME_START

	/* do some minor initialization work */
	LS_ParseArguments( options, NumberOf(options), &argc, argv );

	/* Allocate some space */
	Libs = (LS_LibraryReq **)malloc(sizeof(LS_LibraryReq *) * NUM_SECTIONS);
	LibListCounts = (int *)malloc(sizeof(int) * NUM_SECTIONS);
	for ( i = 0; i < NUM_SECTIONS; i ++ ) {
		Libs[i] = (LS_LibraryReq *)NULL;
		LibListCounts[i] = 0;
	}

	_LS_OpenConfigFile(configFile);
	_LS_OpenErrorFile(errorFile);

	/* parse the config file */
	yyparse();

	/* add any arguments found in the config file */
	_LS_CombineArgs( &argc, &argv, ConfigArgc, ConfigArgv );

	/* set up the library search path */
	_LS_SetLdLibraryPath();

	/* Merge all the lists */
	_LS_MergeLibs(
	    _LS_DefaultSystemLibraries,	NumberOf(_LS_DefaultSystemLibraries),
	    Libs[C_SYSTEM], 		LibListCounts[C_SYSTEM],
	    &LS_SystemLibraries, 	&LS_NumSystemLibraries, 
	    False);
	_LS_MergeLibs(
	    _LS_DefaultCoreLibraries, 	NumberOf(_LS_DefaultCoreLibraries),
	    Libs[C_CORE], 		LibListCounts[C_CORE],
	    &LS_CoreLibraries, 		&LS_NumCoreLibraries, 
	    ls_core_replace);
	
	_LS_MergeLibs(
	    _LS_DefaultDeviceLibraries, NumberOf(_LS_DefaultDeviceLibraries),
	    Libs[C_DEVICE], 		LibListCounts[C_DEVICE],
	    &LS_DeviceLibraries, 	&LS_NumDeviceLibraries, 
	    False);
	
	_LS_MergeLibs(
	    _LS_DefaultExtensionsLibraries, 	
		NumberOf(_LS_DefaultExtensionsLibraries),
	    Libs[C_EXTENSIONS], 	LibListCounts[C_EXTENSIONS],
	    &LS_ExtensionsLibraries, 	&LS_NumExtensionsLibraries, 
	    False);
	
	_LS_MergeLibs(
	    _LS_DefaultFontRenderersLibraries, 	
		NumberOf(_LS_DefaultFontRenderersLibraries),
	    Libs[C_FONT_RENDERERS], 	LibListCounts[C_FONT_RENDERERS],
	    &LS_FontRenderersLibraries, &LS_NumFontRenderersLibraries, 
	    False);
	
	_LS_MergeLibs(
	    _LS_DefaultInputLibraries, 	NumberOf(_LS_DefaultInputLibraries),
	    Libs[C_INPUT], 		LibListCounts[C_INPUT],
	    &LS_InputLibraries, 	&LS_NumInputLibraries, 
	    False);
	
	_LS_MergeLibs(
	    _LS_DefaultAuthProtoLibraries, 
		NumberOf(_LS_DefaultAuthProtoLibraries),
	    Libs[C_AUTH], 		LibListCounts[C_AUTH],
	    &LS_AuthProtoLibraries, 	&LS_NumAuthProtoLibraries, 
	    False);
	
	_LS_MergeLibs(
	    _LS_DefaultTransportsLibraries, 
		NumberOf(_LS_DefaultTransportsLibraries),
	    Libs[C_TRANSPORTS], 	LibListCounts[C_TRANSPORTS],
	    &LS_TransportsLibraries, 	&LS_NumTransportsLibraries, 
	    False);
	
	TIME_CLOSE

	/* load the system and core libraries */
	if ( _LS_LoadLibraryReqs(LS_SystemLibraries, 0, LS_NumSystemLibraries,
		False) == 0 ) {
	    fprintf(stderr, "Cannot load all core libraries.\n");
	    fprintf(stderr, "Aborting.\n");
	    exit(1);
	}
	if ( _LS_LoadLibraryReqs(LS_CoreLibraries, 0, LS_NumCoreLibraries,
		True) == 0 ) {
	    fprintf(stderr, "Cannot load all core libraries.\n");
	    fprintf(stderr, "Aborting.\n");
	    exit(1);
	}

	/* flush all symbols */
	if ( LS_ForceSymbolResolution() == 0 ) {
	    fprintf(stderr, "Core set of libraries is incomplete.\n");
	    fprintf(stderr, "Cannot run with unresolved symbols.\n");
	    fprintf(stderr, "Aborting.\n");
	    exit(1);
	}

	if ( showDefaults == True ) {
	    _LS_ListLibraryReqs(
		_LS_DefaultSystemLibraries, 
		NumberOf(_LS_DefaultSystemLibraries),
		"default system libraries");
	    _LS_ListLibraryReqs(
		_LS_DefaultCoreLibraries, 
		NumberOf(_LS_DefaultCoreLibraries),
		"default core libraries");
	    _LS_ListLibraryReqs(
		_LS_DefaultDeviceLibraries, 
		NumberOf(_LS_DefaultDeviceLibraries),
		"default device libraries");
	    _LS_ListLibraryReqs(
		_LS_DefaultExtensionsLibraries, 
		NumberOf(_LS_DefaultExtensionsLibraries),
		"default extensions libraries");
	    _LS_ListLibraryReqs(
		_LS_DefaultFontRenderersLibraries, 
		NumberOf(_LS_DefaultFontRenderersLibraries),
		"default font renderer libraries");
	    _LS_ListLibraryReqs(
		_LS_DefaultAuthProtoLibraries, 
		NumberOf(_LS_DefaultAuthProtoLibraries),
		"default authorization protocol libraries");
	    _LS_ListLibraryReqs(
		_LS_DefaultTransportsLibraries, 
		NumberOf(_LS_DefaultTransportsLibraries),
		"default transports libraries");
	    _LS_ListLibraryReqs(
		_LS_DefaultInputLibraries, 
		NumberOf(_LS_DefaultInputLibraries),
		"default input libraries");
	}
	if ( showConfigs == True ) {
	    _LS_ListLibraryReqs( Libs[C_SYSTEM], 		
		LibListCounts[C_SYSTEM], "configured system libraries");
	    if ( ls_core_replace == True )
		fprintf(stderr, 
		    "configured core libraries replace default libraries.\n");
	    _LS_ListLibraryReqs( Libs[C_CORE], 		
		LibListCounts[C_CORE], "configured core libraries");
	    _LS_ListLibraryReqs( Libs[C_DEVICE], 		
		LibListCounts[C_DEVICE], "configured device libraries");
	    _LS_ListLibraryReqs( Libs[C_EXTENSIONS], 		
		LibListCounts[C_EXTENSIONS], "configured extensions libraries");
	    _LS_ListLibraryReqs( Libs[C_FONT_RENDERERS], 		
		LibListCounts[C_FONT_RENDERERS], 
		"configured font renderer libraries");
	    _LS_ListLibraryReqs( Libs[C_AUTH], 		
		LibListCounts[C_AUTH], 
		"configured authorization protocol libraries");
	    _LS_ListLibraryReqs( Libs[C_TRANSPORTS], 		
		LibListCounts[C_TRANSPORTS], 
		"configured transport libraries");
	    _LS_ListLibraryReqs( Libs[C_INPUT], 		
		LibListCounts[C_INPUT], "configured input libraries");
	}
	if ( showUsed == True ) {
	    _LS_ListLibraryReqs( LS_SystemLibraries, 
		LS_NumSystemLibraries, "used system libraries");
	    _LS_ListLibraryReqs( LS_CoreLibraries, 
		LS_NumCoreLibraries, "used core libraries");
	    _LS_ListLibraryReqs( LS_DeviceLibraries, 
		LS_NumDeviceLibraries, "used device libraries");
	    _LS_ListLibraryReqs( LS_ExtensionsLibraries, 
		LS_NumExtensionsLibraries, "used extensions libraries");
	    _LS_ListLibraryReqs( LS_FontRenderersLibraries, 
		LS_NumFontRenderersLibraries, "used font renderer libraries");
	    _LS_ListLibraryReqs( LS_AuthProtoLibraries, 
		LS_NumAuthProtoLibraries, 
		"used authorization protocol libraries");
	    _LS_ListLibraryReqs( LS_TransportsLibraries, 
		LS_NumTransportsLibraries, 
		"used transports libraries");
	    _LS_ListLibraryReqs( LS_InputLibraries, 
		LS_NumInputLibraries, "used input libraries");
	}

	/* get the starting point */
	dix_main = LS_GetSymbol("dix_main");

	if ( dix_main == NULL ) {
	    fprintf(stderr, "Cannot find dix_main.\n");
	    fprintf(stderr, "Aborting.\n");
	    exit(1);
	}

	/* convenient stopping place for debugging */
	CallDixMain();

	/* And we're off........ */
	dix_main(argc, argv);

}

