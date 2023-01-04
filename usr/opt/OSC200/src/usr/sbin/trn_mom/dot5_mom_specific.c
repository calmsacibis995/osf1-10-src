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
static char *rcsid = "@(#)$RCSfile: dot5_mom_specific.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/11/19 17:35:36 $";
#endif


#include "moss.h"
#include "common.h"
#include "extern_common.h"
#include "dot5_mom_specific.h"
#include <sys/errno.h>

extern int errno;

static int total_dot5_got = FALSE;


man_status
dot5_mom_specific_init()
{
     man_status status;
     man_status create_dot5_index_info();

     if ((inetfd = open("/dev/streams/kinfo", O_RDWR, 0)) < 0) {
        syslog(LOG_INFO,"TRN_MOM: open fail %d\n", errno);
        return(MAN_C_FAILURE);
     }

     /* Check if the interface exists.  If not exit */
     if ((status = create_dot5_index_info()) != MAN_C_SUCCESS) {
        syslog(LOG_INFO,"TRN_MOM: No token ring interface - trn_mom exiting\n");
        exit(1);
     }

     return(MAN_C_SUCCESS);
}

man_status
create_dot5_index_info()

/*    Function Description:
 *
 *      This initializes the dot5_index_info structure that is global to
 *      the Token Ring MOM.
 *
 *    Arguments:
 *      None.
 *    Return Value:
 *
 *         MAN_C_SUCCESS
 *	   MAN_C_PROCESSING_FAILURE
 *         MAN_C_FAILURE
 *
 *    Side Effects:
 *         None.
 *
 */
{

    if_blk buf;
    struct strioctl str;
    char *intf_prefix = "tr";
    int index = 0, i;
    int found_it = FALSE;
    struct dot5_info *tmp_index_ptr, *tmp_ptr_next;
    int dot5_count = 0;
    man_status status;
    man_status find_info_by_dot5_index();


    /*
     * First free the current list we have. If this is being
     * called at start up time we won't need it. If total_dot5_got
     * is true, then this is not the start up time.
     */
    tmp_index_ptr = dot5_index_info.index;
    if (total_dot5_got == TRUE) {
        while (tmp_index_ptr != NULL) {
            tmp_ptr_next = tmp_index_ptr->next;
            free(tmp_index_ptr);
            tmp_index_ptr = tmp_ptr_next;

        }
    }
    bzero(&buf, sizeof(buf));
    str.ic_cmd = KINFO_GET_INTERFACES;
    str.ic_timout = 15;
    str.ic_len = sizeof(buf);
    str.ic_dp = (char *)&buf;
    do {
        if (ioctl(inetfd, I_STR, &str) < 0) {
           perror(" ioctl");
           return(MAN_C_PROCESSING_FAILURE);
        }
        for (i=0; i < buf.total_cnt; i++) {
            index++;
            /*
             * NOTE: Here we assume that name of all Token Ring interfaces start
             *       with the alphabet "tr".
             */
            if (strncmp(intf_prefix, buf.info[i].if_name, 2) == 0) {
                dot5_count++;
                if (dot5_count == 1) {
		    dot5_index_info.index =
			(struct dot5_info *)malloc(sizeof(struct dot5_info));
                      dot5_index_info.index->next = NULL;
                      tmp_index_ptr = dot5_index_info.index;
                } else {
		    tmp_index_ptr->next =
			(struct dot5_info *)malloc(sizeof(struct dot5_info));
                      tmp_index_ptr = tmp_index_ptr->next;
                      tmp_index_ptr->next = NULL;
                }
                tmp_index_ptr->dot5_index = dot5_count;

                sprintf(tmp_index_ptr->intf_name, "%s%d",
                buf.info[i].if_name, buf.info[i].if_unit);
            }
        }
    } while (buf.more);

    /*
     * We need to get the total number of DOT5s on this station
     * If there are none, this station doesn't have any Token Ring interfaces.
     * NOTE: We have to do this at the end. We can't do this at the
     *       beginning because we don't have the names of any interfaces
     *       available. The names (tra0 etc.) become known to us
     *       only after we do the above.
     */
    if (total_dot5_got == FALSE)
    {
        status = find_info_by_dot5_index(1, TRN_MIB_ENTRY);
	if (status != MAN_C_SUCCESS)
            return(status);
        dot5_index_info.total_dot5 = ctr_info.ctr_ctrs.dot5Entry.dot5TrnNumber;
        if (dot5_index_info.total_dot5 <= 0)
            return(MAN_C_FAILURE); /* No Token Ring interfaces */
        total_dot5_got = TRUE;
    }
    return(MAN_C_SUCCESS);
} /* end of create_dot5_index_info() */

man_status
find_info_by_dot5_index(index,
                   counter_type)
/*    Function Description:
 *
 *    This function does an ioctl to retrieve the DOT5 information.
 *    This checks the index. If the index doesn't match the index
 *    requested it returns an error.
 *
 *    Arguments:
 *
 *        index                 Input: The index received.
 *        counter_type          Input: The type of counter requested.
 *    Return Value:
 *
 *      MAN_C_SUCCESS
 *      MAN_C_PROCESSING_FAILURE
 *      MAN_C_FAILURE
 *
 *    Side Effects:
 *      NOTE: ctr_info structure is global to this file. Information
 *            about the counters is filled into that.
 */
int index;
int counter_type;
{
    int s;
    int tmp_index = index;
    char *buf;
    struct dot5_info *index_ptr = dot5_index_info.index;


    if ( tmp_index < 0 )
        return (MAN_C_FAILURE);

    /*
     * Get a socket.
     */
    s = socket( AF_INET, SOCK_DGRAM, 0 );
    if (s < 0)
        return(MAN_C_PROCESSING_FAILURE);


    /*
     * Get the name of the interface.
     */
    while (index_ptr != NULL) {
        if (index != index_ptr->dot5_index)
            index_ptr = index_ptr->next;
        else {
            buf = index_ptr->intf_name;
            break;
        }
    }

    if (index_ptr == NULL)
        return(MAN_C_FAILURE);

    strcpy(ctr_info.ctr_name, buf);

    ctr_info.ctr_type = counter_type;

    if (ioctl( s, SIOCRDCTRS, &ctr_info ) < 0)
    {
	if (errno == ENOTSUP)
	{
		/* Not supported by driver */
    		close(s);
		return(MAN_C_NO_SUCH_OBJECT_INSTANCE);
	}
	else 
	{
		close(s);
		return(MAN_C_PROCESSING_FAILURE);
	}
    }

    close(s);


    if (ctr_info.ctr_ctrs.dot5Entry.dot5TrnNumber < index)
        return(MAN_C_FAILURE);

    return(MAN_C_SUCCESS);
}

