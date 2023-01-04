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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/10/11 17:37:29 $";
#endif 
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
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */
#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1985 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 *******************************************************************************
 *  
 *   main.c --
 *  
 *  	Main routine and some action routines for the name server
 *	lookup program.
 *
 *  	Andrew Cherenson
 *      U.C. Berkeley Computer Science Div.
 *      CS298-26, Fall 1985
 *  
 *******************************************************************************
 */



#include <sys/param.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <stdio.h>
#include <strings.h>
#include "res.h"
#include "pathnames.h"

/*
 *  Default Internet address of the current host.
 */

#if BSD < 43
#define LOCALHOST "127.0.0.1"
#endif


/*
 * Name of a top-level name server. Can be changed with 
 * the "set root" command.
 */

#ifndef ROOT_SERVER
#define         ROOT_SERVER "ns.nic.ddn.mil."
#endif
char            rootServerName[NAME_LEN] = ROOT_SERVER;


/*
 *  Import the state information from the resolver library.
 */

extern struct state _res;


/*
 *  Info about the most recently queried host.
 */

HostInfo	curHostInfo;
int		curHostValid = FALSE;


/*
 *  Info about the default name server.
 */

HostInfo 	*defaultPtr = NULL;
char 		defaultServer[NAME_LEN];
struct in_addr	defaultAddr;


/*
 *  Initial name server query type is Address.
 */

int 		queryType = T_A;
int 		queryClass = C_IN;

/*
 * Stuff for Interrupt (control-C) signal handler.
 */

extern void 	IntrHandler();
int 		sockFD = -1;
FILE 		*filePtr;
jmp_buf 	env;

static void CvtAddrToPtr();
static void ReadRC();


/*
 *******************************************************************************
 *
 *  main --
 *
 *	Initializes the resolver library and determines the address
 *	of the initial name server. The yylex routine is used to
 *	read and perform commands.
 *
 *****************************************************************************
 */

main(argc, argv)
    int		argc;
    char	**argv;
{
    char	*wantedHost = NULL;
    Boolean	useLocalServer;
    int		result;
    int		i;
    struct hostent	*hp;
    extern int	h_errno;

	setlocale(LC_ALL, "");
        catd = NLcatopen(MF_NSLOOKUP, NL_CAT_LOCALE);
    /*
     *  Initialize the resolver library routines.
     */

    if (res_init() == -1) {
	fprintf(stderr, MSGSTR(CANT_RES, "*** Can't initialize resolver.\n"));
	exit(1);
    }

    /*
     *  Allocate space for the default server's host info and
     *  find the server's address and name. If the resolver library
     *  already has some addresses for a potential name server,
     *  then use them. Otherwise, see if the current host has a server.
     *  Command line arguments may override the choice of initial server. 
     */

    defaultPtr = (HostInfo *) Calloc(1, sizeof(HostInfo));

    /*
     * Parse the arguments:
     *  no args =  go into interactive mode, use default host as server
     *	1 arg	=  use as host name to be looked up, default host will be server
     *		   non-interactive mode
     *  2 args	=  1st arg: 
     *		     if it is '-', then 
     *		        ignore but go into interactive mode
     *	 	     else 
     *		         use as host name to be looked up, 
     *			 go into non-interactive mode
     *  	   2nd arg: name or inet address of server
     *  "Set" options are specified with a leading - and must come before
     *  any arguments. For example, to find the well-known services for
     *  a host, type "nslookup -query=wks host
     *
     */

     ReadRC();                   /* look for options file */

     ++argv; --argc;             /* skip prog name */

    while (argc && *argv[0] == '-' && argv[0][1]) {
        (void) SetOption (&(argv[0][1]));
        ++argv; --argc;
    }
    if (argc > 2) {
        Usage();
    }
    if (argc && *argv[0] != '-') {
        wantedHost = *argv;     /* name of host to be looked up */
    }

    useLocalServer = FALSE;
    if (argc == 2) {
        struct in_addr addr;

        /*
         * Use an explicit name server. If the hostname lookup fails,
         * default to the server(s) in resolv.conf.
         */
    	addr.s_addr = inet_addr(*++argv);
        if (addr.s_addr != (unsigned int)-1) {
            _res.nscount = 1;
            _res.nsaddr.sin_addr = addr;
        } else {
            hp = gethostbyname(*argv);
            if (hp == NULL) {
		fprintf(stderr, "*** Can't find server address for '%s': ",
                        *argv);
                herror((char *)NULL);
                fputc('\n', stderr);
            } else {
#if BSD < 43
                bcopy(hp->h_addr, &_res.nsaddr.sin_addr, hp->h_length);
		_res.nscount = 1;
#else
		for (i = 0; i < MAXNS && hp->h_addr_list[i] != NULL; i++) {
                    bcopy(hp->h_addr_list[i],
                            (char *)&_res.nsaddr_list[i].sin_addr,
                            hp->h_length);
                }
                _res.nscount = i;
#endif    
            }
        }
    }
    
    if (_res.nscount == 0 || useLocalServer) {
        LocalServer(defaultPtr);
    } else {
        for (i = 0; i < _res.nscount; i++) {
            if (_res.nsaddr_list[i].sin_addr.s_addr == INADDR_ANY) {
                LocalServer(defaultPtr);
                break;
  	    } else {
		result = GetHostInfoByAddr(&(_res.nsaddr_list[i].sin_addr),
                                    &(_res.nsaddr_list[i].sin_addr),
                                    defaultPtr);
	    if (result != SUCCESS) {
                    fprintf(stderr,
                    "*** Can't find server name for address %s: %s\n",
                       inet_ntoa(_res.nsaddr_list[i].sin_addr),
                       DecodeError(result));
                } else {
		    defaultAddr = _res.nsaddr_list[i].sin_addr;
                    break;
                }
            }
        }

        /*
         *  If we have exhausted the list, tell the user about the
         *  command line argument to specify an address.
         */

	if (i == _res.nscount) {
            fprintf(stderr,
            "*** Default servers are not available\n");
            exit(1);
        }

    }
    strcpy(defaultServer, defaultPtr->name);


#ifdef DEBUG
#ifdef DEBUG2
    _res.options |= RES_DEBUG2;
#endif
    _res.options |= RES_DEBUG;
    _res.retry    = 2;
#endif DEBUG

    /*
     * If we're in non-interactive mode, look up the wanted host and quit.
     * Otherwise, print the initial server's name and continue with
     * the initialization.
     */

    if (wantedHost != (char *) NULL) {
	LookupHost(wantedHost, 0);
    } else {
	PrintHostInfo(stdout, "Default Server:", defaultPtr);

    	/*
     	* Setup the environment to allow the interrupt handler to return here.
     	*/

    	(void) setjmp(env);
	
    	/* 
     	* Return here after a longjmp.
     	*/
	
    	signal(SIGINT, IntrHandler);
    	signal(SIGPIPE, SIG_IGN);
	
    	/*
     	* Read and evaluate commands. The commands are described in commands.l
     	* Yylex returns 0 when ^D or 'exit' is typed. 
     	*/

    	printf(MSGSTR(R_ARROW, "> "));
	fflush(stdout);
    	while(yylex()) {
		printf(MSGSTR(R_ARROW, "> "));
		fflush(stdout);
    	}
    }
    exit(0);
}

LocalServer(defaultPtr)
    HostInfo *defaultPtr;
{
    char        hostName[NAME_LEN];
    int         result;

    gethostname(hostName, sizeof(hostName));

#if BSD < 43
    defaultAddr.s_addr = inet_addr(LOCALHOST);
    result = GetHostInfo(&defaultAddr, C_IN, T_A, hostName, defaultPtr, 1);
    if (result != SUCCESS) {
	fprintf(stderr,
        "*** Can't find initialize address for server %s: %s\n",
                        defaultServer, DecodeError(result));
        exit(1);
    }
#else
    defaultAddr.s_addr = htonl(INADDR_ANY);
    (void) GetHostInfoByName(&defaultAddr, C_IN, T_A, "0.0.0.0", defaultPtr, 1);
    free(defaultPtr->name);
    defaultPtr->name = Calloc(1, sizeof(hostName)+1);
    strcpy(defaultPtr->name, hostName);
#endif
}



/*
 *******************************************************************************
 *
 *  Usage --
 *
 *	Lists the proper methods to run the program and exits.
 *
 *******************************************************************************
 */

Usage()
{
    fprintf(stderr, MSGSTR(USAGE, "Usage:\n"));
    fprintf(stderr,
     MSGSTR(USAGE1, 
		"\tnslookup 		# interactive mode using default server\n"));
    fprintf(stderr,
     MSGSTR(USAGE2, 
		"\tnslookup - server	# interactive mode using 'server'\n"));
    fprintf(stderr,
     MSGSTR(USAGE3, 
	"\tnslookup host		# just look up 'host' using default server\n"));
    fprintf(stderr,
     MSGSTR(USAGE4, 
	"\tnslookup host server	# just look up 'host' using 'server'\n"));
    exit(1);
}

/*
 *******************************************************************************
 *
 * IsAddr --
 *
 *      Returns TRUE if the string looks like an Internet address.
 *      A string with a trailing dot is not an address, even if it looks
 *      like one.
 *
 *      XXX doesn't treat 255.255.255.255 as an address.
 *
 *******************************************************************************
 */

Boolean
IsAddr(host, addrPtr)
    char *host;
    unsigned int *addrPtr;     /* If return TRUE, contains IP address */
{
    register char *cp;
    unsigned int addr;

    if (isdigit(host[0])) {
            /* Make sure it has only digits and dots. */
            for (cp = host; *cp; ++cp) {
                if (!isdigit(*cp) && *cp != '.')
                    return FALSE;
            }
            /* If it has a trailing dot, don't treat it as an address. */
            if (*--cp != '.') {
                if ((addr = inet_addr(host)) != (unsigned int) -1) {
                    *addrPtr = addr;
                    return TRUE;
#if 0
		} else {
                    /* XXX Check for 255.255.255.255 case */
#endif
                }
            }
    }
    return FALSE;
}


/*
 *******************************************************************************
 *
 *  SetDefaultServer --
 *
 *	Changes the default name server to the one specified by
 *	the first argument. The command "server name" uses the current 
 *	default server to lookup the info for "name". The command
 *	"lserver name" uses the original server to lookup "name".
 *
 *  Side effects:
 *	This routine will cause a core dump if the allocation requests fail.
 *
 *  Results:
 *	SUCCESS 	The default server was changed successfully.
 *	NONAUTH		The server was changed but addresses of
 *			other servers who know about the requested server
 *			were returned.
 *	Errors		No info about the new server was found or
 *			requests to the current server timed-out.
 *
 *******************************************************************************
 */

int
SetDefaultServer(string, local)
    char	*string;
    Boolean	 local;
{
    register HostInfo   *newDefPtr;
    struct in_addr      *servAddrPtr;
    struct in_addr      addr;
    char                newServer[NAME_LEN];
    int                 result;
    int                 i;

    /*
     *  Parse the command line. It maybe of the form "server name",
     *  "lserver name" or just "name".
     */

    if (local) {
	i = sscanf(string, " lserver %s", newServer);
    } else {
	i = sscanf(string, " server %s", newServer);
    }
    if (i != 1) {
	i = sscanf(string, " %s", newServer);
	if (i != 1) {
	    fprintf(stderr,MSGSTR(INV_NAME,
		 "SetDefaultServer: invalid name: %s\n"),  string);
	    return(ERROR);
	}
    }

    /*
     * Allocate space for a HostInfo variable for the new server. Don't
     * overwrite the old HostInfo struct because info about the new server
     * might not be found and we need to have valid default server info.
     */

    newDefPtr = (HostInfo *) Calloc(1, sizeof(HostInfo));


    /*
     *	A 'local' lookup uses the original server that the program was
     *  initialized with.
     *
     *  Check to see if we have the address of the server or the
     *  address of a server who knows about this domain.
     *  XXX For now, just use the first address in the list.
     */

    if (local) {
        servAddrPtr = &defaultAddr;
    } else if (defaultPtr->addrList != NULL) {
        servAddrPtr = (struct in_addr *) defaultPtr->addrList[0];
    } else {
        servAddrPtr = (struct in_addr *) defaultPtr->servers[0]->addrList[0];
    }

    result = ERROR;
    if (IsAddr(newServer, &addr.s_addr)) {
        result = GetHostInfoByAddr(servAddrPtr, &addr, newDefPtr);
        /* If we can't get the name, fall through... */
    }
    if (result != SUCCESS && result != NONAUTH) {
        result = GetHostInfoByName(servAddrPtr, C_IN, T_A,
                        newServer, newDefPtr, 1);
    }

    if (result == SUCCESS || result == NONAUTH) {
	    /*
	     *  Found info about the new server. Free the resources for
	     *  the old server.
	     */

	    FreeHostInfoPtr(defaultPtr);
	    free((char *)defaultPtr);
	    defaultPtr = newDefPtr;
	    strcpy(defaultServer, defaultPtr->name);
	    PrintHostInfo(stdout, "Default Server:", defaultPtr);
	    return(SUCCESS);
    } else {
	    fprintf(stderr, MSGSTR(FIND_ADDR, 
		"*** Can't find address for server %s: %s\n"),
		    newServer, DecodeError(result));
	    free((char *)newDefPtr);

	    return(result);
    }
}

/*
 *******************************************************************************
 *
 *  DoLookupHost --
 *
 *      Common subroutine for LookupHost and LookupHostWithServer.
 *
 *  Results:
 *	SUCCESS		- the lookup was successful.
 *	Misc. Errors	- an error message is printed if the lookup failed.
 *
 *******************************************************************************
 */

static int
DoLookup(host, servPtr, serverName)
    char        *host;
    HostInfo    *servPtr;
    char        *serverName;
{
    int result;
    struct in_addr *servAddrPtr;
    struct in_addr addr;

    /* Skip escape character */
    if (host[0] == '\\')
        host++;

    /*
     *  If the user gives us an address for an address query,
     *  silently treat it as a PTR query. If the query type is already
     *  PTR, then convert the address into the in-addr.arpa format.
     *
     *  Use the address of the server if it exists, otherwise use the
     *  address of a server who knows about this domain.
     *  XXX For now, just use the first address in the list.
     */

    if (servPtr->addrList != NULL) {
        servAddrPtr = (struct in_addr *) servPtr->addrList[0];
    } else {
        servAddrPtr = (struct in_addr *) servPtr->servers[0]->addrList[0];
    }
  
    /*
     * RFC1123 says we "SHOULD check the string syntactically for a
     * dotted-decimal number before looking it up [...]" (p. 13).
     */
    if (queryType == T_A && IsAddr(host, &addr.s_addr)) {
        result = GetHostInfoByAddr(servAddrPtr, &addr, &curHostInfo);
    } else {
        if (queryType == T_PTR) {
            CvtAddrToPtr(host);
        }
        result = GetHostInfoByName(servAddrPtr, queryClass, queryType, host,
                        &curHostInfo, 0);
    }

    switch(result) {
	case SUCCESS:
	    /*
	     *  If the query was for an address, then the curHostInfo
	     *  variable can be used by Finger.
	     *  There's no need to print anything for other query types
	     *  because the info has already been printed.
	     */
	    if (queryType == T_A) {
		curHostValid = TRUE;
		PrintHostInfo(filePtr, "Name:", &curHostInfo);
	    }
	    break;

	/*
	 * No Authoritative answer was available but we got names
	 * of servers who know about the host.
	 */
	case NONAUTH:
	    PrintHostInfo(filePtr, "Name:", &curHostInfo);
	    break;

	case NO_INFO:
	    fprintf(stderr, MSGSTR(NO_INFO_OPT, 
		"*** No %s information is available for %s\n"), 
			DecodeType(queryType), host);
	    break;

	case TIME_OUT:
	    fprintf(stderr, MSGSTR(REQ_TOUT, 
		"*** Request to %s timed-out\n"), defaultServer);
	    break;

	default:
	    fprintf(stderr, MSGSTR(NO_HOST, 
		"*** %s can't find %s: %s\n"), defaultServer, host,
		    DecodeError(result));
    }
    return(result);
}

/*
 *******************************************************************************
 *
 *  LookupHost --
 *
 *      Asks the default name server for information about the
 *      specified host or domain. The information is printed
 *      if the lookup was successful.
 *
 *  Results:
 *      ERROR           - the output file could not be opened.
 *      + results of DoLookup
 *
 *******************************************************************************
 */

int
LookupHost(string, putToFile)
    char        *string;
    Boolean     putToFile;
{
    char        host[NAME_LEN];
    char        file[NAME_LEN];
    int         result;

    /*
     *  Invalidate the current host information to prevent Finger
     *  from using bogus info.
     */

    curHostValid = FALSE;

    /*
     *   Parse the command string into the host and
     *   optional output file name.
     *
     */

    sscanf(string, " %s", host);        /* removes white space */
    if (!putToFile) {
        filePtr = stdout;
    } else {
        filePtr = OpenFile(string, file);
        if (filePtr == NULL) {
            fprintf(stderr, "*** Can't open %s for writing\n", file);
            return(ERROR);
        }
        fprintf(filePtr,"> %s\n", string);
    }

    PrintHostInfo(filePtr, "Server:", defaultPtr);

    result = DoLookup(host, defaultPtr, defaultServer);

    if (putToFile) {
        fclose(filePtr);
        filePtr = NULL;
    }
    return(result);
}

/*
 *******************************************************************************
 *
 *  LookupHostWithServer --
 *
 *	Asks the name server specified in the second argument for 
 *	information about the host or domain specified in the first
 *	argument. The information is printed if the lookup was successful.
 *
 *	Address info about the requested name server is obtained
 *	from the default name server. This routine will return an
 *	error if the default server doesn't have info about the 
 *	requested server. Thus an error return status might not
 *	mean the requested name server doesn't have info about the
 *	requested host.
 *
 *	Comments from LookupHost apply here, too.
 *
 *  Results:
 *	ERROR		- the output file could not be opened.
 *      + results of DoLookup
 *
 *******************************************************************************
 */

int
LookupHostWithServer(string, putToFile)
    char	*string;
    Boolean	putToFile;
{
    char 	file[NAME_LEN];
    char 	host[NAME_LEN];
    char 	server[NAME_LEN];
    int 	result;
    static HostInfo serverInfo;

    curHostValid = FALSE;

    sscanf(string, " %s %s", host, server);
    if (!putToFile) {
	filePtr = stdout;
    } else {
	filePtr = OpenFile(string, file);
	if (filePtr == NULL) {
	    fprintf(stderr, MSGSTR(ERR_WRITE, 
		"*** Can't open %s for writing\n"), file);
	    return(ERROR);
	}
	fprintf(filePtr,MSGSTR(R_ARROW1, "> %s\n"), string);
    }
    result = GetHostInfoByName(
                defaultPtr->addrList ?
                    (struct in_addr *) defaultPtr->addrList[0] :
                    (struct in_addr *) defaultPtr->servers[0]->addrList[0],
                C_IN, T_A, server, &serverInfo, 1);
    
    if (result != SUCCESS) {
	fprintf(stderr,MSGSTR(FIND_ADDR, 
		"*** Can't find address for server %s: %s\n"), server,
		 DecodeError(result));
    } else {
	PrintHostInfo(filePtr, "Server:", &serverInfo);

	result = DoLookup(host, &serverInfo, server);
    }
    if (putToFile) {
        fclose(filePtr);
        filePtr = NULL;
    }
    return(result);
}

/*
 *******************************************************************************
 *
 *  SetOption -- 
 *
 *	This routine is used to change the state information
 *	that affect the lookups. The command format is
 *	   set keyword[=value]
 *	Most keywords can be abbreviated. Parsing is very simplistic--
 *	A value must not be separated from its keyword by white space.
 *
 *	Valid keywords:		Meaning:
 *	all			lists current values of options.
 *	ALL			lists current values of options, including
 *				  hidden options.
 *	[no]d2			turn on/off extra debugging mode (hidden).
 *	[no]debug 		turn on/off debugging mode.
 *	[no]defname	  	use/don't use default domain name.
 *	[no]search		use/don't use domain search list.
 *	domain=NAME		set default domain name to NAME.
 *	[no]ignore		ignore/don't ignore trunc. errors (hidden).
 *	query=value		set default query type to value,
 *				value is one of the query types in RFC883
 *				without the leading T_.	(e.g. A, HINFO)
 *	[no]recurse		use/don't use recursive lookup.
 *	retry=#			set number of retries to #.
 *	root=NAME		change root server to NAME.
 *	time=#			set timeout length to #.
 *	[no]vc			use/don't use virtual circuit.
 *      port                    TCP/UDP port to server.
 *
 *      Deprecated:
 *      [no]primary             use/don't use primary server.
 *
 *  Results:
 *	SUCCESS		the command was parsed correctly.
 *	ERROR		the command was not parsed correctly.
 *
 *******************************************************************************
 */

int
SetOption(option)
    register char *option;
{
    char 	type[NAME_LEN];
    char 	*ptr;
    int		tmp;

    while (isspace(*option))
        ++option;
    if (strncmp (option, "set ", 4) == 0)
        option += 4;
    while (isspace(*option))
        ++option;

    if (*option == 0) {
	fprintf(stderr, MSGSTR( INV_OPTION, 
		"*** Invalid set command\n"));
	return(ERROR);
    } else {
	if (strncmp(option, "all", 3) == 0) {
	    ShowOptions();
	} else if (strncmp(option, "ALL", 3) == 0) {
	    ShowOptions();
	} else if (strncmp(option, "d2", 2) == 0) {	/* d2 (more debug) */
	    _res.options |= (RES_DEBUG | RES_DEBUG2);
	} else if (strncmp(option, "nod2", 4) == 0) {
	    _res.options &= ~RES_DEBUG2;
	    printf("d2 mode disabled; still in debug mode\n");
	} else if (strncmp(option, "def", 3) == 0) {	/* defname */
	    _res.options |= RES_DEFNAMES;
	} else if (strncmp(option, "nodef", 5) == 0) {
	    _res.options &= ~RES_DEFNAMES;
	} else if (strncmp(option, "do", 2) == 0) {	/* domain */
	    ptr = strchr(option, '=');
	    if (ptr != NULL) {
		sscanf(++ptr, "%s", _res.defdname);
		res_re_init();
	    }
	} else if (strncmp(option, "deb", 1) == 0) {    /* debug */
            _res.options |= RES_DEBUG;
        } else if (strncmp(option, "nodeb", 5) == 0) {
            _res.options &= ~(RES_DEBUG | RES_DEBUG2);
	} else if (strncmp(option, "ig", 2) == 0) {	/* ignore */
	    _res.options |= RES_IGNTC;
	} else if (strncmp(option, "noig", 4) == 0) {
            _res.options &= ~RES_IGNTC;
	} else if (strncmp(option, "po", 2) == 0) {     /* port */
            ptr = strchr(option, '=');
            if (ptr != NULL) {
                sscanf(++ptr, "%hu", &nsport);
            }
#ifdef deprecated
        } else if (strncmp(option, "pri", 3) == 0) {    /* primary */
            _res.options |= RES_PRIMARY;
        } else if (strncmp(option, "nopri", 5) == 0) {
            _res.options &= ~RES_PRIMARY;
#endif
	} else if (strncmp(option, "q", 1) == 0 ||	/* querytype */
	  strncmp(option, "ty", 2) == 0) {
	    ptr = index(option, '=');
	    if (ptr != NULL) {
		sscanf(++ptr, "%s", type);
		queryType = StringToType(type, queryType);
	    }
	} else if (strncmp(option, "cl", 2) == 0) {	/* query class */
	    ptr = index(option, '=');
	    if (ptr != NULL) {
		sscanf(++ptr, "%s", type);
		queryClass = StringToClass(type, queryClass);
	    }
	} else if (strncmp(option, "rec", 3) == 0) {	/* recurse */
	    _res.options |= RES_RECURSE;
	} else if (strncmp(option, "norec", 5) == 0) {
	    _res.options &= ~RES_RECURSE;
	} else if (strncmp(option, "ret", 3) == 0) {	/* retry */
	    ptr = strchr(option, '=');
	    if (ptr != NULL) {
                sscanf(++ptr, "%d", &tmp);
                if (tmp >= 0) {
                    _res.retry = tmp;
                }
            }
	} else if (strncmp(option, "ro", 2) == 0) {	/* root */
	    ptr = strchr(option, '=');
	    if (ptr != NULL) {
		sscanf(++ptr, "%s", rootServerName);
	    }
	} else if (strncmp(option, "sea", 3) == 0) {    /* search list */
            _res.options |= RES_DNSRCH;
        } else if (strncmp(option, "nosea", 5) == 0) {
            _res.options &= ~RES_DNSRCH;
	} else if (strncmp(option, "srchl", 5) == 0) {  /* domain search list */
            ptr = strchr(option, '=');
            if (ptr != NULL) {
                res_dnsrch(++ptr);
            }
        } else if (strncmp(option, "ti", 2) == 0) {     /* timeout */
            ptr = strchr(option, '=');
            if (ptr != NULL) {
                sscanf(++ptr, "%d", &tmp);
                if (tmp >= 0) {
                    _res.retrans = tmp;
                }
            }
	} else if (strncmp(option, "v", 1) == 0) {	/* vc */
	    _res.options |= RES_USEVC;
	} else if (strncmp(option, "nov", 3) == 0) {
	    _res.options &= ~RES_USEVC;
	} else {
	    fprintf(stderr, MSGSTR( INV_OPTION, 
		"*** Invalid option: %s\n"),  option);
	    return(ERROR);
	}
    }
    return(SUCCESS);
}

/*
 * Fake a reinitialization when the domain is changed.
 */
res_re_init()
{
    register char *cp, **pp;
    int n;

    /* find components of local domain that might be searched */
    pp = _res.dnsrch;
    *pp++ = _res.defdname;
    for (cp = _res.defdname, n = 0; *cp; cp++)
	if (*cp == '.')
	    n++;
    cp = _res.defdname;
    for (; n >= LOCALDOMAINPARTS && pp < _res.dnsrch + MAXDFLSRCH; n--) {
	cp = strchr(cp, '.');
	*pp++ = ++cp;
    }
    *pp = 0;
    _res.options |= RES_INIT;
}

#define SRCHLIST_SEP '/'

res_dnsrch(cp)
    register char *cp;
{
    register char **pp;
    int n;

    (void)strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
    if ((cp = strchr(_res.defdname, '\n')) != NULL)
            *cp = '\0';
    /*
     * Set search list to be blank-separated strings
     * on rest of line.
     */
    cp = _res.defdname;
    pp = _res.dnsrch;
    *pp++ = cp;
    for (n = 0; *cp && pp < _res.dnsrch + MAXDNSRCH; cp++) {
            if (*cp == SRCHLIST_SEP) {
                    *cp = '\0';
		     n = 1;
            } else if (n) {
                    *pp++ = cp;
                    n = 0;
            }
    }
    if ((cp = strchr(pp[-1], SRCHLIST_SEP)) != NULL) {
        *cp = '\0';
    }
    *pp = NULL;
}

/*
 *******************************************************************************
 *
 *  ShowOptions --
 *
 *	Prints out the state information used by the resolver
 *	library and other options set by the user.
 *
 *******************************************************************************
 */

void
ShowOptions()
{
    int i;
    register char **cp;

    PrintHostInfo(stdout, MSGSTR( OPTIONS1, "Default Server:"), defaultPtr);
    if (curHostValid) {
	PrintHostInfo(stdout, MSGSTR( OPTIONS2, "Host:"), &curHostInfo);
    }

    printf(MSGSTR( OPTIONS3, "Set options:\n"));
    printf(MSGSTR( OPTIONS4, "  %sdebug  \t"), (_res.options & RES_DEBUG) ? "" : "no");
    printf(MSGSTR( OPTIONS5, "  %sdefname\t"), (_res.options & RES_DEFNAMES) ? "" : "no");
    printf(MSGSTR( OPTIONS6, "  %ssearch\t"), (_res.options & RES_DNSRCH) ? "" : "no");
    printf(MSGSTR( OPTIONS7, "  %srecurse\t"), (_res.options & RES_RECURSE) ? "" : "no");
    printf(MSGSTR( OPTIONS10, "  %sd2\t\t"), (_res.options & RES_DEBUG2) ? "" : "no");
    printf(MSGSTR( OPTIONS8, "  %svc\n"), (_res.options & RES_USEVC) ? "" : "no");
    printf(MSGSTR( OPTIONS11, "  %signoretc\t"), (_res.options & RES_IGNTC) ? "" : "no");

    printf(MSGSTR( OPTIONS13, "  querytype=%s\t"), p_type(queryType));
    printf(MSGSTR( OPTIONS14, "  class=%s\t"), p_class(queryClass));
    printf(MSGSTR( OPTIONS15, "  timeout=%d\t"), _res.retrans);
    printf(MSGSTR( OPTIONS16, "  retry=%d\n"), _res.retry);
    printf(MSGSTR( OPTIONS20, "\n  root=%s\n"), rootServerName);
    printf(MSGSTR( OPTIONS17, "  domain=%s\n"), _res.defdname);

    if (cp = _res.dnsrch) {
        printf("  srchlist=%s", *cp);
        for (cp++; *cp; cp++) {
            printf("%c%s", SRCHLIST_SEP, *cp);
        }
        putchar('\n');
    }
    putchar('\n');

}

/*
 *******************************************************************************
 *
 *  PrintHelp --
 *
 *	Prints out the help file.
*	(Code taken from Mail.)
 *
 *******************************************************************************
 */

void
PrintHelp()
{
	register int c;
	register FILE *helpFilePtr;

	if ((helpFilePtr = fopen(_PATH_HELPFILE, "r")) == NULL) {
	    fprintf(stdout, "%s not found\r\n", _PATH_HELPFILE);
	    internal_help();
	    return;
	} 
	while ((c = getc(helpFilePtr)) != EOF) {
	    putchar((char) c);
	}
	fclose(helpFilePtr);
}

char *help_strngs[] = {

{"Commands: 	(identifiers are shown in uppercase, [] means optional)"},
{"NAME		- print info about the host/domain NAME using default server"},
{"NAME1 NAME2	- as above, but use NAME2 as server"},
{"help or ?	- print help information"},
{"set OPTION	- set an option"},
    {"    all		- print options, current server and host"},
    {"    [no]debug	- print debugging information"},
    {"    [no]d2	- print exhaustive debugging information"},
    {"    [no]defname	- append domain name to each query "},
    {"    [no]recurse	- ask for recursive answer to query"},
    {"    [no]vc	- always use a virtual circuit"},
    {"    domain=NAME	- set default domain name to NAME"},
    {"    srchlist=N1[/N2/.../N6] - set domain to N1 and search list to N1,N2, etc."},
    {"    root=NAME	- set root server to NAME"},
    {"    retry=X	- set number of retries to X"},
    {"    timeout=X	- set time-out interval to X"},
    {"    querytype=X	- set query type to one of A,CNAME,HINFO,MB,MG,MINFO,MR,MX"},
    {"    type=X	- synonym for querytype"},
    {"    class=X	- set query class to one of IN (Internet), CHAOS, HESIOD or ANY"},
{"server NAME	- set default server to NAME, using current default server"},
{"lserver NAME	- set default server to NAME, using initial server"},
{"finger [NAME]	- finger the optional NAME"},
{"root		- set current default server to the root"},
{"ls [opt] DOMAIN [> FILE] - list addresses in DOMAIN (optional: output to FILE)"},
    {"    -a		-  list canonical names and aliases"},
    {"    -h		-  list HINFO (CPU type and operating system)"},
    {"    -s		-  list well-known services"},
    {"    -d		-  list all records"},
    {"    -t TYPE	-  list records of the given type (e.g., A,CNAME,MX, etc.)"},
{"view FILE	- sort an 'ls' output file and view it with more"},
{"exit		- exit the program, ^D also exits"},
{ 0 } 
};


internal_help()
{
	register int i = 0;
	char **cp = help_strngs;

	while (*cp != NULL)
		printf("%s\r\n", *cp++);
}	






/*
 *******************************************************************************
 *
 * CvtAddrToPtr --
 *
 *      Convert a dotted-decimal Internet address into the standard
 *      PTR format (reversed address with .in-arpa. suffix).
 *
 *      Assumes the argument buffer is large enougth to hold the result.
 *
 *******************************************************************************
 */

static void
CvtAddrToPtr(name)
    char *name;
{
    char *p;
    int ip[4];
    struct in_addr addr;

    if (IsAddr(name, &addr.s_addr)) {
        p = inet_ntoa(addr);
        if (sscanf(p, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == 4) {
            sprintf(name, "%d.%d.%d.%d.in-addr.arpa.",
                ip[3], ip[2], ip[1], ip[0]);
        }
    }
}
 
/*
 *******************************************************************************
 *
 * ReadRC --
 *
 *      Use the contents of ~/.nslookuprc as options.
 *
 *******************************************************************************
 */

static void
ReadRC()
{
    register FILE *fp;
    register char *cp;
    char buf[NAME_LEN];

     if ((cp = getenv("HOME")) != NULL) {
        (void) strcpy(buf, cp);
        (void) strcat(buf, "/.nslookuprc");

        if ((fp = fopen(buf, "r")) != NULL) {
            while (fgets(buf, sizeof(buf), fp) != NULL) {
                if ((cp = strchr(buf, '\n')) != NULL) {
                    *cp = '\0';
                }
                (void) SetOption(buf);
            }
            (void) fclose(fp);
        }
    }
}
