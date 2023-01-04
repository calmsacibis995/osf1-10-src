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
static char *rcsid = "@(#)$RCSfile: method_bin_compat.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 09:02:22 $";
#endif

#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <loader.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/sysconfig.h>
#include <sys/mount.h>

#include "cm.h"

/*
 * Modules by be either static or dynamic. Static modules are linked
 * into the kernel and are always marked as loaded, configured and
 * kernel resident. When the state is read from the kernel the resulting
 * structs are marked as kernel resident.  Dynamic modules are loaded 
 * and configured by this method. 
 * When a module is requested by name (whether or not the name is valid)
 * it is added to the collection of modules and marked as having had a 
 * STANZA. This helps to keep track of where we think the name came
 * from.
 */
typedef struct {
	uint		compat_flags;
	kmod_id_t	compat_id;
	struct compat_mod	module;
} compat_mod_t;
#define compat_name	module.cm_ld_name
#define COMPAT_LOADED		0x01
#define COMPAT_CONFIGURED	0x02
#define COMPAT_STANZA		0x04
#define COMPAT_KERNEL		0x08

/*
 *      Local BSS
 */
#define		MAXCOMPAT	MAXHABITATS*5+1
compat_mod_t	cm_list[MAXCOMPAT];


/*
 *
 *	Name:		BIN_COMPAT_method()
 *	Description:	Generic OSF/1 BIN_COMPAT Configuration Method
 *	Returns:	Zero 		On success.
 *			Non-zero	On failure.
 *	Passes back:	last command that succeeded.
 *	op:		"or(ed)" flags for commands to process
 *
 *	Operation:	The method establishes it's state by calling
 *			BIN_COMPAT_lookup_compat_mod. It then (in the
 *			coded order) performs the operations specified
 *			in the (op) command. It passws back the last
 *			command that it successfully executed and
 *			returns 0 for success or -1 for error.
 */
int
BIN_COMPAT_method( cm_log_t *logp, ENT_t entry, cm_op_t op, cm_op_t *rop,
    char *opts )
{
	int		rc = -1;
	cm_op_t		rrop;
	compat_mod_t *	compat_mod;

	/* Get a pointer to the needed compat mod */
	if (rc=BIN_COMPAT_lookup_compat_mod(logp, entry, &compat_mod)) {
		return(-1);
	}
	/* Module was requested by sysconfig so there is a stanza for it */
	compat_mod->compat_flags |= COMPAT_STANZA;

	if (op & CM_OP_START) {
		rc = BIN_COMPAT_method_start(logp, entry, compat_mod);
		rrop = CM_OP_START;
	}

	if (op & CM_OP_LOAD) {
		rc = BIN_COMPAT_method_load(logp, entry, compat_mod);
		rrop = CM_OP_LOAD;
	}

	if((op&CM_OP_CONFIGURE) && (compat_mod->compat_flags&COMPAT_LOADED)){
		rc = BIN_COMPAT_method_configure(logp, entry, compat_mod);
		rrop = CM_OP_CONFIGURE;
	}

	if (op & CM_OP_QUERY) {
		rc = BIN_COMPAT_method_query(logp, entry, compat_mod);
		rrop = CM_OP_QUERY;
	}

	if (op & CM_OP_OPERATE) {
		rc = BIN_COMPAT_method_operate(logp, entry, compat_mod, opts);
		rrop = CM_OP_OPERATE;
	}

	if (op & CM_OP_UNCONFIGURE) {
		rc = BIN_COMPAT_method_unconfigure(logp, entry, compat_mod);
		rrop = CM_OP_UNCONFIGURE;
	}

	if (op & CM_OP_UNLOAD) {
		rc = BIN_COMPAT_method_unload(logp, entry, compat_mod);
		rrop = CM_OP_UNLOAD;
	}

	if (rc == 0) {
		*rop = rrop;
		return(0);
	}
	return(-1);
}


/*
 * BIN_COMPAT_method_load
 *	load the module into the kernel.
 */
int
BIN_COMPAT_method_load(cm_log_t * logp, ENT_t entry, compat_mod_t * compat_mod )
{
	int	rc = 0;

	if (compat_mod->compat_flags & COMPAT_LOADED) {
		METHOD_LOG(LOG_ERR, KMOD_LOAD_L_EBUSY );
		return(KMOD_LOAD_L_EBUSY);
	}

	compat_mod->compat_flags &= ~COMPAT_CONFIGURED;

	if ((rc=cm_kls_load(entry, &compat_mod->compat_id)) != 0) {
		METHOD_LOG(LOG_ERR, rc);
		return(rc);
	}

	compat_mod->compat_flags |=  COMPAT_LOADED;

	METHOD_LOG(LOG_INFO, MSG_LOADED );
	return(0);
}


/*
 * BIN_COMPAT_method_unload
 * 	Un-load the module from the kernel.
 */
int
BIN_COMPAT_method_unload(cm_log_t * logp,ENT_t entry,compat_mod_t * compat_mod )
{
	int	rc = 0;

	if (!(compat_mod->compat_flags & COMPAT_LOADED)) {
		METHOD_LOG(LOG_ERR, KMOD_UNLOAD_L_EEXIST);
		return(KMOD_UNLOAD_L_EEXIST);
	}

	if ( (compat_mod->module.cm_flags & CM_STATIC)) {
		METHOD_LOG(LOG_ERR, KMOD_UNLOAD_STATIC);
		return(KMOD_UNLOAD_STATIC);
	}

	if (compat_mod->compat_flags & COMPAT_CONFIGURED) {
		METHOD_LOG(LOG_ERR, KMOD_UNLOAD_C_EBUSY);
		return(KMOD_UNLOAD_C_EBUSY);
	}

	if ((rc=cm_kls_unload(compat_mod->compat_id)) != 0) {
		METHOD_LOG(LOG_ERR, rc);
		return(rc);
	}

	compat_mod->compat_id = LDR_NULL_MODULE;
	compat_mod->compat_flags &= ~COMPAT_CONFIGURED;
	compat_mod->compat_flags &= ~COMPAT_LOADED;

	METHOD_LOG(LOG_INFO, MSG_UNLOADED );
	return(0);
}


/*
 * BIN_COMPAT_method_configure
 * 	Configure the module by calling (via kmodcall) the configuration
 *	entry point that was returned by the load operation with a 
 *	SYSCONFIG_CONFIGURE command.
 */
int
BIN_COMPAT_method_configure( cm_log_t * logp, ENT_t entry, 
	compat_mod_t * compat_mod )
{
	int	rc = 0;

	if (compat_mod->compat_flags & COMPAT_CONFIGURED) {
		METHOD_LOG(LOG_ERR, KMOD_CONFIG_C_EBUSY);
		return(KMOD_CONFIG_C_EBUSY);
	}

	/* the stanza name is passed so that the kernel compat_mod 
	 * struct can be associated with a stanza name */
        if ((rc=cm_kls_call(compat_mod->compat_id, SYSCONFIG_CONFIGURE,
		compat_mod->module.cm_ld_name, 
		strlen(compat_mod->module.cm_ld_name),
		NULL, 0)) != 0) {
		if (rc == KLDR_EFAIL)
			cm_log(logp, LOG_ERR, "%s: %s\n", AFentname(entry),
				strerror(errno));
		else
			METHOD_LOG(LOG_ERR, rc);
		return(rc);
	}

	compat_mod->compat_flags |= COMPAT_CONFIGURED;

	METHOD_LOG(LOG_INFO, MSG_CONFIGURED );
	return(0);
}


/*
 * BIN_COMPAT_method_unconfigure
 * 	Unconfigure the module by calling (via kmodcall) the configuration
 *	entry point that was returned by the load operation with a 
 *	SYSCONFIG_UNCONFIGURE command.
 */
int
BIN_COMPAT_method_unconfigure( cm_log_t * logp, ENT_t entry, 
	compat_mod_t * compat_mod )
{
	int	rc;

	if ((compat_mod->module.cm_flags & CM_STATIC)) {
		METHOD_LOG(LOG_ERR, KMOD_UNLOAD_STATIC);
		return(KMOD_UNLOAD_STATIC);
	}

	if (!(compat_mod->compat_flags & COMPAT_LOADED)) {
		METHOD_LOG(LOG_ERR, KMOD_UNCONFIG_L_EEXIST);
		return(KMOD_UNCONFIG_L_EEXIST);
	}

	if (!(compat_mod->compat_flags & COMPAT_CONFIGURED)) {
		METHOD_LOG(LOG_ERR, KMOD_UNCONFIG_C_EEXIST);
		return(KMOD_UNCONFIG_C_EEXIST);
	}

        if ((rc=cm_kls_call(compat_mod->compat_id, SYSCONFIG_UNCONFIGURE,
		NULL, 0, NULL, 0)) != 0) {
		if (rc == KLDR_EFAIL)
			cm_log(logp, LOG_ERR, "%s: %s\n", AFentname(entry),
				strerror(errno));
		else
			METHOD_LOG(LOG_ERR, rc);
		return(rc);
	}

	compat_mod->compat_flags &= ~COMPAT_CONFIGURED;

	METHOD_LOG(LOG_INFO, MSG_UNCONFIGURED );
	return(0);
}


/*
 * BIN_COMPAT_method_query
 *	Extract information about the module from the kernel, format
 *	the information and return it to the caller.
 */
int
BIN_COMPAT_method_query(cm_log_t * logp, ENT_t entry, compat_mod_t * compat_mod)
{
	int	rc = 0;
	cm_log_t logq = *logp; /* for sending query back */
	int	i, len, next, last;
	char	buf[1024];
	struct  compat_query qu;

	/* All querys are returned via CM_LOG_MSG. We don't log them */
	logq.log_type = CM_LOG_MSG;

	/* last svc number */
	last = compat_mod->module.cm_base + compat_mod->module.cm_nsysent;

	/* construct the output line */
	strcpy(buf, compat_mod->module.cm_ld_name);
	strcat(buf, ": ");
	if(compat_mod->module.cm_flags & CM_STATIC) {
		strcat(buf, "STATIC ");
	} else {
		if(compat_mod->compat_flags & COMPAT_LOADED) {
			strcat(buf, "LOADED ");
			if(compat_mod->compat_flags & COMPAT_CONFIGURED)
				strcat(buf, "CONFIGURED ");
			else
				strcat(buf, "UNCONFIGURED ");
		} else 
			strcat(buf, "UNLOADED ");
	}
	/* in-kernel flags */
	if(compat_mod->compat_flags & COMPAT_KERNEL) {
		if(compat_mod->module.cm_flags & CM_TRACE)
			strcat(buf, "trace ");
		else
			strcat(buf, "notrace ");
		if(compat_mod->module.cm_flags & CM_DEBUG)
			strcat(buf, "debug ");
		else
			strcat(buf, "nodebug ");
	}
	cm_log(&logq, LOG_ERR, "%s\n", buf);

	if(compat_mod->compat_flags & COMPAT_CONFIGURED) {
		/* skip count */
		sprintf(buf, "\tDo not trace next %d system calls.", 
			compat_mod->module.cm_skipcount);
		/* number of system calls since last clear */
		sprintf(buf+strlen(buf), " Processed %d system calls.", 
			compat_mod->module.cm_nsyscalls);
		cm_log(&logq, LOG_ERR, "%s\n", buf);

		if(! compat_mod->module.cm_habitat ) {
			/* usage stats, curent & total */
			sprintf(buf, "\t%d executables currently active.", 
				compat_mod->module.cm_refcount);
			cm_log(&logq, LOG_ERR, "%s\n", buf);
	
			sprintf(buf, 
			    "\tModule has serviced a total of %d executables.",
				compat_mod->module.cm_totalcount);
			cm_log(&logq, LOG_ERR, "%s\n", buf);
		}

		/* get trace data */
		len = sizeof(qu);
		next = 0;
		while ( next < last && next >= 0) {
			/* use count from kernel */
			bzero(&qu, sizeof(qu));
			if(compat_mod->module.cm_flags & CM_STATIC) {
			    if (syscall(SYS_kmodcall, 
				compat_mod->module.cm_configure, 
				SYSCONFIG_QUERY, &next, 4, &qu, len)==-1) {
				cm_log(logp, LOG_ERR, "%s: %s\n", 
					AFentname(entry), strerror(errno));
				METHOD_LOG(LOG_ERR, KLDR_EFAIL);
				return(KLDR_EFAIL);
			    }
			} else if ((rc=cm_kls_call(compat_mod->compat_id, 
			    SYSCONFIG_QUERY, &next, 4, &qu, len)) != 0) {
				if (rc == KLDR_EFAIL)
				    cm_log(logp, LOG_ERR, "%s: %s\n", 
					AFentname(entry), strerror(errno));
				METHOD_LOG(LOG_ERR, (rc) ? rc : MSG_QUERIED );
				return(rc);
			}
			if(qu.count) {
			    sprintf(buf ,"\t%3d:%-16s\t%10d", 
				qu.svc, qu.name, qu.count);
			    if(qu.trace && 
				(compat_mod->module.cm_flags & CM_TRACE))
					sprintf(buf+strlen(buf),"\tTracing");
			    else if(qu.trace)
					sprintf(buf+strlen(buf),"\tTrace set");
			    cm_log(&logq, LOG_ERR, "%s\n", buf);
			}
			next = qu.next;
		}
	}


	METHOD_LOG(LOG_INFO, MSG_QUERIED );
	return(0);
}


/*
 * BIN_COMPAT_method_operate
 *	Set control flags in the loaded module.
 *	Flags that are being turned on are placed in on_flgs and flags
 *	that are being turned off are placed in off_flgs. The valid
 *	flags are CM_TRACE and CM_DEBUG.
 *
 *	The skip count tells trace to skip the next <number> system
 *	calls that it would have otherwise traced. A value of -1 causes
 *	this field to be ignored. A value of -2 causes the trace
 *	counters to be cleared to zero.
 *
 *	{no}trace=<svc_name> controls tracing on a per svc basis. The
 *	string is passed to the module for evaluation. A value of "all"
 *	results in all of the svcs being selected.
 */
int
BIN_COMPAT_method_operate(cm_log_t *logp, ENT_t entry, compat_mod_t *compat_mod,
char *opts)
{
	struct compat_operate opr;
	int	rc = 0;
	int	i;
	char	*p;

	if (!(compat_mod->compat_flags & COMPAT_LOADED)) {
		METHOD_LOG(LOG_ERR, KMOD_CONFIG_L_EEXIST);
		return(KMOD_CONFIG_L_EEXIST);
	}

	if (!(compat_mod->compat_flags & COMPAT_CONFIGURED)) {
		METHOD_LOG(LOG_ERR, KMOD_UNCONFIG_C_EEXIST);
		return(KMOD_UNCONFIG_C_EEXIST);
	}

	/* parse the input */
	bzero(&opr, sizeof(opr));
	opr.skip = -1;		/* don't change skip count */
	if(strncmp(opts, "trace", 5) == 0) {
		opr.on_flgs |= CM_TRACE;	/* [0] == flags to turn on */
		if(*(opts+5) == '=')
			strcpy(opr.svc, opts+6);
		rc = 1;
	}
	if(strncmp(opts, "notrace", 7) == 0) {
		opr.off_flgs |= CM_TRACE;	/* [1] == flags to turn off */
		if(*(opts+7) == '=')
			strcpy(opr.svc, opts+8);
		rc = 1;
	}
	if(strcmp(opts, "debug") == 0) {
		opr.on_flgs |= CM_DEBUG;
		rc = 1;
	}
	if(strcmp(opts, "nodebug") == 0) {
		opr.off_flgs |= CM_DEBUG;
		rc = 1;
	}
	if(strcmp(opts, "clear") == 0) {
		opr.skip = -2;		/* skip count == -2 means clear */
		rc = 1;
	}
	if(strlen(opts) > 5 && strncmp(opts, "skip=", 5) == 0) {
		for(p = opts+5, opr.skip = 0; *p && isdigit(*p); p++) 
			opr.skip = (opr.skip * 10) + *p - '0';
		rc = 1;
	}
	if(! rc) {
		METHOD_LOG(LOG_ERR, KMOD_OPERATE_ERR);
		return(KMOD_OPERATE_ERR);
	}

	/* modify the loaded image */
	if(compat_mod->module.cm_flags & CM_STATIC) {
	    if (syscall(SYS_kmodcall, compat_mod->module.cm_configure, 
		SYSCONFIG_OPERATE, &opr, sizeof(opr), NULL, 0)==-1) {
		cm_log(logp, LOG_ERR, "%s: %s\n", 
			AFentname(entry), strerror(errno));
		METHOD_LOG(LOG_ERR, KLDR_EFAIL);
		return(KLDR_EFAIL);
	    }
	} else {
            if ((rc=cm_kls_call(compat_mod->compat_id, SYSCONFIG_OPERATE,
		&opr, sizeof(opr), NULL, 0)) != 0) {
		cm_log(logp, LOG_ERR, "%s: %s\n", 
			AFentname(entry), strerror(errno));
		METHOD_LOG(LOG_ERR, rc);
		return(rc);
	    }
	}

	METHOD_LOG(LOG_INFO, MSG_OPERATED );
	return(0);
}



/*
 * BIN_COMPAT_method_start
 *	Called by startup for each dynamic module in auto list when -l flag 
 *	is set. (This may be called 0 or more times depending on how
 *	many modules are mentioned.)
 */
int
BIN_COMPAT_method_start(cm_log_t *logp, ENT_t entry, compat_mod_t *compat_mod)
{
	METHOD_LOG(LOG_INFO, MSG_STARTED );
	return(0);
}



/*
 *
 */
int
BIN_COMPAT_prtcfg(cm_log_t * logp, ENT_t entry, compat_mod_t * compat_mod, 
	char * string)
{
	cm_log(logp, LOG_ERR, "%s: %s\n", AFentname(entry), string);
}



/*
 * BIN_COMPAT_lookup_compat_mod
 *	Look up the name in the list of known modules. The list is the 
 *	union of names from stanzas in the database and loaded modules 
 *	in the kernel. Asking for an unknown name causes the name to
 *	be looked up in the database and if found it is added to the list.
 */
int
BIN_COMPAT_lookup_compat_mod(cm_log_t *logp, ENT_t entry, compat_mod_t ** p)
{
	char *	entname;
	int 	rc = 0;
	int	i;

	/* get state from kernel */
	if(rc = BIN_COMPAT_get_state( logp, entry )) {
		return(rc);
	}

	/* get the entry name and make sure it is OK */
	if ((entname=AFentname(entry)) == NULL
		|| (i=strlen(entname)) >= MAXCOMPATNAMSZ || i < 1) {
		METHOD_LOG(LOG_ERR, KMOD_ENOENT);
		return(KMOD_ENOENT);
	}


	/* look for name among extisting names */
	for(i=0; i < MAXCOMPAT; i++) {
		if (cm_list[i].compat_name[0] == '\0')
			continue;
		if (!strcmp(cm_list[i].compat_name, entname)) {
			*p = &cm_list[i];
			(*p)->compat_flags |= COMPAT_STANZA;
			return(0);
		}
	}
	for(i=0; i < MAXCOMPAT; i++) {
		if (cm_list[i].compat_name[0] == '\0') {
			*p = &cm_list[i];
			bzero(*p, sizeof( compat_mod_t ));
			strncpy((*p)->compat_name, entname, MAXCOMPATNAMSZ);
			(*p)->compat_flags |= COMPAT_STANZA;
			(*p)->compat_id = LDR_NULL_MODULE;
			return(0);
		}
	}
	METHOD_LOG(LOG_ERR, KMOD_ENOMEM);
	return(KMOD_ENOMEM);
}


/*
 * BIN_COMPAT_get_state
 *	Get the state of all currently loaded compatability modules
 *	from the kernel. Make sure that there is an entry for each 
 *	module in the module list.
 */
int
BIN_COMPAT_get_state( cm_log_t *logp, ENT_t entry )
{
int  avail = -1, i;
char *start = 0;
struct compat_mod buf;

	if(getsysinfo(GSI_COMPAT_MOD,&buf,sizeof(buf),&start,0)!= 0){
		METHOD_LOG(LOG_ERR, KMOD_KERNEL);
		return(KMOD_KERNEL);
	}

	/* Loop over all kernel entries. In the kernel each module has
	 * a name. When there is no module to return, buf is cleared to
	 * zero.	*/
	while(buf.cm_ld_name[0]) {
		/* look for name among extisting names */
		avail = -1;
		for(i=0; i < MAXCOMPAT; i++) {
			if (cm_list[i].compat_name[0] == '\0') {
				if(avail == -1)
					avail = i;
				continue;
			}
			/* found an entry so update the compat_mod */
			if(strcmp(cm_list[i].module.cm_ld_name,buf.cm_ld_name)
			    ==0){
				cm_list[i].compat_flags |= COMPAT_LOADED;
				cm_list[i].compat_flags |= COMPAT_KERNEL;
				cm_list[i].module = buf;
				if(cm_list[i].module.cm_flags&CM_CONFIG)
				    cm_list[i].compat_flags|=COMPAT_CONFIGURED;
				break;
			}
		}
		/* didn't find the name so allocate an entry */
		if(i == MAXCOMPAT) {
			if(avail == -1)
				return(KMOD_ENOMEM);
			else {
				i = avail;
				cm_list[i].compat_flags |=COMPAT_KERNEL;
				cm_list[i].compat_flags |=COMPAT_LOADED;
				cm_list[i].module = buf;
				if(cm_list[i].module.cm_flags&CM_CONFIG)
				     cm_list[i].compat_flags|=COMPAT_CONFIGURED;
				cm_list[i].compat_id = 
					(kmod_id_t)buf.cm_configure;
			}
		}
		/* get the next entry */
		(void)getsysinfo(GSI_COMPAT_MOD, &buf, sizeof(buf), &start, 0);
	}
#ifdef DEBUG_CODE
	for(i=0; i < MAXCOMPAT; i++) {
		if (cm_list[i].compat_name[0] != '\0') {
		    fprintf(stderr,"%d, %s, compat_flags 0x%x, cm_flags 0x%x\n",
			i, cm_list[i].compat_name,cm_list[i].compat_flags,
			cm_list[i].module.cm_flags);
		}
	}
#endif
	return(0);
}
