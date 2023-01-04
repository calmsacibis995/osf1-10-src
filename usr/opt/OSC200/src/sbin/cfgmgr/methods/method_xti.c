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
static char	*sccsid = "@(#)$RCSfile: method_xti.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:04:25 $";
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

#if !defined(lint) && !defined(_NOIDENT)

#endif


#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysconfig.h>
#include <sys/stream.h>
#include <sys/socket.h>
#include <sys/un.h>					/* AF_UNIX */
#include <netinet/in.h>					/* AF_INET */
#include <netns/ns.h>					/* AF_NS */
#include <xti.h>					/* XTI */
#include <tli/xtiso_config.h>				/* XTISO config */
#include "cm.h"


/*
 *      Local BSS
 */
uint			XTI_loaded;
kmod_id_t		XTI_id;
char			namebuf[64 +1];

							/* UNIX */
xtiso_inadm_t	xti_inadm_unix_UDG = {
		{ OSF_XTISO_CONFIG_10, STR_IS_DEVICE, "xtisoUDG", NODEV },
          	{ AF_UNIX, SOCK_DGRAM, 0, T_CLTS, 16384, -2,
			-2, -2, sizeof(struct sockaddr_un), -2, 4096 }
};

xtiso_inadm_t	xti_inadm_unix_UST = {
        	{ OSF_XTISO_CONFIG_10, STR_IS_DEVICE, "xtisoUST", NODEV },
       		{ AF_UNIX, SOCK_STREAM, 0, T_COTS_ORD, 0, 1024,
			-2, -2, sizeof(struct sockaddr_un), 0, 4096 }
};


							/* INET */
xtiso_inadm_t	xti_inadm_inet_UDP = {
		{ OSF_XTISO_CONFIG_10, STR_IS_DEVICE, "xtisoUDP", NODEV },
          	{ AF_INET, SOCK_DGRAM, 0, T_CLTS, 16384, -2,
			-2, -2, sizeof(struct sockaddr_in), -2, 4096 }
};

xtiso_inadm_t	xti_inadm_inet_TCP = {
		{ OSF_XTISO_CONFIG_10, STR_IS_DEVICE, "xtisoTCP", NODEV },
          	{ AF_INET, SOCK_STREAM, 0, T_COTS_ORD, 0, 1024,
			-2, -2, sizeof(struct sockaddr_in), 0, 4096 }
};


							/* XNS */
xtiso_inadm_t	xti_inadm_xns_IDP = {
        	{ OSF_XTISO_CONFIG_10, STR_IS_DEVICE, "xtisoIDP", NODEV },
		{ AF_NS, SOCK_DGRAM, 0, T_CLTS, 16384, -2,
			-2, -2, sizeof(struct sockaddr_ns), -2, 4096 }
};

xtiso_inadm_t	xti_inadm_xns_SPP = {
        	{ OSF_XTISO_CONFIG_10, STR_IS_DEVICE, "xtisoSPP", NODEV },
          	{ AF_NS, SOCK_STREAM, 0, T_COTS_ORD, 0, 1024,
			-2, -2, sizeof(struct sockaddr_ns), 0, 4096 }
};


#define	XTI_NUMSUBDEV	2		/* max # of sub devices per domain */
typedef struct {
	uint		xti_type;
	uint		xti_typeconfigured;
	uint		xti_numadm;
	char *		xti_name[XTI_NUMSUBDEV];
	xtiso_inadm_t *	xti_inadm[XTI_NUMSUBDEV];
	xtiso_outadm_t  xti_outadm[XTI_NUMSUBDEV];
} xti_mod_t;


xti_mod_t xti_mod_info[] = {
	{ AF_UNIX, 0, 2, {"UDG", "UST"}, 
		{&xti_inadm_unix_UDG, &xti_inadm_unix_UST}, 0, 0 },
	{ AF_INET, 0, 2, {"UDP", "TCP"},
		{&xti_inadm_inet_UDP, &xti_inadm_inet_TCP}, 0, 0 },
	{ AF_NS,   0, 2, {"IDP", "SPP"},
		{&xti_inadm_xns_IDP,  &xti_inadm_xns_SPP }, 0, 0 }
};

#define XTI_MAXTYPES    (sizeof xti_mod_info / sizeof xti_mod_info[0])

char * 	XTI_glue(char * , char * );

/*
 *
 *	Name:		XTI_method()
 *	Description:	XTI Configuration Method
 *	Returns:	Zero 		On success.
 *			-1		On failure.
 *
 */
int
XTI_method( cm_log_t * logp, ENT_t entry, cm_op_t op, cm_op_t * rop, 
	char *opts )
{
	int		rc;
	xti_mod_t *	xti_mod;

	if (rc=XTI_lookup_xti_mod(entry, &xti_mod)) {
		METHOD_LOG(LOG_ERR, rc);
		return(-1);
	}

	rc = 0;
	if (op & CM_OP_LOAD) {
		rc = XTI_method_load(logp, entry, xti_mod);
		if (rc == 0) {
			*rop = CM_OP_LOAD;
			METHOD_LOG(LOG_INFO, MSG_LOADED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}

	if (op & CM_OP_CONFIGURE) {
		rc = XTI_method_configure(logp, entry, xti_mod);
		if (rc == 0) {
			*rop = CM_OP_CONFIGURE;
			METHOD_LOG(LOG_INFO, MSG_CONFIGURED);
		} else {
			METHOD_LOG(LOG_ERR, rc);
		}
	}

	if (op & CM_OP_UNCONFIGURE) {
		rc = XTI_method_unconfigure(logp, entry, xti_mod);
		if (rc == 0) {
			*rop = CM_OP_UNCONFIGURE;
			METHOD_LOG(LOG_INFO, MSG_UNCONFIGURED);
		} else {
			METHOD_LOG(LOG_ERR, rc);
		}
	}

	if (op & CM_OP_UNLOAD) {
		rc = XTI_method_unload(logp, entry, xti_mod);
		if (rc == 0) {
			*rop = CM_OP_UNLOAD;
			METHOD_LOG(LOG_INFO, MSG_UNLOADED);
		} else {
			METHOD_LOG(LOG_ERR, rc);
		}
	}
	return(rc == 0 ? 0 : -1);
}


/*
 *
 */
XTI_method_load( cm_log_t * logp, ENT_t entry, xti_mod_t * xti_mod )
{
	int	rc;

	if (XTI_loaded)
		return(KMOD_LOAD_L_EBUSY);
#ifndef TEST
	if ((rc=cm_kls_load(entry, &XTI_id)) != 0)
		return(rc);
#endif
	XTI_loaded = 1;
	return(0);
}


/*
 *
 */
XTI_method_unload( cm_log_t * logp, ENT_t entry, xti_mod_t * xti_mod )
{
	int	rc;

	if (!XTI_loaded)
		return(KMOD_UNLOAD_L_EEXIST);
	if (XTI_inuse())
		return(KMOD_UNLOAD_C_EBUSY);
#ifndef TEST
	if ((rc=cm_kls_unload(XTI_id)) != 0)
		return(rc);
#endif
	XTI_id = LDR_NULL_MODULE;
	XTI_loaded = 0;
	return(0);
}


/*
 *
 */
int
XTI_method_configure( cm_log_t * logp, ENT_t entry, xti_mod_t * xti_mod)
{
	int	rc;
	int	i;
	char *	name;

	if (!XTI_loaded)
		return(KMOD_CONFIG_L_EEXIST);
	if (xti_mod->xti_typeconfigured > 0)
		return(KMOD_CONFIG_C_EBUSY);

	for( i=0; i < xti_mod->xti_numadm; i++) {
		name = xti_mod->xti_name[i];
		xti_mod->xti_inadm[i]->sc.sc_devnum =
			dbattr_devno(entry, XTI_glue(name,XTI_MAJOR), NODEV);
	}

	for( i=0; i < xti_mod->xti_numadm; i++) {
#ifndef TEST
        	rc = cm_kls_call(XTI_id, SYSCONFIG_CONFIGURE, 
			xti_mod->xti_inadm[i], sizeof(xtiso_inadm_t),
			xti_mod->xti_outadm[i], sizeof(xtiso_outadm_t));
		if (rc != 0) {
			if (rc == KLDR_EFAIL)
				cm_log(logp, LOG_ERR, "%s: %s\n", 
					AFentname(entry), strerror(errno));
			return(rc);
		}
#else
		bcopy(xti_mod->xti_inadm[i], &xti_mod->xti_outadm[i], 
			sizeof(str_config_t));
#endif
		xti_mod->xti_typeconfigured++;
		XTI_prtcfg(logp, entry, xti_mod, i, "configured");
		XTI_mknods(logp, entry, xti_mod, i, 1);
	}

	return(0);
}


/*
 *
 */
int
XTI_method_unconfigure( cm_log_t * logp, ENT_t entry, xti_mod_t * xti_mod)
{
	int	rc;
	int	i;

	if (!XTI_loaded)
		return(KMOD_UNCONFIG_L_EEXIST);
	if (xti_mod->xti_typeconfigured <= 0)
		return(KMOD_UNCONFIG_C_EEXIST);

	for( i=0; i < xti_mod->xti_numadm; i++) {
#ifndef TEST
        	rc = cm_kls_call(XTI_id, SYSCONFIG_UNCONFIGURE, NULL, 0, 
			xti_mod->xti_outadm[i], sizeof(xtiso_outadm_t));
		if (rc != 0) {
			if (rc == KLDR_EFAIL)
				cm_log(logp, LOG_ERR, "%s: %s\n", 
					AFentname(entry), strerror(errno));
			return(rc);
		}
#endif
		xti_mod->xti_typeconfigured--;
		XTI_prtcfg(logp, entry, xti_mod, i, "deconfigured");
		XTI_mknods(logp, entry, xti_mod, i, 0);
	}

	return(0);
}


XTI_prtcfg(cm_log_t * logp, ENT_t entry, xti_mod_t * xti_mod, int idx, char * string)
{
	str_config_t  * outsc;

	outsc = &(xti_mod->xti_outadm[idx].sc);

        if (outsc->sc_version == OSF_XTISO_CONFIG_10) {
                switch (outsc->sc_sa_flags) {
                case STR_IS_DEVICE:
                        cm_log(logp, LOG_ERR,
                                "%s: %s XTI \"%s\" device (%d/%d)\n",
                                AFentname(entry), string,
                                outsc->sc_sa_name,
                                major(outsc->sc_devnum),
                                minor(outsc->sc_devnum));
                        break;
                case STR_IS_MODULE:
                        cm_log(logp, LOG_ERR,
                                "%s: %s XTI \"%s\" module\n",
                                AFentname(entry), string,
                                outsc->sc_sa_name);
                        break;
		}
        } else
                cm_log(logp, LOG_ERR, "%s: %s STREAMS \"%s\" module\n",
                        AFentname(entry), string, 
			xti_mod->xti_inadm[idx]->sc.sc_sa_name);
}

int
XTI_lookup_xti_mod( ENT_t entry, xti_mod_t ** p)
{
	char *	entname;
	int 	rc;
	int	i;
	int	type;

	if ((entname=AFentname(entry)) == NULL
                || (i=strlen(entname)) > FMNAMESZ || i < 1)
		return(KMOD_ENOENT);

	if (!strcmp(entname,"xti-unix"))
		type = AF_UNIX;
	else if (!strcmp(entname,"xti-inet"))
		type = AF_INET;
	else if (!strcmp(entname,"xti-xns"))
		type = AF_NS;
	else
		return(KMOD_ENOENT);

	for(i=0; i < XTI_MAXTYPES; i++) {
		if (xti_mod_info[i].xti_type == type) {
			*p = &xti_mod_info[i];
			return(0);
		}
	}
	return(KMOD_ENOENT);
}

XTI_inuse()
{
	int	i;
	int	cnt = 0;

	for(i=0; i < XTI_MAXTYPES; i++) {
		if (xti_mod_info[i].xti_type
		&&  xti_mod_info[i].xti_typeconfigured)
			cnt++;
	}
	return(cnt);
}


int
XTI_mknods( cm_log_t * logp, ENT_t entry, xti_mod_t * xti_mod, int idx, int make)
{
	cm_devices_t 	devices;
	struct ATTR	files_attr;
	struct ATTR	minors_attr;
	char		files_buf[32];
	char		minors_buf[32];
	char *		unit_name;
	int		xti_major;
	int		xti_minor;
	int		clone_major;
	int		clone_minor;
	int		unit_major;
	int		unit_num;
	int		rc;
	int		mknod_op;
	str_config_t * outsc;
	char *		name;

	outsc = &(xti_mod->xti_outadm[idx].sc);
	name = xti_mod->xti_name[idx];

	if (outsc->sc_version != OSF_XTISO_CONFIG_10)
		return(-1);

	if ( !(outsc->sc_sa_flags & STR_IS_DEVICE)
		|| outsc->sc_devnum == NODEV)
		return(-2);

	devices.dir = dbattr_string(entry, DEVICE_DIR, "/dev");
	devices.mode = dbattr_mode(entry, DEVICE_MODE, DEVMODE_DFLT);
	devices.uid = dbattr_user(entry, DEVICE_USER, 0);
	devices.gid = dbattr_group(entry, DEVICE_GROUP, 0);
	devices.type = DEVTYPE_CHR;
	devices.subdir = dbattr_string(entry, DEVICE_SUBDIR, NULL);
	unit_num = dbattr_num(entry,XTI_glue(name,XTI_UNITS),0);
	unit_name = dbattr_string(entry, XTI_glue(name, XTI_NAME), NULL);
	if (unit_name == NULL)
		return(-1);

	xti_major = major(outsc->sc_devnum);
	xti_minor = minor(outsc->sc_devnum);
	if (xti_minor > 0) {
				/* Driver supports BOTH Clone & Unit devices */
		clone_major = xti_major;
		clone_minor = xti_minor;
		if (unit_num > 0)
			unit_major = xti_minor;
		else
			unit_major = 0;
	} else {
				 /* Driver supports ONLY Unit devices */
		clone_major = 0;
		clone_minor = 0;
		if (unit_num > 0)
			unit_major = xti_major;
		else
			unit_major = 0;
	}

	if (clone_major > 0) {
		sprintf(files_buf,"%s", unit_name);
		sprintf(minors_buf,"%d", clone_minor);
		XTI_mkatr(&files_attr, DEVICE_CHRFILES, files_buf);
		XTI_mkatr(&minors_attr, DEVICE_CHRMINOR, minors_buf);
		devices.majno = clone_major;
		devices.devfiles = &files_attr;
		devices.devminors = &minors_attr;
		if (make)
			mknod_op = CM_RMNOD_FILE | CM_MKNOD_FILE | 
				CM_RPT_MKNOD | CM_RPT_HEADER;
		else
			mknod_op = CM_RMNOD_FILE | CM_RPT_RMNOD |
				CM_RPT_HEADER;
		rc = cm_mknods(logp, AFentname(entry), mknod_op, &devices);
	}

	if (unit_num > 0) {
		if (unit_num == 1) {
			sprintf(files_buf,"%s0", unit_name);
			sprintf(minors_buf,"0");
		} else {
			sprintf(files_buf,"%s[0-%d]", unit_name, unit_num-1);
			sprintf(minors_buf,"[0-%d]", unit_num-1);
		}
		XTI_mkatr(&files_attr, DEVICE_CHRFILES, files_buf);
		XTI_mkatr(&minors_attr, DEVICE_CHRMINOR, minors_buf);
		devices.majno = unit_major;
		devices.devfiles = &files_attr;
		devices.devminors = &minors_attr;
		if (make)
			mknod_op = CM_RMNOD_MAJR | CM_RMNOD_FILE |
					CM_MKNOD_FILE | CM_RPT_MKNOD;
		else
			mknod_op = CM_RMNOD_MAJR | CM_RPT_RMNOD;
		if (clone_major == 0)
			mknod_op |= CM_RPT_HEADER;
		rc = cm_mknods(logp, AFentname(entry), mknod_op, &devices);
	}

	return(0);
}


int
XTI_mkatr( ATTR_t attr, char * name, char * value )
{
	if (attr == NULL || name == NULL || value == NULL)
		return(-1);

	attr->AT_nvalue = NULL;
	attr->AT_name = name;
	attr->AT_value = value;
	while (*value++ != '\0') 		/* Doubly terminate value */
		;
	*value = '\0';
	return(0);
}


char *
XTI_glue(char * s1, char * s2)
{
	sprintf(namebuf, "%s_%s", s1, s2);
	return(namebuf);
}

