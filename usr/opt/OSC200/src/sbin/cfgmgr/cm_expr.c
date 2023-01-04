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
static char   *sccsid = "@(#)$RCSfile: cm_expr.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/04/14 13:59:05 $";
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

#include <stdio.h>
#include <AFdefs.h>
#include <sys/limits.h>
#include <sys/types.h>
#include "cm.h"

#define S_SEQ			'['		/* Start of Seq */
#define E_SEQ			']'		/* End of Seq */
#define	MAX_DIGIT_LEN		6
#define MAX_DIGIT_NUM		99999		/* ((10**(MAX_DIGIT_LEN-1))-1)*/

typedef struct {
	int	seq_type;
	int	seq_val1;
	int	seq_val2;
} seq_t;
						/* seq_type: */
#define	ISERROR_SEQ		0		/* Error in sequence */
#define	ISDIGIT_SEQ		1		/* Digit sequence */
#define	ISALPHA_SEQ		2		/* Character sequence */


int
parse_seq(seq, s)
	seq_t *	seq;
	char **	s;
{
	int	d1, d2;
	char	c1, c2;
	char *	eox;

	seq->seq_val1 = 0;
	seq->seq_val2 = 0;
	seq->seq_type = ISERROR_SEQ;

	(*s)++;

	if (isalpha(**s)) {
	    if (sscanf(*s, "%c-%c", &c1, &c2) == 2) {
		if ((islower(c1) && islower(c2))
		||  (isupper(c1) && isupper(c2)) ) {
		    seq->seq_val1 = c1;
		    seq->seq_val2 = c2;
		    seq->seq_type = ISALPHA_SEQ;
		}
	    }
	} else if (isdigit(**s)) {
	    if (sscanf(*s, "%d-%d", &d1, &d2) == 2) {
		if (d1 >= 0 && d1 <= MAX_DIGIT_NUM
		&&  d2 >= 0 && d2 <= MAX_DIGIT_NUM) {
		    seq->seq_val1 = d1;
		    seq->seq_val2 = d2;
		    seq->seq_type = ISDIGIT_SEQ;
		}
	    }
	}
	if (seq->seq_val1 > seq->seq_val2)
	    seq->seq_type = ISERROR_SEQ;

	if ((eox = (char *)strchr(*s, E_SEQ)) == NULL)
	    seq->seq_type = ISERROR_SEQ;
	else
	    *s = eox +1;

	return((seq->seq_type == ISERROR_SEQ) ? -1: 0);
}


int
expand_sfile_seq(seq_t * seq, int dindexc1, int dindexc2, int * dindexc,
		device_names_t * devnamelst)
{
    int		i, j;
    char	tmp[MAX_DIGIT_LEN +1];

	/*
	 *	Make new entry for (val_1 +1) -> val_2
	 *	Copy corresponding original entry
	 *	Cat on x
	 */
	for (i=seq->seq_val1 +1; i <= seq->seq_val2; i++) {
		(seq->seq_type == ISDIGIT_SEQ)
		    ? sprintf(tmp, "%d", i)
		    : sprintf(tmp, "%c", i);
		for (j=dindexc1; j <= dindexc2; j++) {
		    if ((*dindexc)+1 >= devnamelst->dsiz) {
			    devnamelst->derr = DBPARSE_E2BIG;
			    return(-1);
		    }
		    (*dindexc)++;
		    devnamelst->dargc++;
		    strcpy(devnamelst->dargv[*dindexc], devnamelst->dargv[j]);
		    strcat(devnamelst->dargv[*dindexc], tmp);
		}
	}

	/*
	 *	Make new entry for val_1
	 *	Copy corresponding original entry
	 *	Cat on x
	 */
	(seq->seq_type == ISDIGIT_SEQ)
		? sprintf(tmp, "%d", seq->seq_val1)
		: sprintf(tmp, "%c", seq->seq_val1);
	for (j=dindexc1; j <= dindexc2; j++)
		strcat(devnamelst->dargv[j], tmp);

	return(0);
}

int
expand_minor_seq(seq_t * seq, int mindexc1, int mindexc2, int * mindexc,
		device_minors_t * devminorlst)
{
	int	i, j;

	for (i=seq->seq_val1 +1; i <= seq->seq_val2; i++) {
		for (j = mindexc1; j <= mindexc2; j++) {
			if ((*mindexc)+1 >= devminorlst->msiz) {
			    devminorlst->merr = DBPARSE_E2BIG;
			    return(-1);
		         }
	    		(*mindexc)++;
	    		devminorlst->margc++;
	    		devminorlst->margv[*mindexc] =  devminorlst->margv[j];
	    		devminorlst->margv[*mindexc] =
				(devminorlst->margv[*mindexc] * 10) + i;
		}
    	}
    	for (j=mindexc1; j <= mindexc2; j++)
		devminorlst->margv[j] =
			(devminorlst->margv[j] * 10) + seq->seq_val1;

	return(0);
}


int
dbattr_mkdevnames(ATTR_t attr, device_names_t * devnamelst)
{
	int	rc, i;				/* Counter */
	int	dindexc;			/* Start of current list */
	int	dindexc1;			/* Start of current sublist */
	int	dindexc2;			/* End of current sublist */
	seq_t	seq;				/* Seq struct */
	char * 	p;
	char *	val;

	devnamelst->dargc = 0;
/*
Assumed zero'd by caller
	for (i=0; i < devnamelst->dsiz; i++)
		devnamelst->dargv[i][0] = '\0';
*/

	val=AFgetval(attr);
        for(dindexc=0; val != NULL; dindexc++, val=AFnxtval(attr)) {
		devnamelst->dargc++;
		devnamelst->dargv[dindexc][0] = '\0';
		dindexc1 = dindexc2 = dindexc;

		for (p=val; *p; ) {
		    if (*p == S_SEQ) {
			if (parse_seq(&seq, &p)) {
				devnamelst->derr = DBPARSE_EINVAL;
				return(-1);
			}
			if (expand_sfile_seq(&seq, dindexc1, dindexc2,
			    &dindexc, devnamelst))
				return(-1);
			dindexc2 = dindexc;
			continue;
		    }
		    for (i=dindexc1; i <= dindexc2; i++)
			devnamelst->dargv[i][strlen(devnamelst->dargv[i])] = *p;
		    p++;
		}
		if (dindexc +1 >= devnamelst->dsiz) {
			devnamelst->derr = DBPARSE_ENOMEM;
			return(-1);
		}
	}
    	return(0);
}

int
dbattr_mkdevminors(ATTR_t attr, device_minors_t * devminorlst)
{
	int	rc, i;				/* Counter */
	int 	mindexc;
	int	mindexc1;			/* Start of current sublist */
	int	mindexc2;			/* End of current sublist */
	seq_t	seq;				/* Seq struct */
	char *	p;
	char *	val;

	for (i=0; i < devminorlst->msiz; i++)
		devminorlst->margv[i] = 0;
    	devminorlst->margc = 0;

	val=AFgetval(attr);
        for(mindexc=0; val != NULL; mindexc++, val=AFnxtval(attr)) {

	    	devminorlst->margc++;
	    	devminorlst->margv[mindexc] =  0;
		mindexc1 = mindexc2 = mindexc;

		for (p=val; *p ; ) {
			if (*p == S_SEQ) {
	    			if (parse_seq(&seq, &p) < 0
				    || seq.seq_type != ISDIGIT_SEQ) {
					devminorlst->merr = DBPARSE_EINVAL;
					return(-1);
				}
	    			if (expand_minor_seq(&seq, mindexc1, mindexc2,
					&mindexc, devminorlst))
					return(-1);
	    			mindexc2 = mindexc;
	    			continue;
			}
	    		if (!isdigit(*p)) {
				devminorlst->merr = DBPARSE_EINVAL;
				return(-1);
			}
			for (i=mindexc1; i <= mindexc2; i++)
		    		devminorlst->margv[i] =
					(devminorlst->margv[i] *10) +(*p -'0');
			p++;
		}
		if (mindexc +1 >= devminorlst->msiz) {
			devminorlst->merr = DBPARSE_ENOMEM;
			return(-1);
		}
    	}
	return(0);
}
