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
#define	tstring	1
#define tint	2
#define t2int	3
/* note that these types don't exist, but code is already written to
    support them */
#define	t3string 4
#define	t3int	5
#define t2char	6
#define t3char	7
#define t4char 8

/* define literals for the index into this structure */
#define iwinactive 0
#define iwininactive 1
#define ibell 2
#define ibell_volume 3
#define ikeyclick 4
#define ikey_volume 5
#define iautorepeat 6
#define ilock 7
#define ioperator 8
#define ioperator_mod 9
#define idissaver 10
#define idissaverseconds 11
#define idisforeground 12
#define idisbackground 13
#define idispattern 14
#define iwinforeground 15
#define iwinbackground 16
#define iptrforeground 17
#define iptrbackground 18
#define iptrbuttonorder 19
#define iptrshape 20
#define iptraccelnum 21
#define iptraccelden 22
#define iptraccelthr 23
#define iptrdoubleclick 24
#define paspect 25
#define pcolor 26
#define pfile 27
#define psaver 28
#define pformat 29
#define smstate 30
#define smconfirm 31
#define ihostlist 32
#define inumhosts 33
#define prtprompt 34
#define ikey_dialect 35
#define iwmiconstyle 36
#define iwmbordercolor 37
#define iwmformforeground 38
#define smscreennum 39
#define smscreenprompt 40
#define pscreennum 41
#define pscreenprompt 42
#define ilanguage 43
#define iappmenu 44
#define inumappmenu 45
#define iautostart 46
#define inumautostart 47
#define smx 48
#define smy 49
#define smpausetext 50
#define iwinmgrexe 51
#define iapps 52
#define inumapps 53
#define iapp1 54
#define iapp2 55
#define prtrotateprompt 56
#define pausesession 57

#define num_elements 58

globalref	char	*def_buffer;

globalref	struct	default_table
		{
		char	*name;
		int	format;
		char	*def_value;
		int	(*valid_check)();
		int	rdb_index;
		int	onroot;
		}def_table[num_elements];

/*
 * prototype
 */
char **get_remove_list(
#ifdef _NO_PROTO
#else
		       unsigned int *
#endif
		       );


