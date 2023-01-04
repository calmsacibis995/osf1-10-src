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
static char	*sccsid = "@(#)$RCSfile: cm_method.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/05/05 13:55:39 $";
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
 * cm_method.c
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "cfgmgr.h"
#include "cm_cmdpkt.h"
#include "cm.h"
#include "cm_method.h"


/*
 *      CMGR:
 */
typedef struct method {
        struct method *         method_next;
        int			method_type;
        char 			method_name[CFGMGR_MAXATRSIZ];
        char                    method_path[CFGMGR_MAXATRSIZ];
        int                     method_load_refcnt;
        int                     method_configure_refcnt;
        kmod_id_t               method_id;
        ldr_load_flags_t        method_flgs;
        int                     (*method_entrypt)();
} method_t;

method_t *      method_list = NULL;

/*
 *	METHOD:	Load subsystem configuration method (if needed)
 *
 *	ALGORITHM: If the method type is dynamic then dynamically load the
 *	configuration method into the cfgmgr.
 *	If the method type is static then lookup the method in the static
 *	cfgmgr method table.
 *
 *	RETURNS:
 *	METHOD_ENOENT - If the method structure is NULL or no method entry
 *			point is found.
 *	METHOD_ELOAD - If unable to dynamically load the method.
 */
int
method_load( method_t * mp )
{
	int	rc;
	extern cmgr_cmfunc_t    cmgr_cmfunc[];
	cmgr_cmfunc_t *   	cmgr_cmp = cmgr_cmfunc;

	if (mp == NULL)
		return(METHOD_ENOENT);

	if (mp->method_type == TYPE_DYNAMIC) {
        	ldr_process_t	cmgr_proc;

	    	cmgr_proc = ldr_my_process();
	    	if ((mp->method_id=load(mp->method_path, mp->method_flgs)) != 0)
		    	return(METHOD_ELOAD);
	    	mp->method_entrypt = ldr_entry(mp->method_id);
	} else {
		for( ;cmgr_cmp->name != NULL; cmgr_cmp++ )
			if (strcmp(cmgr_cmp->name,mp->method_name) == 0)
				break;
		mp->method_entrypt = cmgr_cmp->func;
	}

	if (mp->method_entrypt == (int (*)())NULL) {
		method_unload(mp);
		return(METHOD_ENOENT);
	}

	return(0);
}


/*
 *	METHOD:	Unload subsystem configuration method (if needed)
 *
 *	ALGORITHM: If the method type is dynamic then dynamically unload the
 *	configuration method from the cfgmgr and reset the method info.
 *	If the method type is static then reset the method info.
 *
 *	RETURNS:
 *	METHOD_ENOENT - If the method structure is NULL.
 *	METHOD_EUNLOAD - If unable to dynamically unload the method.
 */
int
method_unload( method_t * mp )
{
	int	rc = 0;

	if (mp == NULL)
		return(METHOD_ENOENT);

	if (mp->method_type == TYPE_DYNAMIC && mp->method_id > 0)
		if (unload(mp->method_id) != 0)
			rc = METHOD_EUNLOAD;
	mp->method_id = 0;
	mp->method_entrypt = (int (*)())NULL;
	return(rc);
}



/*
 *	METHOD:	create method structure
 *
 *	ALGORITHM: Create new method structure and initialize fields.
 *	Lookup in supplied entry the necessary required method related fields.
 *
 * 	RETURNS:
 *	METHOD_ENOMEM - If a new method structure can not be created.
 */
int
method_create( ENT_t entry, method_t ** methodp )
{
	method_t * 	mp;
	char *		name;
	char *		path;

	if ((mp=(method_t *)malloc(sizeof(method_t))) == NULL)
		return(METHOD_ENOMEM);

	*methodp = mp;

	mp->method_next = NULL;
	mp->method_load_refcnt = 0;
	mp->method_configure_refcnt = 0;
	mp->method_id = 0;
	mp->method_entrypt = (int (*)())NULL;
	mp->method_type =dbattr_match_type(entry,METHOD_TYPE, TYPE_LIST);
	mp->method_flgs =dbattr_match_type(entry,METHOD_FLAGS, LFLAGS_LIST);

	strncpy(mp->method_name, "", sizeof(mp->method_name));
	name = dbattr_value(entry, METHOD_NAME);
	if (name != NULL)
		strncpy(mp->method_name, name, sizeof(mp->method_name));

	strncpy(mp->method_path, "", sizeof(mp->method_path));
	path = dbattr_value(entry, METHOD_PATH);
	if (path != NULL)
		strncpy(mp->method_path, path, sizeof(mp->method_path));

	return(0);
}



/*
 *	METHOD:	destroy method structure
 *
 *	ALGORITHM: Destroy new method structure.
 *
 */
int
method_destroy( method_t ** methodp)
{
	free((void *)*methodp);
	*methodp = NULL;
	return(0);
}


/*
 *	METHOD:	Locate a method structure
 *
 *	ALGORITHM: Search the linked list of method structures looking for
 *	method by method path and method name.
 *
 */
method_t *
method_lookup( int type, char * name, char * path )
{
	method_t *	r = method_list;

	if (name == NULL)
		return(NULL);
	for( ;r != NULL ; r = r->method_next )
		if (r->method_type == type
		&& !strcmp(r->method_name,name)
		&& (path == NULL || !strcmp(r->method_path,path)))
			break;
	return(r);
}

/*
 *	METHOD:	Register a method structure
 *
 *	ALGORITHM: Place the new method structure at the head of the method
 *	structure linked list.
 *
 */
void
method_register( method_t * mp )
{
	method_t *	r = method_list;

	if (mp == NULL)
		return;

	for( ;r != NULL && r->method_next != NULL; r = r->method_next )
	;
	if (r == NULL)
		method_list = mp;
	else
		r->method_next = mp;
	return;
}

/*
 *	METHOD:	Unregister a method structure
 *
 *	ALGORITHM: Remove the method structure from the linked list of method
 *	structures.
 *
 */
void
method_unregister( method_t * mp )
{
	method_t *	r = method_list;

	if (mp == NULL)
		return;

	if (r != NULL) {
		if (r == mp) {
			method_list = r->method_next;
		} else {
			for( ;r->method_next != NULL; r = r->method_next ) {
				if (r->method_next == mp) {
					r->method_next = mp->method_next;
					break;
				}
			}
		}
	}
	return;
}


/*
 *	METHOD:	Load and run configuration method
 *
 *	RETURNS:
 *
 */
int
method_call( cm_log_t * logp, char * entryname, int op, char * opts )
{
	int			rc;
	int			methodtype;
	int			rop;
	char *			methodpath;
	char *			methodname;
	method_t *		method;
	ENT_t			entry;
	static AFILE_t		afd = NULL;

	/*
	 *	If pending reset action - close database for reopen
	 */
	if (CMGR.reset == TRUE) {
		dbfile_close(afd);
		afd = NULL;
		CMGR.reset = FALSE;
	}

	/*
	 *	Open DB, if not already open
	 */
	if (afd == NULL && (rc=dbfile_open_dflt(&afd)) != 0)
		return(rc);

	/*
	 *	Get DB configuration entry
	 */
	dbfile_rewind(afd);
	if ((rc=dbent_lookup(afd, entryname, &entry)) != 0)
		return(rc);

	/*
	 *	Attach to KLS
	 */
	if ((rc=cm_kls_attach()) != 0)
		goto ret_method_call;

        /*
         *      Lookup subsystem configuration method (create if nec.)
         */
	methodtype = dbattr_match_type(entry, METHOD_TYPE, TYPE_LIST);
	methodname = AFgetval(AFgetatr(entry, METHOD_NAME));
	methodpath = AFgetval(AFgetatr(entry, METHOD_PATH));
	if ((method=method_lookup(methodtype,methodname,methodpath)) == NULL) {

		if ((rc=method_create(entry, &method)) != 0)
			goto ret_method_call;

		if ((rc=method_load(method)) != 0) {
			method_destroy(&method);
			goto ret_method_call;
		}
		method_register(method);
	}

	/*
	 *	Call configuration method
	 */
	rop = 0;
	if (((*(method->method_entrypt))(logp, entry, op, &rop, opts)) != 0) {
		rc = METHOD_EFAIL;
		goto ret_method_call;
	} 
	if (rop & CM_OP_LOAD)
		method->method_load_refcnt++;
	if (rop & CM_OP_UNLOAD)
		method->method_load_refcnt--;
	if (rop & CM_OP_CONFIGURE)
		method->method_configure_refcnt++;
	if (rop & CM_OP_UNCONFIGURE)
		method->method_configure_refcnt--;

	/*
	 *	Unload configuration method, if not needed
	 */
	if (method != NULL
	&&  method->method_load_refcnt == 0
	&&  method->method_configure_refcnt == 0) {
#ifdef DEBUG
fprintf(stderr, "Able to release method\n");
#endif
		method_unload(method);
		method_unregister(method);
		method_destroy(&method);
	}

ret_method_call:
	/*
	 *	Detach from KLS
	 */
	(void) cm_kls_detach();

	return(rc);
}



/*
 *	METHOD:	Method message reporting
 *	ARGUMENTS:
 *		cm_log_t log	- Passed to methods entry pt by cfgmgr
 *		int level	- Priority level (see <syslog.h>)
 *				  >= LOG_DEBUG is only sent if "cfgmgr -d"
 *				  >= LOG_INFO is only sent if "cfgmgr -d|v"
 *		va_list args	- Message format, followed by format list
 *				  The message can not exceed BUFSIZ
 */
void
#ifndef	_NO_PROTO
cm_log( cm_log_t * log, int level, char * format, ...)
#else
cm_log(log, level, format)
cm_log_t *log;
int level;
char *format;
#endif
{
	unsigned int	mlen, rlen;
        char    	message[BUFSIZ +1];
	va_list 	args;

	if (log->log_lvl < level)
		return;

	va_start(args, format);
	vsprintf(message, format, args);
	va_end(args);

	if (log->log_type & CM_LOG_SYSLOG)
		cfgmgr_log(level, message);

	if (log->log_type & CM_LOG_FILE && log->log_fd >= 0) {
		mlen = strlen(message);
		errno = 0;
		if ((rlen=write(log->log_fd, message, mlen)) != mlen)
		     cfgmgr_log(LOG_ERR,"%s: %s: %s\n",
			CMGR.progname, "write ", strerror(errno));
	}

	if ((log->log_type & CM_LOG_MSG) && CMGR.client) {
		mlen = strlen(message);
		errno = 0;
        	switch ( log->log_domain ) {
        	case AF_UNIX:
			if (write(log->log_fd, message, mlen) != mlen)
			    cfgmgr_log(LOG_ERR,"%s: %s: %s\n",
				CMGR.progname, "write  ", strerror(errno));
			break;
#ifdef CFG_INET
		case AF_INET:
			if (send(log->log_fd, message, mlen, 0) != mlen)
			    cfgmgr_log(LOG_ERR, "%s: %s: %s\n",
				CMGR.progname, "send", strerror(errno));
			break;
#endif
		default:
			break;
        	}
        }
	return;
}

